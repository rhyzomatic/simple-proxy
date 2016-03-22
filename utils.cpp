// utils.cpp
//
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
#include <sys/file.h>
#include "utils.h"

using namespace std;

void error_handler (string error_string){
	perror(error_string.c_str());
	exit(0);
}

int send_all (int client_socket, unsigned char * buffer, int length){
	unsigned char *current_ptr = buffer;
	while (length > 0){
		int sent_char = send(client_socket, current_ptr, length, MSG_NOSIGNAL);
		if (sent_char < 0){
			return -1;
		}
		current_ptr += sent_char;
		length -= sent_char;
	}
	return 0;
}

int rec_all (int client_socket, unsigned char * payload, int length){
	unsigned char *current_ptr = payload;
	int len = 0;
	while (length > 0){
		int rec_char = recv(client_socket, current_ptr, length, 0);
		if (rec_char < 1){
			printf("Error: error recieving payload.\n");
			return -1;
		}
		current_ptr += rec_char;
		len += rec_char;
		length -= rec_char;
	}
	return len;
}

int get_content_length (string header){
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

string get_crypt (string & url){
	crypt_data data;
	data.initialized = 0;
	char *enc;
	enc = crypt_r(url.c_str(), "$1$00$", &data);
	string s = string(enc + 6);
	replace(s.begin(), s.end(), '/', '_');
	replace(s.begin(), s.end(), '.', '-');
	return s;
}

bool cache_exist (string & url){
	return (access((CACHE_DIR+get_crypt(url)).c_str(), F_OK) != -1); //file exist	
}

void send_cache (int client_socket, string url){
	FILE *file = fopen((CACHE_DIR+get_crypt(url)).c_str(), "r");
	flock(fileno(file), LOCK_EX);
	unsigned char buf[BUF_SIZE];
	int size;
	while (size = fread(buf,1,BUF_SIZE,file)){
		send_all(client_socket, buf, size);
	}
	fclose(file);
}

pair <string,int> get_hostname_and_port (string & header){
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

string get_IMS (string & header){
	size_t IMS_location = header.find("If-Modified-Since");
	string IMS;

	if (IMS_location != string::npos){ 
		IMS_location += 19; // length of If-Modified-Since string
		IMS = header.substr(IMS_location, header.find("\r\n", IMS_location) - IMS_location);
		cout << IMS_location << "\n";
	}
	
	cout << IMS << "\n";
	return IMS;
}


string get_LM (string & header){
	size_t LM_location = header.find("Last-Modified: ");
	string LM;

	if (LM_location != string::npos){ 
		LM_location += 15; // length of "Last-Modified: "
		LM = header.substr(LM_location, header.find("\r\n", LM_location) - LM_location);
		//cout << LM_location << "\n";
	}
	//cout << LM << "\n";
	return LM;
}


bool change_IMS (string & header){
	bool change = false;
	string IMS = get_IMS(header);
	string LM = get_LM(header);
	if(IMS.compare(LM) != 0){
	 change = true;
	}
	return change;
}


bool get_cache (string & header){
	size_t CC_location = header.find("Cache-Control");
	string CC;
	bool no_cache = false;
	if (CC_location != string::npos){
		CC_location += 15; // length of "Cache-Control: "
		CC = header.substr(CC_location, header.find("\r\n", CC_location) - CC_location);
		if (CC.find("no-cache") != string::npos){
			no_cache = true;
		}
	}
	cout << CC << "\n";
	return no_cache;
}


int get_status_code (string header){
	size_t status_location = header.find("HTTP/1.1 ");

	status_location += 9; // length of "HTTP/1.1 "
	string length_str(header.substr(status_location,3));
	int status;
	stringstream(length_str) >> status;
	cout << status << "\n";
	return status;
	
}


string get_url (string & header){
	size_t space_ind = header.find(" ", 4);
	string url = header.substr(4, space_ind - 4 + 1 - 1); // exclude ending space
	return url;
}


string get_extension (string & header){
	string url = get_url(header);
	size_t dot_ind = url.find_last_of(".");
	size_t bs_ind = url.find_last_of("/");
	string ext;
	if (bs_ind != string::npos && dot_ind > bs_ind && dot_ind != string::npos){
		ext = url.substr(dot_ind + 1);
	//	cout<<ext<<endl;
	}
	return ext;
}

bool is_valid_ext (string ext){
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	string exts[] = {"html", "pdf" ,"jpg", "gif","txt"};
	for (int i=0,len=sizeof(exts)/sizeof(exts[0]);i<len;i++){
		if (exts[i]==ext) return true;
	}
	return false;
}

time_t cache_LM(string url){
	struct stat buf;
	time_t LMT;
	string filename = get_crypt(url);
	if((stat((CACHE_DIR+filename).c_str(),&buf))!=0){
		if(errno != ENOENT){
			perror("Error");
		}else{
			//Cache not exists
		}
	}else{
		memcpy(&(LMT),&buf.st_mtime,sizeof(time_t));
	}
	return LMT;
}

time_t str_to_time(string time){
	struct tm buf;
	time_t IMS;
	strptime(time.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &buf);
	IMS = mktime(&buf);
	return IMS;
}
