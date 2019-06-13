#include "Server.h"


Server::Server(char * IP, char * Port)
	:_ip(IP)
	,_port(Port)
{
	set_endian();
}

Server::~Server()
{
}

void Server::run()
{
	_event.socket_listen(_ip, _port);
	_event.set_connect_callback(
			std::bind(&Server::deal_connect, this, std::placeholders::_1));

	_event.set_readable_callback(
			std::bind(&Server::deal_readable, this, std::placeholders::_1));

	_event.set_disconnect_callback(
			std::bind(&Server::deal_disconnect, this, std::placeholders::_1));

	_event.socket_run( );
}

void Server::deal_connect(int fd)
{
	sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int c = accept(_event.get_listenfd(),
			(struct sockaddr*)&cli, &len);
	assert(-1 != c );

	epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = c;

	_event.add_event(&event, c);
}

void Server::deal_readable(int fd)
{
	CMD_DATA cmd;
	int n = recv(fd, &cmd, sizeof(cmd), 0);
    assert( n != -1);
	if( n <= 0 )
	{
		deal_disconnect(fd);
	}


	switch (cmd._type)
	{
		case REGISTERED:
			Registerfd(fd, cmd._len);
			break;
		case CLI_LIST:
			GetOnLine(fd);
			break;
		case CLI_IP:
			GetOneUser(fd, cmd._len);
			break;
		default:
			break;
	}
}

void Server::deal_disconnect(int fd)
{
	// 删除客户端信息
	close(fd);
	_map.erase(fd);
	_event.del_event(fd);
}

void Server::Registerfd(int fd, int len)
{
	CMD_DATA cmd;
	_DataStruct data;
	memset(&data, sizeof(data), 0);
	int	i = recv(fd, &data, len , 0);
//	printf("%m \n",errno);
	assert( i!= -1);

	auto it = _map.find(fd);
	if( it != _map.end())
	{
		cmd._type = ERR;
		cmd._len = 0;
	}
	else
	{
		cmd._type = OK;
		cmd._len = 0;
		_map[fd] = data;
	}
	send(fd, &cmd, sizeof(cmd), 0);
}

void Server::GetOnLine(int fd)
{
	int n = _map.size();
	if( _map.find(fd) != _map.end())
	{
		n--;
	}

	CMD_DATA cmd;
	cmd._type = _CLI_LIST;
	cmd._len = n;
	send(fd, &cmd, sizeof(cmd), 0);

	if( n == 0 ) return;

	auto it = _map.begin();
	auto aim = _map.find(fd);
	char name[21] = {0};
	for(; it !=  _map.end(); it++)
	{
		if( it == aim ) continue;
		memset(name, 0, 21);
		strcpy(name, it->second.user_name);
		send(fd, name, 21, 0);
	}
}

void Server::GetOneUser(int fd, int len)
{
	char aimname[21] = {0};
	int n = recv(fd, aimname, len, 0 );
	auto it = _map.begin();
	auto aim = _map.find(fd);
	sockaddr_in aim_addr;
	CMD_DATA cmd;
	cmd._type = _CLI_IP;
	cmd._len = 0;
	for(; it != _map.end(); it++)
	{
		if( strncmp(aimname, it->second.user_name, len) == 0 && it != aim)
		{
			cmd._len = sizeof( aim_addr );
			aim_addr = it->second.tcp_addr;
			break;
		}
	}

	send(fd, &cmd, sizeof(cmd), 0);
	if( cmd._len == 0 ) return;
	send(fd, &aim_addr, sizeof( aim_addr ), 0);
}

void Server::set_endian()
{
	int testdata = 0x41424344;
	char* c = (char*)&testdata;
	if( *c == 'A')
	{
		_is_little_endian = false;
	}
	else
	{
		_is_little_endian = true;
	}
}
