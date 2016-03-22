#include<assert.h>
#include"myproxy.h"
#include"utils.h"
#include<cstdio>
#include<string>
#define FUCK puts("FUCK");
using namespace std;

void test_get_content_length(){
	string str = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(44==get_content_length(str));

	string str2 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(0==get_content_length(str2));

	string str3 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 256\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(256==get_content_length(str3));
}

void test_get_cache(){
	string str3 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nCache-Control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(true==get_cache(str3)); 

	string str4 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nCache-Control: max-age=0, max-age=1, no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(true==get_cache(str4));

	string str6 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nCache-Control: max-age=0\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(false==get_cache(str6));

	string str5 = "HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert(false==get_cache(str5)); 
}


void test_get_IMS(){
	
	string str4 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Wed, 19 Oct 2005 10:50:00 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "Wed, 19 Oct 2005 10:50:00 GMT" == get_IMS(str4)); 

	string str5 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "Sat, 29 Oct 1994 19:43:31 GMT" == get_IMS(str5));

	//no IMS
	string str6 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "" == get_IMS(str6)); 

}

void test_get_LM(){

	string str4 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Wed, 19 Oct 2005 10:50:00 GMT\r\nLast-Modified: Wed, 19 Oct 2005 10:50:00 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "Wed, 19 Oct 2005 10:50:00 GMT" == get_LM(str4)); 

	string str5 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Wed, 19 Oct 2005 10:50:00 GMT\r\nLast-Modified: Sun, 20 Oct 2017 10:50:11 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "Sun, 20 Oct 2017 10:50:11 GMT" == get_LM(str5)); 

}


void test_get_hostname_and_port(){
	string str5 = "HTTP/1.1 200 OK\r\nHost: www.cse.cuhk.edu.hk:80\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( (pair<string,int> ("www.cse.cuhk.edu.hk",80) )== get_hostname_and_port(str5));
	
	string str6 = "HTTP/1.1 200 OK\r\nHost: www.cse.cuhk.edu.hk/index.html:8000\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( (pair<string,int> ("www.cse.cuhk.edu.hk/index.html",8000) )== get_hostname_and_port(str6));
	
	string str8 = "HTTP/1.1 200 OK\r\nHost: www.cse.cuhk.edu.hk\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( (pair<string,int> ("www.cse.cuhk.edu.hk",80) )== get_hostname_and_port(str8));
	
}

void test_get_url(){
	string str = "GET http://www.cse.cuhk.edu.hk/v7/javascripts/marquee_i.js HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "http://www.cse.cuhk.edu.hk/v7/javascripts/marquee_i.js" == get_url(str));

	string str2 = "GET http://www.cse.cuhk.edu.hk/~kychan4 HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "http://www.cse.cuhk.edu.hk/~kychan4" ==get_url(str2));

	
}

void test_get_extension(){
	string str = "GET http://www.cse.cuhk.edu.hk/v7/javascripts/marquee_i.js HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "js" == get_extension(str));

	string str2 = "GET http://www.cse.cuhk.edu.hk/~kychan4/index.html HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "html" == get_extension(str2));

	string str3 = "GET http://www.cse.cuhk.edu.hk/~kychan4/index.js HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "js" == get_extension(str3));

	string str4 = "GET http://www.cse.cuhk.edu.hk/~kychan4 HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "" == get_extension(str4));

	string str5 = "GET http://www.cse.cuhk.edu.hk/~kychan4/index.gif HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "gif" == get_extension(str5));

	string str6 = "GET http://www.cse.cuhk.edu.hk/index.txt HTTP/1.1 200 OK\r\nDate: Sun, 18 Oct 2009 08:56:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( "txt" == get_extension(str6));
}

void test_is_valid_ext(){
	//js 
	assert( false == is_valid_ext("js"));
	//no extension
	assert( false == is_valid_ext(""));
	//jpg
	assert( true == is_valid_ext("jpg"));
	//gif
	assert( true == is_valid_ext("gif"));
	//txt
	assert( true == is_valid_ext("txt"));
	//pdf 
	assert( true == is_valid_ext("pdf"));
	//html
	assert( true == is_valid_ext("html"));

}

void test_get_status_code(){

	string str4 = "HTTP/1.1 200 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Wed, 19 Oct 2005 10:50:00 GMT\r\nLast-Modified: Wed, 19 Oct 2005 10:50:00 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( 200 == get_status_code(str4)); 

	string str5 = "HTTP/1.1 404 OK\r\nServer: Apache/2.2.14 (Win32)\r\nIf-Modified-Since: Wed, 19 Oct 2005 10:50:00 GMT\r\nLast-Modified: Wed, 20 Oct 2005 10:50:00 GMT\r\nAccept-Ranges: bytes\r\nCache-control: no-cache\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>";
	assert( 404 == get_status_code(str5)); 

	

}


void test_str_to_time(){
	string str = "Wed, 19 Oct 2005 10:50:00 GMT";
	assert(1129690200 == str_to_time(str)); 
}

void test_time_to_str(){
	time_t IMS = 1129690200;
	assert("Wed, 19 Oct 2005 10:50:00 GMT" == time_to_str(IMS)); 
}




int main(){

	test_get_content_length();
	test_get_cache();
	test_get_IMS();
	test_get_hostname_and_port();
	test_get_url();
	test_get_extension();
	test_is_valid_ext();
	test_get_LM();
	//test_replace_IMS();
	test_get_status_code();
	test_str_to_time();
	test_time_to_str();
}

