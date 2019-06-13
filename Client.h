#pragma once
#include "libevent.h"
#include <dirent.h>

class Client
{
public:
	Client(char* ip, char* port);
	~Client();
	void run();

	void deal_connect(int fd);
	void deal_readable(int fd);
	void deal_cli_readable(int fd);
	void deal_disconnect(int fd);

	void deal_cli(int fd);
	void send_filelist(int fd);
	void get_filelist();
	void send_file(int fd, int len);
	void get_file();

	void Regist();
	void GetOnLine();
	bool GetOneUser();

	void deal_cli_input(char* cmd);
	void set_endian();

private:
	void printf_info_ser();
	void printf_info_cli();
	bool deal_input(char* cmd);

	libevent _event;
	char* _ip;
	char* _port;
	bool _is_little_endian;
};

