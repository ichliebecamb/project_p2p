#pragma once
#include "libevent.h"
using CliMap = std::unordered_map< int , _DataStruct>;
class Server
{
public:
	Server(char* IP , char* Port );
	~Server();
	void run();

	void deal_connect(int fd );
	void deal_readable(int fd);
	void deal_disconnect(int fd);

	void Registerfd(int fd, int len);
	void GetOnLine(int fd);
	void GetOneUser(int fd, int len);
 //	struct sockaddr_in* find_name(char* name);
    void set_endian();
private:
	libevent _event;
	CliMap _map;
	char* _ip;
	char* _port;
	bool _is_little_endian;
};
