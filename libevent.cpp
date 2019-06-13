#include "libevent.h"

libevent::libevent()
{
	listenfd = -1;
	ser_sockfd = -1;
	cli_sockfd = -1;
	epfd = epoll_create(3);
	assert(-1 != epfd);
}

libevent::~libevent()
{
	close(listenfd);
	close(epfd);
}

void libevent::socket_listen(char * ip, char * port)
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(-1 != listenfd);

	ifreq ifr;
	strcpy(ifr.ifr_name, "eth0");
	ioctl(listenfd, SIOCGIFADDR, &ifr);

	struct sockaddr_in ser;
	memset(&ser, 0, sizeof(ser));
	ser.sin_family = AF_INET;

	if( NULL == port )
	{
		ser.sin_port = 0;
	}
	else
	{
		ser.sin_port = htons(atoi(port));
	}

	if( NULL == ip )
	{
		ser.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}
	else
	{
		ser.sin_addr.s_addr = inet_addr(ip);
	}

	int res = bind(listenfd, (struct sockaddr*)&ser, sizeof(ser));
	assert(-1 != res);

    struct sockaddr_in tcp_addr;
	unsigned len = sizeof(tcp_addr);
	getsockname(listenfd, (struct sockaddr*)&tcp_addr, &len);


	std::cout << "IP  = " << inet_ntoa(tcp_addr.sin_addr) << std::endl;
    std::cout << "PORT= " << ntohs(tcp_addr.sin_port)<<std::endl;

	listen(listenfd, 5);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = listenfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
}

void libevent::sock_connect(char * ip, char * port)
{
	ser_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(-1 != ser_sockfd);

	ifreq ifr;
	strcpy(ifr.ifr_name, "eth0");
	ioctl(ser_sockfd, SIOCGIFADDR, &ifr);
	
	struct sockaddr_in ser;
	memset(&ser, 0, sizeof(ser));
	ser.sin_family = AF_INET;
	if( NULL == port )
	{
		ser.sin_port = 0;
	}
	else
	{
		ser.sin_port = htons(atoi(port));
	}

	if( NULL == ip )
	{
		ser.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}
	else
	{
		ser.sin_addr.s_addr = inet_addr(ip);
	}

	int res = connect(ser_sockfd, (struct sockaddr*)&ser, sizeof(ser));
	printf("%m \n",errno);
	assert(-1 != res);
}

void libevent::socket_run()
{
	while (1)
	{
		struct epoll_event events[100];
		int n = epoll_wait(epfd, events, 100, -1);
		if (n <= 0)
		{
			exit(0);
		}

		for(int i = 0; i < n; i++)
		{
			int fd = events[i].data.fd;
			if( fd == listenfd )
			{
				_connect_callback( fd );
			}
			else if(events[i].events & EPOLLRDHUP )
			{
				_disconect(fd);
			}
			else
			{
				_readable(fd);
			}
		}
	}
}

void libevent::add_event(epoll_event* event, int fd)
{
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event );
}

void libevent::change_endian(char* str, int len)
{
	char* ptem = new char[len];
	memcpy(ptem, str, len);
	for(int i = 0; i < len; i++)
	{
		str[i] = ptem[len - 1 - i];
	}
	delete[] ptem;
}
