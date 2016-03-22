#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include "myproxy.h"
#include "utils.h"
#include <sys/file.h>

using namespace std;

string rec_header(int client_socket, size_t max_length = 2500){
	char payload [max_length];
	CL(payload,0);

	char *current_ptr = (char*) payload;
	int total_rec_length = 0;
	bool good_header = false;
	while (total_rec_length < max_length){
//		puts("WHAT WE HAVE NOW---");
//		puts(payload);
		ssize_t rec_char = recv(client_socket, current_ptr, 1, 0);
		if (rec_char < 1){
			printf("[%d] Error receiving any header.\n", client_socket);
			break;
		} else { // we know we got one byte
			total_rec_length++;
			current_ptr++;
			if (strcmp(current_ptr - 4, "\r\n\r\n") == 0){
				good_header = true;
				break;
			}
		}
	}

	if (good_header){
		string header(payload);
		return header;
	} else {
		puts("HEADER----");
		puts(payload);
		printf("[%d] Error receiving header.\n", client_socket);
		return "";
	}
}

void parse_remote_header(int client_socket, int ext_conn_socket, string url, bool cache, bool need_obj){


	//TODO:proxy-server connection option
	//TODO:chunked

	// grab the header from the remote server
	string header = rec_header(ext_conn_socket);
	int content_length = get_content_length(header);

	// recieve the body from remote
	char body [content_length];
	CL(body,0);
	//rec_all(ext_conn_socket, body, content_length); // no more rec_all, we need buffer

	send_all(client_socket, (unsigned char *) header.c_str(), header.length());

	int status = get_status_code(header);
	if (status==304 && need_obj){
		printf("[%d] 304 and need obj\n",client_socket);
		send_cache(client_socket, url);
		return;
	}

	FILE *file = NULL;
	if (status==200 && cache) {
		string enc = get_crypt(url);
		file = fopen((CACHE_DIR + enc).c_str(), "w+");
		flock(fileno(file), LOCK_EX);
		fwrite(header.c_str(), 1, header.length(), file);
	}

	unsigned char buf[BUF_SIZE];
	while (content_length > 0){
		printf("[%d] remain len %d\n",client_socket,content_length);
		int rec_char = rec_all(ext_conn_socket, buf, min(content_length, BUF_SIZE));
		if (rec_char < 1){
			error_handler("Error: error recieving payload."); //TODO: make this so it doesn't crash
		}
		send_all(client_socket, buf, rec_char); 
		
		if (status==200 && cache){
			fwrite(buf, 1, rec_char, file);
		}

		content_length -= rec_char;
	}
	printf("[%d] done parsing remote content\n",client_socket);
	if (status==200 && cache) fclose(file);
}



