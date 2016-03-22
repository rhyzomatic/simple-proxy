CFLAGS = -lpthread -lcrypt -Wall
LIB = 

all:
	make clean
	make myproxy
	make test

myproxy: 
	g++ -o myproxy myproxy.cpp utils.cpp ${CFLAGS} ${LIB} -I.

test:
	g++ -o test test.cpp myproxy.cpp utils.cpp ${CFLAGS} ${LIB} -I. -D TEST
	
clean:
	rm -f myproxy test
