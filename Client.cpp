#include "Client.h"

Client::Client(char* ip, char* port)
	:_ip(ip)
	 ,_port(port)
{
	set_endian();
}

Client::~Client()
{
}

void Client::run()
{
	_event.sock_connect(_ip, _port);

	_event.socket_listen(NULL, NULL);
	printf_info_ser();
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = STDIN_FILENO;

	_event.add_event(&event, STDIN_FILENO);

	_event.set_connect_callback(
			std::bind(&Client::deal_connect, this, std::placeholders::_1));

	_event.set_readable_callback(
			std::bind(&Client::deal_readable, this, std::placeholders::_1));

	_event.set_disconnect_callback(
			std::bind(&Client::deal_disconnect, this, std::placeholders::_1));

	_event.socket_run(  );
}

void Client::deal_connect(int fd)
{
	sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int c = accept(_event.get_listenfd(),
			(struct sockaddr*)&cli, &len);
	assert( c != -1);

	epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = c;

	_event.add_event(&event, c);
}

void Client::deal_readable(int fd)
{
	if( fd ==  STDIN_FILENO )
	{
    	char cmd[128] = {0};
    	fgets(cmd, 127, stdin);
    	bool sig = deal_input(cmd);
        if( sig && (strncmp(cmd, "3", 1) == 0 || strncmp(cmd, "chooseuser", 10) == 0))
    	{
	    	return;
    	}
	    printf_info_ser();
	}
	else
	{
		deal_cli( fd );
	}
//	printf_info_ser();

}

void Client::deal_cli_readable(int fd)
{
	char cmd[128] = {0};
	fgets(cmd, 127, stdin);
	deal_cli_input(cmd);
	if( strncmp(cmd, "4", 1) == 0 || strncmp(cmd, "return", 6) == 0)
	{
		_event.set_readable_callback(
			std::bind(&Client::deal_readable, this, std::placeholders::_1));
		printf_info_ser();
		return;
	}
	printf_info_cli();
}

void Client::deal_disconnect(int fd)
{
	close(fd);
	_event.del_event(fd);
	_event.set_cli_sockfd( -1 );
}

void Client::printf_info_ser()
{
	std::cout << "*********************" << std::endl;
	std::cout << "1 register           " << std::endl;
	std::cout << "2 userlist           " << std::endl;
	std::cout << "3 chooseuser         " << std::endl;
	std::cout << "4 exit               " << std::endl;
	std::cout << "*********************" << std::endl;
	std::cout << "please input the num:";
	fflush(stdout);
}

void Client::printf_info_cli()
{
	std::cout << "*********************" << std::endl;
	std::cout << "1 file list          " << std::endl;
	std::cout << "2 down               " << std::endl;
	std::cout << "3 play               " << std::endl;
	std::cout << "4 return             " << std::endl;
	std::cout << "*********************" << std::endl;
	std::cout << "please input the num:";
	fflush(stdout);
}

bool Client::deal_input(char* cmd)
{
	bool sig  = true;
	int out = -2;
	if(strncmp(cmd, "1", 1) == 0 || strncmp(cmd, "register", 8 ) == 0 )
	{
		out = REGISTERED;
	}
	if(strncmp(cmd, "2", 1) == 0 || strncmp(cmd,"userlist", 8 ) == 0)
	{
		out = CLI_LIST;
	}
	if( strncmp(cmd, "3", 1) == 0 || strncmp(cmd, "chooseuser", 10) == 0 )
	{
		out = CLI_IP;
	}
	if( strncmp(cmd, "4", 1) == 0 || strncmp(cmd, "exit", 4) == 0)
	{
		out = EXIT;
	}

	switch( out )
	{
		case REGISTERED:
			Regist();
			break;
	    case CLI_LIST:
		     GetOnLine();
			break;
	    case CLI_IP:
		    sig = GetOneUser();
			break;
		case EXIT:
			exit(0);
			break;
	    default:
		    break;
	}
	return sig;
}

