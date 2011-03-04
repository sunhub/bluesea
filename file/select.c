#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	int fd;
	fd_set fds;
	fd_set fdr;
	struct timeval timeout;
	char buffer;

	if(argc < 2)
	{
		printf("Usage: command filename\n");
		return 0;
	}

	fd = open(argv[1], O_RDONLY);
	
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	
	while(1)
	{
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	int ret = select(fd+1, &fdr, NULL, NULL, &timeout);	
	if(ret == 0)
	{
		printf("Select time out!\n");
		continue;
	}
	else if(ret < 0)
	{
		printf("Select error with %i.\n",ret);
		continue;
	}
	if(FD_ISSET(fd, &fdr))
	{	
		read(fd, &buffer, sizeof(char));
		printf("<<<<<<<%i\n",buffer);
	}
	}
	return 0;
}
