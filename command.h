#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>
enum CMD
{
	REGISTERED,       //请求注册
	_REGISTERED,      //回复注册信息
	CLI_LIST,         //请求客户列表
    _CLI_LIST,        // 回复客户列表
	CLI_IP,           // 请求客户ip
	_CLI_IP,          // 回复客户ip

	FILE_LIST,        // 请求文件列表
	_FILE_LIST,       // 回复文件列表
	DOWN,             // 请求下载
	_DOWN,
	PLAY,             // 请求播放

	OK,               // 状态 成功
	ERR,              // 状态 失败
	EXIT              // 退出
};

struct CMD_DATA
{
	CMD _type;
	int _len;
};

struct _DataStruct
{
	sockaddr_in tcp_addr;
	char user_name[24];
};


