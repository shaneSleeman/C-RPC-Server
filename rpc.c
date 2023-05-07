#include "rpc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

// Socket functions
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_FUNCTIONS 1000 // Placeholder before dynamic

/*  Note that due to being limited on time, I only aim to
    obtain 3 marks to pass the assignment hurdle. */

struct rpc_server {
    int port;
    int functions_count;
    char **functions;
    rpc_handler *handlers;
    int socket;
};

rpc_server *rpc_init_server(int port) {
    rpc_server *server = malloc(sizeof(rpc_server));
    server->port = port;
    server->functions_count = 0;
    server->functions = NULL;
    server->handlers = NULL;

    // Set up socket
    server->socket = socket(AF_INET6, SOCK_STREAM, 0);

    // Reuse option
    int reuse = 1;
    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        close(server->socket);
        free(server);
        return NULL;
    }

    // Set up address
    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    // Binding
    if (bind(server->socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        close(server->socket);
        free(server);
        return NULL;
    }

    // Listen
    if (listen(server->socket, SOMAXCONN) == -1) {
        close(server->socket);
        free(server);
        return NULL;
    }

    return server;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    return -1;
}

void rpc_serve_all(rpc_server *srv) {

}

struct rpc_client {
    char *ip;
    int port;
};

struct rpc_handle {
    /* Add variable(s) for handle */
};

rpc_client *rpc_init_client(char *addr, int port) {
    rpc_client *client = malloc(sizeof(rpc_client));
    client->ip = strdup(addr);
    client->port = port;
    return client;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {

}

void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}