void open_ext_conn(int client_socket, string &header, char *hostname, int port, int content_length, bool cache, bool need_obj = false){
	// need_obj is only true for request case iii and iv

	printf("[%d] opening ext conn\n", client_socket);
	struct hostent* host;
	host = gethostname(hostname);
	struct sockaddr_in server_addr;
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	bcopy((char *) host -> h_addr, (char *) &server_addr.sin_addr.s_addr, host -> h_length);
	int ext_conn_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	printf("[%d] before connect\n", client_socket);
	int remote_socket = connect(ext_conn_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

	printf("[%d] after connect\n", client_socket);
	if (remote_socket < 0){
		printf("[%d] Could not connect to remote server", client_socket); // maybe change this so it doesn't exit the program
	} else {
		printf("[%d] Connected to remote\n",client_socket);
		char body [content_length];
		CL(body,0);
		rec_all(client_socket, (unsigned char *)body, content_length); // body now has the body of what the client is trying to send

		//send header and then body to remote server
		send_all(ext_conn_socket, (unsigned char *) header.c_str(), header.length());
		send_all(ext_conn_socket, (unsigned char *) body, content_length);

		// receive what the server sends
		parse_remote_header(client_socket, ext_conn_socket, get_url(header), cache, need_obj);
	}
	close(ext_conn_socket);
	
}


void pass_along_request(int client_socket, string &header){
	printf("[%d] would be passing it along\n", client_socket);
	//TODO: pass along
}


void parse_client_header(int client_socket, string &header){
	//cout << header << endl;
	if (header.substr(0,3) != "GET"){
		pass_along_request(client_socket, header);
	} else {
		istringstream header_stream(header);

		string address(get_url(header)); //TODO: check if this is legit... maybe first line doesn't necessarily have a space and the HTTP/1.1 or whatever
		cout << "[" << client_socket << "] " << address << "\n";

		
		string IMS = get_IMS(header);

		bool no_cache = get_cache(header);

		//cout << no_cache << "\n";

		pair<string,int> host = get_hostname_and_port(header);
		//cout << host.first << " " << host.second << "\n";
		int content_length = get_content_length(header);

		string url = get_url(header);

		bool will_cache = is_valid_ext(get_extension(header)); // handles 5 file types

		if (cache_exist(url)){ // cache exists YAY
			time_t cache_lmt = cache_LM(url);

			printf("[%d] Cache exist, sending cache\n", client_socket);
			if (IMS == "" && !no_cache){ //case i
				send_cache(client_socket, url);
			} else if (IMS != "" && !no_cache){ //case ii
				/*
				   MYPROXY checks if the IMS
				   time is later than the last modified time of the cached web object (see the hints below for how to
				   obtain the last modified time). If yes, MYPROXY returns a 304 (not modified) response to the
				   client; else it returns the cached object to the client.
				 */
				time_t ims_t = str_to_time(IMS);
				if (ims_t > cache_lmt) {
					string res = "HTTP/1.1 304 Not Modified\r\n\r\n";
					send_all(client_socket, (unsigned char *)res.c_str(), res.length());
				} else {
					send_cache(client_socket, url);
				}

			} else if (IMS == "" && no_cache){ // case iii
				/*
				   No If-Modified-Since and with Cache-Control: no-cache. MYPROXY will
				   forward the request to the web server. It will also insert the If-Modified-Since header to the
				   request, where the IMS time is set to be the last modified time of the cached web object.
				 */
				header = replace_IMS(header, time_to_str(cache_lmt));
				open_ext_conn(client_socket, header, (char *) host.first.c_str(), host.second, content_length, will_cache, true);
			} else { // case iv
				/*
				   With If-Modified-Since and with Cache-Control: no-cache. MYPROXY will
				   also forward the request to the web server. However, if the last modified time of the cached web
				   object is after the IMS time, then the IMS time will be overwritten with the last modified time of
				   the cached web object.
				 */
				time_t ims_t = str_to_time(IMS);
				if (cache_lmt > ims_t) {
					header = replace_IMS(header, time_to_str(cache_lmt));
				}
				open_ext_conn(client_socket, header, (char *) host.first.c_str(), host.second, content_length, will_cache, true);

			}

		} else { // NOPE, cache does not exist SOSAD
			open_ext_conn(client_socket, header, (char *) host.first.c_str(), host.second, content_length, will_cache);
		}

	}
}

void *connection_handler(void *client_socket_ptr){
	int client_socket = *(int*) client_socket_ptr;
	printf("[%d] Client connection accepted.\n", client_socket);
	int conn_status = 0;
	while(conn_status == 0){ //TODO: redundant while?
		string header = rec_header(client_socket);
		if (header != ""){ 
			parse_client_header(client_socket, header);
		} else {
			conn_status = -1;
		}
	}
	close(client_socket);
	printf("[%d] Client connection closed.\n", client_socket);
	pthread_exit(NULL);
	return 0;
}

#ifndef TEST
int main(int argc, char *argv[]){
	struct stat st = {0};

	if (stat("cache", &st) == -1) {
		mkdir("cache", 0700);
	}
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	long val = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(long)) == 1) {
		perror("setsockopt");
		exit(1);
	}
	int clisd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));
	if(bind(server_socket,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		printf("bind error: %s (Errno:%d)\n", strerror(errno), errno);
		exit(0);
	}
	if(listen(server_socket, 3) < 0){
		printf("listen error: %s (Errno:%d)\n", strerror(errno), errno);
		exit(0);
	}
	socklen_t client_addr_len = sizeof(client_addr);
	while(true){
		clisd = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
		int *clisd_cpy = new int(clisd);
		pthread_t new_thread;
		if (pthread_create(&new_thread, NULL, connection_handler, (void *)clisd_cpy)){
			perror("Could not spawn thread.\n");
			return 1;
		}
		void *status;
//		pthread_detach(new_thread);
//		pthread_join(new_thread,&status);
	}
	close(server_socket);
	return 0;
}
#endif
