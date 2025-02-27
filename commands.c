#include "ndn.h"

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

    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating TCP socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(connect_port);
    server_addr.sin_addr.s_addr = inet_addr(connect_ip);

    // Connect to the external node
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to external node");
        close(sockfd);
        return;
    }

    printf("Connected to node %s:%d\n", connect_ip, connect_port);

    // Send ENTRY message
    char message[MAX_BUFFER];
    snprintf(message, MAX_BUFFER, "ENTRY %s %d\n", node->ip, node->port);
    send(sockfd, message, strlen(message), 0);

    // Store the correct external neighbor (IP and Port)
    strcpy(node->ext_neighbor_ip, connect_ip);
    node->ext_neighbor_port = connect_port;
    printf("External neighbor set to %s:%d\n", connect_ip, connect_port);
}




// Show node topology
void show_topology(ndn_node *node) {
    printf("External neighbor: ");
    if (node->ext_neighbor_port != -1) {
        printf("%s:%d", node->ext_neighbor_ip, node->ext_neighbor_port);
    } else {
        printf("-1");
    }
    printf("\n");

    printf("Safeguard: %d\n", node->safeguard);

    printf("Internal neighbors: ");
    for (int i = 0; i < node->num_internal_neighbors; i++) {
        printf("%s:%d ", node->internal_neighbors[i].ip, node->internal_neighbors[i].port);
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

void process_tcp_messages(int client_fd, ndn_node *node) {
    char buffer[MAX_BUFFER];
    int bytes_received = recv(client_fd, buffer, MAX_BUFFER, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "ENTRY", 5) == 0) {
            char new_ip[16];
            int new_port;
            sscanf(buffer, "ENTRY %s %d", new_ip, &new_port);

            printf("Received ENTRY request from %s:%d\n", new_ip, new_port);

            // If no external neighbor is set, assign this node as external
            if (node->ext_neighbor_port == -1) {
                strcpy(node->ext_neighbor_ip, new_ip);
                node->ext_neighbor_port = new_port;
                printf("Setting external neighbor to %s:%d\n", new_ip, new_port);
            } else {
                // Otherwise, add it as an internal neighbor
                strcpy(node->internal_neighbors[node->num_internal_neighbors].ip, new_ip);
                node->internal_neighbors[node->num_internal_neighbors].port = new_port;
                node->num_internal_neighbors++;
                printf("Added %s:%d as an internal neighbor\n", new_ip, new_port);
            }
        }
    }
}

