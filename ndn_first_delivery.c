#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define MAX_BUFFER 256
#define DEFAULT_SERVER_IP "193.136.138.142"
#define DEFAULT_SERVER_PORT 59000
#define MAX_NEIGHBORS 10

// Structure for an NDN node
typedef struct {
    char ip[16];
    int port;
    int fd;
    int ext_neighbor;
    int safeguard;
    int internal_neighbors[MAX_NEIGHBORS];
    int num_internal_neighbors;
} ndn_node;

// Initialize a node
void initialize_node(ndn_node *node, char *ip, int port) {
    strcpy(node->ip, ip);
    node->port = port;
    node->fd = -1;
    node->ext_neighbor = -1;
    node->safeguard = -1;
    node->num_internal_neighbors = 0;
}

// Send a UDP request and receive a response
int send_udp_request(char *message, char *response, size_t response_size) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating UDP socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_SERVER_IP);

    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    struct timeval timeout = {3, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    int received = recvfrom(sockfd, response, response_size, 0, (struct sockaddr *)&server_addr, &addrlen);
    close(sockfd);
    return received;
}

// Join a network via the server
void join_network(ndn_node *node, char *network) {
    char request[MAX_BUFFER], response[MAX_BUFFER];
    snprintf(request, MAX_BUFFER, "NODES %s", network);

    int received = send_udp_request(request, response, MAX_BUFFER);
    if (received < 0) {
        printf("No response from server. Cannot join network.\n");
        return;
    }

    response[received] = '\0';
    char *token = strtok(response, "\n");
    token = strtok(NULL, "\n");  // Skip header

    if (!token) {
        printf("No nodes found. Creating new network.\n");
        snprintf(request, MAX_BUFFER, "REG %s %s %d", network, node->ip, node->port);
        send_udp_request(request, response, MAX_BUFFER);
        return;
    }
    
    // Pick a random node to connect to
    srand(time(NULL));
    int count = 0, choice = rand() % MAX_NEIGHBORS;
    char connect_ip[16];
    int connect_port;
    
    while (token && count <= choice) {
        sscanf(token, "%s %d", connect_ip, &connect_port);
        token = strtok(NULL, "\n");
        count++;
    }
    
    printf("Connecting to existing node %s:%d\n", connect_ip, connect_port);
    // Here we would initiate a TCP connection and send ENTRY message (omitted for now)
    
    snprintf(request, MAX_BUFFER, "REG %s %s %d", network, node->ip, node->port);
    send_udp_request(request, response, MAX_BUFFER);
}

// Direct join a network without server contact
void direct_join(ndn_node *node, char *network, char *connect_ip, int connect_port) {
    if (strcmp(connect_ip, "0.0.0.0") == 0) {
        printf("Creating new network as first node.\n");
        return;
    }
    
    printf("Directly connecting to %s:%d\n", connect_ip, connect_port);
    // Here we would initiate a TCP connection and send ENTRY message (omitted for now)
}

// Show node topology
void show_topology(ndn_node *node) {
    printf("External neighbor: %d\n", node->ext_neighbor);
    printf("Safeguard: %d\n", node->safeguard);
    printf("Internal neighbors: ");
    for (int i = 0; i < node->num_internal_neighbors; i++) {
        printf("%d ", node->internal_neighbors[i]);
    }
    printf("\n");
}

// Process user commands
void process_command(char *command, ndn_node *node) {
    char cmd[MAX_BUFFER], arg1[MAX_BUFFER], arg2[MAX_BUFFER];
    int arg3;
    int parsed = sscanf(command, "%s %s %s %d", cmd, arg1, arg2, &arg3);

    if (strcmp(cmd, "join") == 0 || strcmp(cmd, "j") == 0) {
        join_network(node, arg1);
    } else if ((strcmp(cmd, "direct_join") == 0 || strcmp(cmd, "dj") == 0) && parsed == 4) {
        direct_join(node, arg1, arg2, arg3);
    } else if (strcmp(cmd, "show_topology") == 0 || strcmp(cmd, "st") == 0) {
        show_topology(node);
    } else {
        printf("Unknown command.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ndn_node node;
    initialize_node(&node, argv[1], atoi(argv[2]));
    
    char command[MAX_BUFFER];
    while (1) {
        printf("Enter command: ");
        if (fgets(command, MAX_BUFFER, stdin) == NULL) {
            perror("Error reading input");
        }
                process_command(command, &node);
    }
    
    return 0;
}
