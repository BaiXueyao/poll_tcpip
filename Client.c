/*This file is for client*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<sys/time.h>
#define SA struct sockaddr
#define MAXLINE 4096
#define SERV_PORT 9877

int max(int a, int b)
{
	if(a >= b)
		return a;
	return b;
}
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft > 0)
	{
		if((nwritten = write(fd, ptr, nleft)) <= 0){

			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
	  }
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}


ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;
	ptr = vptr;

	for(n = 1; n < maxlen; n++)
	{
	again:
		if((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if(c == '\n')
				break;
		}else if(rc == 0)
		{
			*ptr = 0;
			return (n -1);
		}else
		{
			if(errno == EINTR)
				goto again;
			return -1;
		}
	}
	*ptr =0;
	return n;
}

void str_cli(FILE *fp, int sockfd)
{
	char buf[MAXLINE];
	fd_set rset;
	int maxfdp1,stdineof;
	int n;
	stdineof = 0;
	FD_ZERO(&rset);
	for( ; ; ){
		if(stdineof == 0)
		{
			FD_SET(fileno(fp),&rset);
		}
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(sockfd, &rset))
		{
			if((n = read(sockfd, buf, MAXLINE)) == 0)
			{
				if(stdineof == 1)
					return ;
				else
				{
					printf("str_cli: server terminated prematurely");
					exit(-1);
				}
			}
			write(fileno(stdout),buf,n);
		}
		if(FD_ISSET(fileno(fp), &rset))
		{
			if((n = read(fileno(fp),buf,MAXLINE)) == 0)
			{
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp),&rset);
				continue;
			}
			writen(sockfd,buf,n);
		}
	}
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

	if(argc != 2)
	{
		printf("less arguments !!!!");
		exit(-1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);

	exit(0);
}

