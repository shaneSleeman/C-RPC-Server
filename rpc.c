#include "rpc.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Socket functions
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_FUNCTIONS 1000	// Placeholder before dynamic

/*  Note that due to being limited on time, I only aim to
    obtain 3 marks in the CI to pass the assignment hurdle. 
*/

struct rpc_server
{
  int port;
  int functions_count;
  char **functions;
  rpc_handler *handlers;
  int socket;
};

/*  Extending data structure to include location, as cannot
    modify rpc.h */
typedef struct
{
  rpc_data data;
  int location;
}
rpc_data_location;

typedef enum
{
  FIND,
  CALL
}
rpc_request_type;

rpc_server *
rpc_init_server (int port)
{
  rpc_server *server = malloc (sizeof (rpc_server));
  server->port = port;
  server->functions_count = 0;
  server->functions = NULL;
  server->handlers = NULL;

  // Set up socket
  server->socket = socket (AF_INET6, SOCK_STREAM, 0);

  // Reuse option
  int enable = 1;
  if (setsockopt
      (server->socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) == -1)
    {
      close (server->socket);
      free (server->functions);
      free (server->handlers);
      free (server);
      return NULL;
    }

  // Set up address
  struct sockaddr_in6 address;
  memset (&address, 0, sizeof (address));
  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons (port);

  // Binding
  if (bind (server->socket, (struct sockaddr *) &address, sizeof (address)) ==
      -1)
    {
      close (server->socket);
      free (server->functions);
      free (server->handlers);
      free (server);
      return NULL;
    }

  // Listen
  if (listen (server->socket, SOMAXCONN) == -1)
    {
      close (server->socket);
      free (server->functions);
      free (server->handlers);
      free (server);
      return NULL;
    }

  return server;
}

int
rpc_register (rpc_server * srv, char *name, rpc_handler handler)
{

  // Dynamically resize arrays for new added function
  srv->functions =
    (char **) realloc (srv->functions,
		       (srv->functions_count + 1) * sizeof (char *));
  srv->handlers =
    (rpc_handler *) realloc (srv->handlers,
			     (srv->functions_count +
			      1) * sizeof (rpc_handler));

  // Add function/handler to server
  srv->functions[srv->functions_count] = strdup (name);
  srv->handlers[srv->functions_count] = handler;
  srv->functions_count++;

  if (srv->functions_count >= MAX_FUNCTIONS)
    {
      free (srv->functions[srv->functions_count - 1]);
      srv->functions_count--;
      return -1;
    }

  return 0;
}

// Find if/where module exists on server
int
rpc_find_location (rpc_server * srv, char *name)
{
  for (int i = 0; i < srv->functions_count; i++)
    {
      if (strcmp (srv->functions[i], name) == 0)
	return i;
    }

  return -1;
}

void
rpc_serve_all (rpc_server * srv)
{

  while (1)
    {

      // Detect client message
      int client = accept (srv->socket, NULL, NULL);
      if (client < 0)
	continue;

      // Detect request type
      rpc_request_type request_type;
      recv (client, &request_type, sizeof (request_type), 0);

      if (request_type == FIND)
	{

	  // Ammend name of module in find request
	  size_t length;
	  recv (client, &length, sizeof (length), 0);
	  char *name = (char *) malloc (length + 1);
	  recv (client, name, length, 0);
	  name[length] = '\0';

	  // Send module existence to client
	  int location = rpc_find_location (srv, name);
      location = htonl(location);
	  send (client, &location, sizeof (location), 0);

	  free (name);
	}
      else if (request_type == CALL)
	{

	  // Receive handler location from client
	  rpc_data_location request;
	  recv (client, &request, sizeof (request), 0);

	  // Receive data format from client
	  recv (client, &request.data.data2_len,
		sizeof (request.data.data2_len), 0);
	  request.data.data2 = malloc (request.data.data2_len);
	  recv (client, request.data.data2, request.data.data2_len, 0);
	  if (request.location < 0
	      || request.location >= srv->functions_count)
	    {
	      close (client);
	      continue;
	    }

	  // Call the handler
	  rpc_handler handler = srv->handlers[request.location];
	  rpc_data *response = handler (&(request.data));
	  free (request.data.data2);

	  // Send confirmation to client
	  if (response != NULL)
	    {
	      send (client, response, sizeof (*response), 0);
	      rpc_data_free (response);
	    }
	}

      close (client);
    }
}

