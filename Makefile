CFLAGS = -lpthread -Wall
LIB = 

all:
	make clean
	make myproxy

myproxy: 
	g++ -o myproxy myproxy.cpp ${CFLAGS} ${LIB} -I.

test:
	make clean
	g++ -o test test.cpp myproxy.cpp ${CFLAGS} ${LIB} -I. -D TEST
	
clean:
	rm -f myproxy test
