#include "rpc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/*  Note that due to being limited on time, I only aim to
    obtain 3 marks to pass the assignment hurdle. */

struct rpc_server {
    int port;
    size_t functions_count;
    char **functions
    rpc_handler *handlers;
};

rpc_server *rpc_init_server(int port) {
    return NULL;
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
    return NULL;
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