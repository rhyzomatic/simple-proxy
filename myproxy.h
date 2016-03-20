// myproxy.h
//

#ifndef LZZ_myproxy_h
#define LZZ_myproxy_h
#define LZZ_INLINE inline
void error_handler (string error_string);
int send_all (int client_socket, unsigned char * buffer, int length);
int rec_all (int client_socket, char * payload, int length);
string rec_header (int client_socket, size_t max_length = 1000);
int get_content_length (string header);
void parse_remote_header (int client_socket, int ext_conn_socket);
void open_ext_conn (int client_socket, string & header, char * hostname, int port, int content_length = 0);
pair <string,int> get_hostname_and_port (string & header);
void pass_along_request (int client_socket, string & header);
void parse_client_header (int client_socket, string & header);
void * connection_handler (void * client_socket_ptr);
int main (int argc, char * (argv) []);
#undef LZZ_INLINE
#endif