void Client::deal_cli_input(char* cmd)
{
	int out = -2;
	if(strncmp(cmd, "1", 1) == 0 || strncmp(cmd, "file", 4 ) == 0 )
	{
		out = FILE_LIST;
	}
	if(strncmp(cmd, "2", 1) == 0 || strncmp(cmd,"down", 4 ) == 0)
	{
		out = DOWN;
	}
	if( strncmp(cmd, "3", 1) == 0 || strncmp(cmd, "play", 4) == 0 )
	{
		out = PLAY;
	}

	switch( out )
	{
		case FILE_LIST:
			get_filelist();
			break;
	    case DOWN:
			get_file();
			break;
	    case PLAY:
			break;
	    default:
		    break;
	}
}

void Client::Regist()
{
	int fd = _event.get_socketfd();
	char myName[21] = { 0 };

	fgets(myName, 20, stdin);
    myName[strlen(myName) - 1] = 0;

	_DataStruct data;
	strcpy(data.user_name, myName);

	struct sockaddr_in tcp_addr;
	unsigned len = sizeof(tcp_addr);
	getsockname(_event.get_listenfd(), (struct sockaddr*)&tcp_addr, &len);
	data.tcp_addr = tcp_addr;

	CMD_DATA cmd;
	cmd._type = REGISTERED;
	cmd._len = sizeof(data);

    int n = send(fd, &cmd, sizeof(cmd), 0);
    assert( n != -1);
	n = send(fd, &data, sizeof(data), 0);
	assert( n != -1);

    n = recv(fd, &cmd, sizeof(cmd), 0);
	assert( n != -1);
	if( cmd._type == OK)
	{
		std::cout << "Regist Success" << std::endl;
	}
	else
	{
		std::cout << "Regist Fault" << std::endl;
	}
}

void Client::GetOnLine()
{
	CMD_DATA cmd;
	cmd._type = CLI_LIST;
	cmd._len = 0;
	int fd = _event.get_socketfd();
	int n = 0;
	n = send(fd, &cmd, sizeof(cmd), 0);
	assert( n != -1 );
	n = recv(fd, &cmd, sizeof(cmd), 0);
	assert( n != -1);
	int count = cmd._len;
	char name[21] = {0};
	for(int i = 0; i < count ; i++)
	{
		memset(name, 0, 21);
		n = recv(fd, name, 21, 0);
		assert( n != -1);
		std::cout << name << std::endl;
	}
}

bool Client::GetOneUser()
{
	int fd = _event.get_socketfd();
	char aimname[21] = { 0 };
	fgets(aimname, 20, stdin);

	aimname[ strlen(aimname) - 1 ] = 0;
	CMD_DATA cmd;
	cmd._type = CLI_IP;
	cmd._len = strlen(aimname);
    int n =  send(fd, &cmd, sizeof(cmd), 0);
	assert(n != -1);
    n =	send(fd, aimname, sizeof(aimname), 0);
	assert( n != -1);

	struct sockaddr_in aim_addr;
    n = recv(fd, &cmd, sizeof(cmd), 0);
	assert( n != -1 );
	if (cmd._len == 0)
	{
		printf("Not Found!\n");
		return false;
	}
    n = recv(fd, &aim_addr, cmd._len, 0);
	assert( n != -1 );

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(-1 != sockfd);

	int res = connect(sockfd, (struct sockaddr*)&aim_addr, sizeof(aim_addr));
	assert( -1 != res );

	_event.set_cli_sockfd(sockfd);
    _event.set_readable_callback(
		std::bind(&Client::deal_cli_readable, this, std::placeholders::_1));
	printf_info_cli();
	return true;
}

void Client::deal_cli(int fd)
{
	CMD_DATA cmd;
	int n = recv(fd, &cmd, sizeof(cmd), 0);
	if( n <= 0 )
	{
		deal_disconnect(fd);
	}

	switch (cmd._type)
	{
		case FILE_LIST:
			send_filelist(fd);
			break;
		case DOWN:
			send_file(fd, cmd._len);
			break;
		case PLAY:
			break;
		default:	
			break;
	}
}

