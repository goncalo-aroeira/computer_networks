#include "ndn.h"

void process_tcp_messages(int client_fd, ndn_node *node);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ndn_node node;
    initialize_node(&node, argv[1], atoi(argv[2]));

    // Create TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(node.port);
    server_addr.sin_addr.s_addr = inet_addr(node.ip);

    // Bind socket to IP and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Error listening on socket");
        return EXIT_FAILURE;
    }

    fd_set rfds;
    int maxfd = server_fd;

    char command[MAX_BUFFER];

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(server_fd, &rfds);

        int activity = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Error in select");
            continue;
        }

        // Handle new TCP connections
        if (FD_ISSET(server_fd, &rfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (new_socket < 0) {
                perror("Error accepting connection");
                continue;
            }
            printf("New node connected.\n");

            // Process incoming messages
            process_tcp_messages(new_socket, &node);
        }

        // Handle user input
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (fgets(command, MAX_BUFFER, stdin) == NULL) {
                perror("Error reading input");
            }
            process_command(command, &node);
        }
    }

    close(server_fd);
    return 0;
}
