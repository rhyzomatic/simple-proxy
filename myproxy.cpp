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
#include "myproxy.h"
#define CL(x,y) memset(x,y,sizeof(x))
#define FUCK puts("FUCK");

using namespace std;

void error_handler(string error_string){
	perror(error_string.c_str());
	exit(0);
}

int send_all(int client_socket, unsigned char *buffer, int length){
	char *current_ptr = (char*) buffer;
	while (length > 0){
		int sent_char = send(client_socket, current_ptr, length, 0);
		if (sent_char < 0){
			return -1;
		}
		current_ptr += sent_char;
		length -= sent_char;
	}
	return 0;
}


int rec_all(int client_socket, char *payload, int length){
	char *current_ptr = (char*) payload;
	while (length > 0){
		int rec_char = recv(client_socket, current_ptr, length, 0);
		if (rec_char < 1){
			printf("Error: error recieving payload.\n");
			return -1;
		}
		current_ptr += rec_char;
		length -= rec_char;
	}
	return 0;
}

string rec_header(int client_socket, size_t max_length = 1000){
	char payload [max_length];
	CL(payload,0);

	char *current_ptr = (char*) payload;
//	int total_rec_length = 0;
	bool good_header = false;
	while (true){ //(total_rec_length < max_length){
		ssize_t rec_char = recv(client_socket, current_ptr, 1, 0);
		if (rec_char < 1){
			printf("Error receiving any payload.\n");
			break;
		}
		else { // we know we got one byte
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
	}
	else {
		printf("Error receiving header.\n");
		return "";
	}
}

int get_content_length(string header){
	size_t content_length_location = header.find("Content-Length: ");
	if (content_length_location == string::npos){ // not found, must be 0
		return 0;
	}
	else {
		content_length_location += 16; // length of "Content-Length: "
		string length_str(header.substr(content_length_location, (header.find("\r\n", content_length_location) - content_length_location)));
		int content_length;
		stringstream(length_str) >> content_length;
		return content_length;
	}
}


void parse_remote_header(int client_socket, int ext_conn_socket){
	// grab the header from the remote server
	string header = rec_header(ext_conn_socket);
	int content_length = get_content_length(header);

	// recieve the body from remote
	char body [content_length];
	CL(body,0);
	rec_all(ext_conn_socket, body, content_length);

	// pass header and body along to client
	send_all(client_socket, (unsigned char *) header.c_str(), header.length());
	send_all(client_socket, (unsigned char *)body, content_length);
}



void open_ext_conn(int client_socket, string &header, char *hostname, int port, int content_length = 0){
	struct hostent* host;
	host = gethostbyname(hostname);
	struct sockaddr_in server_addr;
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	bcopy((char *) host -> h_addr, (char *) &server_addr.sin_addr.s_addr, host -> h_length);
	int ext_conn_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int remote_socket = connect(ext_conn_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if (remote_socket < 0){
		error_handler("Could not connect to remote server"); // maybe change this so it doesn't exit the program
	}
	else {
		cout << "Connected to remote\n";
		char body [content_length];
		CL(body,0);
		rec_all(ext_conn_socket, body, content_length); // body now has the body of what the client is trying to send

		//send header and then body to remote server
		send_all(ext_conn_socket, (unsigned char *) header.c_str(), header.length());
		send_all(ext_conn_socket, (unsigned char *) body, content_length);

		// receive what the server sends
		parse_remote_header(client_socket, ext_conn_socket);
	}
}

pair<string,int> get_hostname_and_port(string &header){
	size_t hostname_location = header.find("Host: ");
	if (hostname_location == string::npos){
		error_handler("Could not extract hostame from header");
	}
	else {
		size_t end_pos = header.find("\r\n", hostname_location);
		size_t port_pos = header.find(":", hostname_location + 6); // skip the http://
		int port = 80;
		if (port_pos != string::npos && port_pos < end_pos) {// port number exist
			end_pos = port_pos; 
			port = atoi(header.c_str() + port_pos + 1);
			printf("THIS IS PORT %d\n",port);
		}

		string hostname = header.substr(hostname_location + 6, end_pos - hostname_location - 6);
		return make_pair(hostname,port);
	}
}

void pass_along_request(int client_socket, string &header){
	printf("would be passing it along\n");
	//TODO: pass along
}

void parse_client_header(int client_socket, string &header){
	//TODO: 5 special file types
	cout << header << endl;
	if (header.substr(0,3) != "GET"){
		pass_along_request(client_socket, header);
	}
	else {
		istringstream header_stream(header);

		string address(header.substr(4, header.find(" ", 4)-4)); //TODO: check if this is legit... maybe first line doesn't necessarily have a space and the HTTP/1.1 or whatever
		cout << address << "\n";

		size_t IMS_location = header.find("If-Modified-Since");
		size_t CC_location = header.find("Cache-Control");
		string IMS("");
		bool no_cache = false;

		if (IMS_location != string::npos){ //TODO: this is untested
			IMS_location += 17; // length of If-Modified-Since string
			IMS = header.substr(IMS_location, header.find("\r\n", IMS_location) - IMS_location);
			cout << IMS_location << "\n";
		}

		if (CC_location != string::npos){
			CC_location += 15; // length of "Cache-Control: "
			string CC(header.substr(CC_location, 8)); // 8 is length of no-cache
			if (CC == "no-cache"){
				no_cache = true;
			}
		}

		cout << no_cache << "\n";

		pair<string,int> host = get_hostname_and_port(header);
		cout << host.first << " " << host.second << "\n";
		int content_length = get_content_length(header);
		if (true){ //TODO: cache condition
			open_ext_conn(client_socket, header, (char *) host.first.c_str(), host.second, content_length);
			//TODO: MUTEX
		}

	}
}

void *connection_handler(void *client_socket_ptr){
	int client_socket = *(int*) client_socket_ptr;
	printf("Client connection accepted.\n");
	int conn_status = 0;
	while(conn_status == 0){ //TODO: redundant while?
		string header = rec_header(client_socket);

		parse_client_header(client_socket, header);

		conn_status = -1;
	}
	close(client_socket);
	printf("Client connection closed.\n");
	return 0;
}

#ifndef TEST
int main(int argc, char *argv[]){
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	long val = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(long)) == 1) {
		perror("setsockopt");
		exit(1);
	}
	int client_socket;
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
	while(client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len)){
		pthread_t new_thread;
		void *new_socket = malloc(sizeof(int));
		memcpy(new_socket, &client_socket, sizeof(int));
		if (pthread_create(&new_thread, NULL, connection_handler, new_socket)){
			perror("Could not spawn thread.\n");
			return 1;
		}
	}
	close(server_socket);
	return 0;
}
#endif