void Client::send_filelist(int fd)
{

	char nowpath[128] = {0};
	getcwd(nowpath, 127);
	DIR *dirp = opendir(nowpath);
	assert( dirp != NULL);

	struct dirent *dt = NULL;
	int flag = 0;

	CMD_DATA cmd;
	cmd._type = _FILE_LIST;

	while( NULL != (dt = readdir(dirp)))
	{
		if( strncmp(dt->d_name, ".", 1) == 0)
		{
			continue;
		}

		cmd._len = strlen(dt->d_name);
		send(fd, &cmd, sizeof(cmd), 0);
		send(fd, dt->d_name, strlen(dt->d_name), 0);
	}
	closedir(dirp);
	cmd._len = 0;
	send(fd, &cmd, sizeof(cmd), 0);
}

void Client::get_filelist()
{
	char buff[256] = {0};
	CMD_DATA cmd;
	cmd._type = FILE_LIST;
	cmd._len = 0;
	int fd = _event.get_cli_sockfd();
	assert(fd != -1 );
	int n = send(fd, &cmd, sizeof(cmd), 0);
	n = recv(fd, &cmd, sizeof(cmd), 0);
	assert( n > 0);
	while( cmd._len != 0 && cmd._type == _FILE_LIST )
	{
		memset(buff, 0, 256);
		n = recv(fd, buff, cmd._len, 0);
		buff[cmd._len] = 0;
		assert( n != -1);
		std::cout << buff << std::endl;
		fflush(stdout);
    	n = recv(fd, &cmd, sizeof(cmd), 0);	
		assert( n != -1);
	}
}

void Client::send_file(int fd, int len)
{

	CMD_DATA cmd;
	char buff[256] = {0};
	int n = recv(fd, buff, len, 0);
	assert( n != -1);
	int filefd = open(buff, O_RDONLY);
	if( filefd <= 0 )
	{
		cmd._type = ERR;
		cmd._len = 0;
		send(fd, &cmd, sizeof(cmd), 0);
		return;
	}
	else
	{
		cmd._type = _DOWN;
		struct stat st;
		fstat(filefd, &st);
		cmd._len = st.st_size;
	    n =	send(fd, &cmd, sizeof(cmd), 0);
        if( n <= 0)
		{
			close(filefd);
			return ;
		}
		n = recv(fd, &cmd, sizeof(cmd), 0);
		if( n <= 0 || cmd._type == ERR )
		{
			close(filefd);
			return ;
		}

		while(true)
		{
			n = read(filefd, buff, 255);
			if( n == 0 ) break;
			send(fd, buff, n, 0);
		}
	}
	close(filefd);
}

void Client::get_file()
{
	int fd =  _event.get_cli_sockfd();
	char aimfilename[256] = { 0 };
	fgets(aimfilename, 255, stdin);
	aimfilename[ strlen(aimfilename) - 1 ] = 0;

	CMD_DATA cmd;
	cmd._type = DOWN;
	cmd._len = strlen(aimfilename);
    int n =  send(fd, &cmd, sizeof(cmd), 0);
	assert(n != -1);
    n =	send(fd, aimfilename, sizeof(aimfilename), 0);
	assert( n != -1);
	n = recv(fd, &cmd, sizeof(cmd), 0);
	assert( n != -1);
	if( cmd._type == _DOWN && cmd._len != 0)
	{
		int len = cmd._len;
		int file = open(aimfilename, O_WRONLY | O_CREAT | O_TRUNC, 0664);
		if(file == -1 )
		{
			cmd._type = ERR;
			send(fd, &cmd, sizeof(cmd), 0);
			return;
		}
		else
		{
			cmd._type = OK;
			send(fd, &cmd, sizeof(cmd), 0);
			char buff[256] = {0};
			n = recv(fd, buff, 255, 0);
			int i = n;
			while( i != len)
			{
				write(file, buff, n);
				n = recv(fd, buff, 255, 0);
				if( n <= 0) break;
				i += n;
			}
			write(file, buff, n);
			close(file);
		}
	}
	else if( cmd._type == ERR)
	{
		std::cout << "NOT FOUND " <<std::endl;
		return;
	}


}
void Client::set_endian()
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

