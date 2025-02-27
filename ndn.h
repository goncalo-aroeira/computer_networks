#ifndef NDN_H
#define NDN_H

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
    char ext_neighbor_ip[16];
    int ext_neighbor_port;
    int safeguard;
    struct {
        char ip[16];
        int port;
    } internal_neighbors[MAX_NEIGHBORS];
    int num_internal_neighbors;
} ndn_node;


// Function declarations
void initialize_node(ndn_node *node, char *ip, int port);
int send_udp_request(char *message, char *response, size_t response_size);
void join_network(ndn_node *node, char *network);
void direct_join(ndn_node *node, char *network, char *connect_ip, int connect_port);
void show_topology(ndn_node *node);
void process_command(char *command, ndn_node *node);

#endif
