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


void error_handler (std::string error_string);
int send_all (int client_socket, unsigned char * buffer, int length);
int rec_all (int client_socket, char * payload, int length);
std::string rec_header (int client_socket, size_t max_length);
int get_content_length (std::string header);
void parse_remote_header (int client_socket, int ext_conn_socket);
void open_ext_conn (int client_socket, std::string & header, char * hostname, int port, int content_length );
std::pair <std::string,int> get_hostname_and_port (std::string & header);
void pass_along_request (int client_socket, std::string & header);
std::string get_IMS (std::string & header);
bool get_cache (std::string & header);
void parse_client_header (int client_socket, std::string & header);
void * connection_handler (void * client_socket_ptr);
std::string get_url(std::string &header);
std::string get_extension(std::string &header);
bool is_valid_ext(std::string ext);


#endif
