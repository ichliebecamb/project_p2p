#include "Client.h"

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		Client ser(argv[1], argv[2]);
		ser.run();
	}
	else
	{
		std::cout << "./client ip port" <<std::endl;
	}
	return 0;
}
