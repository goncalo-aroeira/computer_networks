#include "ndn.h"

// Initialize a node
void initialize_node(ndn_node *node, char *ip, int port) {
    strcpy(node->ip, ip);
    node->port = port;
    node->fd = -1;
    node->ext_neighbor_ip[0] = '\0'; 
    node->ext_neighbor_port = -1;
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
