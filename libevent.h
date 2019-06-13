#pragma once
#include "command.h"
#include <iostream>
#include <unordered_map>
#include <string.h>

#include <assert.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <functional>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netdb.h>

#include <netdb.h>

#include <errno.h>

class libevent
{
public:
	libevent();
	~libevent();
	
	void socket_listen(char* ip, char* port); 
	void sock_connect(char* ip, char* port);  

	void set_connect_callback(std::function<void( int)> fun)
	{
		_connect_callback = fun;
	}
	void set_readable_callback(std::function<void( int)> fun)
	{
		_readable = fun;
	}
	void set_disconnect_callback(std::function<void( int)> fun)
	{
		_disconect = fun;
	}
	void socket_run( );

	int get_listenfd()   { return listenfd; } 
	int get_socketfd()   { return ser_sockfd; }
	void set_cli_sockfd(int fd ) { cli_sockfd = fd; }
	int get_cli_sockfd() { return cli_sockfd; }
	
	void add_event(epoll_event * event, int fd);
	void del_event(int fd)
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
	}

	void change_endian(char* str, int len);

private: 
	int listenfd;  
	int ser_sockfd;   
	int cli_sockfd;
	int epfd; 

	std::function<void( int)> _connect_callback;
	std::function<void( int)> _readable;
	std::function<void( int)> _disconect;

};