struct rpc_client
{
  char *ip;
  int port;
};

struct rpc_handle
{
  int location;
};

rpc_client *
rpc_init_client (char *addr, int port)
{
  rpc_client *client = malloc (sizeof (rpc_client));
  client->ip = strdup (addr);
  client->port = port;
  return client;
}

rpc_handle *
rpc_find (rpc_client * cl, char *name)
{

  // Create socket
  int socket_fd = socket (AF_INET6, SOCK_STREAM, 0);
  if (socket_fd < 0)
    {
      close (socket_fd);
      return NULL;
    }

  // Server address
  struct sockaddr_in6 address;
  memset (&address, 0, sizeof (address));
  address.sin6_family = AF_INET6;
  inet_pton (AF_INET6, cl->ip, &(address.sin6_addr));
  address.sin6_port = htons (cl->port);

  // Connect to server
  if (connect (socket_fd, (struct sockaddr *) &address, sizeof (address)) < 0)
    {
      close (socket_fd);
      return NULL;
    }

  // Send request type to server
  rpc_request_type request_type = FIND;
  send (socket_fd, &request_type, sizeof (request_type), 0);

  // Send module to server
  size_t length = strlen (name);
  send (socket_fd, &length, sizeof (length), 0);
  send (socket_fd, name, length, 0);

  // Receive module index
  int location;
  recv (socket_fd, &location, sizeof (location), 0);
  close (socket_fd);
  if (location == -1)
    {
      return NULL;
    }

  // Store index of function in handle, return handle
  rpc_handle *handle = (rpc_handle *) malloc (sizeof (rpc_handle));
  handle->location = location;
  return handle;
}

rpc_data *
rpc_call (rpc_client * cl, rpc_handle * h, rpc_data * payload)
{

  // Create socket
  int socket_fd = socket (AF_INET6, SOCK_STREAM, 0);
  if (socket_fd < 0)
    {
      close (socket_fd);
      return NULL;
    }

  // Server address
  struct sockaddr_in6 address;
  memset (&address, 0, sizeof (address));
  address.sin6_family = AF_INET6;
  inet_pton (AF_INET6, cl->ip, &(address.sin6_addr));
  address.sin6_port = htons (cl->port);

  // Connect to server
  if (connect (socket_fd, (struct sockaddr *) &address, sizeof (address)) < 0)
    {
      close (socket_fd);
      return NULL;
    }

  // Set up rpc data struct
  rpc_data_location request;
  request.data = *payload;
  request.location = h->location;

  // Send request type to server
  rpc_request_type request_type = CALL;
  send (socket_fd, &request_type, sizeof (request_type), 0);

  // Send correct data fields to server
  send (socket_fd, &request, sizeof (request), 0);
  send (socket_fd, &payload->data2_len, sizeof (payload->data2_len), 0);
  send (socket_fd, payload->data2, payload->data2_len, 0);

  // Get and return response
  rpc_data *response = (rpc_data *) malloc (sizeof (rpc_data));
  recv (socket_fd, response, sizeof (*response), 0);
  response->data1 = ntohl(response->data1);
  close (socket_fd);
  return response;
}

void
rpc_close_client (rpc_client * cl)
{
  if (cl == NULL)
    return;
  free (cl->ip);
  free (cl);
}

void
rpc_data_free (rpc_data * data)
{
  if (data == NULL)
    {
      return;
    }
  if (data->data2 != NULL)
    {
      free (data->data2);
    }
  free (data);
}