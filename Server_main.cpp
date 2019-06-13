#include "Server.h"

int main(int argc, char* argv[])
{

	if (argc > 1)
	{
		Server ser(argv[1], argv[2]);
		ser.run();
	}
	else
	{
		Server ser(NULL, NULL);
		ser.run();
	}
	return 0;
}
