// myproxy.h
//

#ifndef LZZ_myproxy_h
#define LZZ_myproxy_h

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <string>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <crypt.h>
#include <algorithm>



std::string rec_header (int client_socket, size_t max_length);
//void parse_remote_header (int client_socket, int ext_conn_socket, std::string url, bool cache);
//void open_ext_conn (int client_socket, std::string & header, char * hostname, int port, int content_length, bool cache);
void pass_along_request (int client_socket, std::string & header);
void parse_client_header (int client_socket, std::string & header);
void * connection_handler (void * client_socket_ptr);



#endif
