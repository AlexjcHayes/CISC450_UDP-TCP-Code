CC= /usr/bin/gcc

all:	tcpclient group10-server 

tcpclient: tcpclient.c;
	${CC} tcpclient.c -o tcpclient

group10-server: tcpserver.c;
	${CC} tcpserver.c -o group10-server

clean:
	rm tcpclient group10-server
