// utils.h
//

#ifndef LZZ_utils_h
#define LZZ_utils_h

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
#define CL(x,y) memset(x,y,sizeof(x))
#define FUCK puts("FUCK");
#define BUF_SIZE 4096
#define CACHE_DIR "cache/"

void error_handler (std::string error_string);
int send_all (int client_socket, unsigned char * buffer, int length);
int rec_all (int client_socket, unsigned char * payload, int length);
int get_content_length (std::string header);
std::string get_crypt (std::string & url);
bool cache_exist (std::string & url);
void send_cache (int client_socket, std::string url);
std::pair <std::string,int> get_hostname_and_port (std::string & header);
std::string get_IMS (std::string & header);
std::string get_LM (std::string & header);
std::string replace_IMS (std::string header, std::string new_IMS);
bool get_cache (std::string & header);
int get_status_code (std::string header);
std::string get_url (std::string & header);
std::string get_extension (std::string & header);
bool is_valid_ext (std::string ext);
bool is_using_chunked_encoding(std::string header);
time_t cache_LM(std::string url);
time_t str_to_time(std::string time);
struct hostent *gethostname (char *host);
std::string time_to_str(time_t IMS);

#endif
