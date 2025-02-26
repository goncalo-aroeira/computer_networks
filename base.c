#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/select.h>

#define MAX_BUFFER 256
#define DEFAULT_SERVER_IP "193.136.138.142"
#define DEFAULT_SERVER_PORT 59000
#define CACHE_SIZE 10

// Estrutura do nó na rede NDN
typedef struct {
    int id;
    char ip[16];
    int port;
    int fd;
    int ext_neighbor;
    int safeguard;
    int internal_neighbors[10];
    int num_internal_neighbors;
    char cache[CACHE_SIZE][MAX_BUFFER];
    int cache_count;
} ndn_node;

// Função para visualizar topologia
typedef struct {
    int ext_neighbor;
    int safeguard;
    int internal_neighbors[10];
    int num_internal_neighbors;
} topology;

// Função para iniciar um nó na rede
void initialize_node(ndn_node *node, char *ip, int port) {
    strcpy(node->ip, ip);
    node->port = port;
    node->id = -1;
    node->fd = -1;
    node->ext_neighbor = -1;
    node->safeguard = -1;
    node->num_internal_neighbors = 0;
    node->cache_count = 0;
}

// Função para registrar o nó na rede via UDP
void register_node(ndn_node *node, char *network) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER];
    socklen_t addrlen;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_SERVER_IP);

    snprintf(buffer, MAX_BUFFER, "REG %s %s %d", network, node->ip, node->port);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    addrlen = sizeof(server_addr);
    recvfrom(sockfd, buffer, MAX_BUFFER, 0, (struct sockaddr *)&server_addr, &addrlen);

    printf("Resposta do servidor: %s\n", buffer);
    close(sockfd);
}

// Função para conectar a outro nó via TCP
void connect_to_node(ndn_node *node, char *ip, char *port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket TCP");
        return;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao nó");
        close(sockfd);
        return;
    }
    
    printf("Conectado ao nó %s:%s\n", ip, port);
    node->fd = sockfd;
}

// Função para criar um objeto na cache
void create_object(ndn_node *node, char *name) {
    if (node->cache_count < CACHE_SIZE) {
        strcpy(node->cache[node->cache_count], name);
        node->cache_count++;
        printf("Objeto %s criado.\n", name);
    } else {
        printf("Cache cheia! Não foi possível criar o objeto.\n");
    }
}

// Função para remover um objeto da cache
void delete_object(ndn_node *node, char *name) {
    for (int i = 0; i < node->cache_count; i++) {
        if (strcmp(node->cache[i], name) == 0) {
            for (int j = i; j < node->cache_count - 1; j++) {
                strcpy(node->cache[j], node->cache[j + 1]);
            }
            node->cache_count--;
            printf("Objeto %s removido.\n", name);
            return;
        }
    }
    printf("Objeto %s não encontrado.\n", name);
}

// Função para buscar um objeto na rede via TCP
void retrieve_object(ndn_node *node, char *name) {
    if (node->fd < 0) {
        printf("Nenhuma conexão ativa para buscar o objeto.\n");
        return;
    }
    char message[MAX_BUFFER];
    snprintf(message, MAX_BUFFER, "INTEREST %s\n", name);
    send(node->fd, message, strlen(message), 0);
    printf("Buscando objeto %s...\n", name);
}


void show_topology(ndn_node *node) {
    printf("Vizinho externo: %d\n", node->ext_neighbor);
    printf("Salvaguarda: %d\n", node->safeguard);
    printf("Vizinhos internos: ");
    for (int i = 0; i < node->num_internal_neighbors; i++) {
        printf("%d ", node->internal_neighbors[i]);
    }
    printf("\n");
}

// Função para exibir nomes dos objetos na cache
void show_names(ndn_node *node) {
    printf("Objetos na cache:\n");
    for (int i = 0; i < node->cache_count; i++) {
        printf("%s\n", node->cache[i]);
    }
}
// Função para sair da rede
void leave_network(ndn_node *node) {
    printf("Saindo da rede...\n");
    close(node->fd);
    node->fd = -1;
    printf("Nó removido da rede.\n");
}

// Função para fechar a aplicação
void exit_application(ndn_node *node) {
    leave_network(node);
    printf("Fechando aplicação.\n");
    exit(0);
}

// Processar comandos do usuário
void process_command(char *command, ndn_node *node) {
    char cmd[MAX_BUFFER];
    char arg1[MAX_BUFFER], arg2[MAX_BUFFER];
    sscanf(command, "%s %s %s", cmd, arg1, arg2);
    
    if (strcmp(cmd, "join") == 0 || strcmp(cmd, "j") == 0) {
        register_node(node, arg1);
    } else if (strcmp(cmd, "direct") == 0 || strcmp(cmd, "dj") == 0) {
        connect_to_node(node, arg1, arg2);
    } else if (strcmp(cmd, "create") == 0 || strcmp(cmd, "c") == 0) {
        create_object(node, arg1);
    } else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "dl") == 0) {
        delete_object(node, arg1);
    } else if (strcmp(cmd, "retrieve") == 0 || strcmp(cmd, "r") == 0) {
        retrieve_object(node, arg1);
    }
    else if (strcmp(cmd, "st") == 0) {
        show_topology(node);
    } else if (strcmp(cmd, "sn") == 0) {
        show_names(node);
    } else if (strcmp(cmd, "leave") == 0 || strcmp(cmd, "l") == 0) {
        leave_network(node);
    } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "x") == 0) {
        exit_application(node);
    } else {
        printf("Comando não reconhecido.\n");
    }
}




// Função principal
int main(int argc, char *argv[]) {
    fd_set rfds;
    int fd, maxfd;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char buffer[MAX_BUFFER], command[MAX_BUFFER], my_IP[16], server_ip[16];
    ndn_node node;

    if (argc < 3) {
        fprintf(stderr, "Uso: %s <IP> <PORT> [<SERVER_IP> <SERVER_PORT>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    strcpy(my_IP, argv[1]);
    int my_port = atoi(argv[2]);
    char *server_ip_arg = (argc >= 4) ? argv[3] : DEFAULT_SERVER_IP;
    int server_port = (argc >= 5) ? atoi(argv[4]) : DEFAULT_SERVER_PORT;
    
    strcpy(server_ip, server_ip_arg);
    initialize_node(&node, my_IP, my_port);
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Erro ao criar socket TCP");
        exit(EXIT_FAILURE);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(NULL, argv[2], &hints, &res) != 0) {
        perror("Erro no getaddrinfo");
        exit(EXIT_FAILURE);
    }
    
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Erro ao fazer bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(fd, 5) == -1) {
        perror("Erro ao escutar conexões");
        exit(EXIT_FAILURE);
    }
    
    maxfd = fd;


    while (1) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        FD_SET(STDIN_FILENO, &rfds);
        
        int counter = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (counter <= 0) {
            perror("Erro no select");
            exit(EXIT_FAILURE);
        }
        
        if (FD_ISSET(fd, &rfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_socket = accept(fd, (struct sockaddr *)&client_addr, &client_len);
            if (new_socket < 0) {
                perror("Erro ao aceitar conexão");
                continue;
            }
            printf("Novo nó conectado.\n");
            if (new_socket > maxfd) maxfd = new_socket;
        }
        
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            fgets(command, MAX_BUFFER, stdin);
            process_command(command, &node);
        }
    }
    
    close(fd);
    return 0;
}