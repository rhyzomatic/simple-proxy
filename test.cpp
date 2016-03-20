#include<assert.h>
#include"myproxy.h"
#include<string>

using namespace std;

void test_get_content_length(){
	string str = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(44==get_content_length(str));
	assert(43==get_content_length(str));
}

int main(){
	test_get_content_length();
}
