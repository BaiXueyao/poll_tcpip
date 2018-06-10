/*This is for server*/

#include<stdio.h>
#include<poll.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<errno.h>
#include<arpa/inet.h>
#include <limits.h>
#include<stdlib.h>
#include<unistd.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<signal.h>
#define SERV_PORT 9877
#define SA struct sockaddr
#define MAXLINE 4096
#define LISTENQ 1024
#define INFTIM -1



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

void str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXLINE];
	again:
		while((n = read(sockfd, buf, MAXLINE)) > 0)
			writen(sockfd, buf, n);
		if(n < 0 && errno == EINTR)
			goto again;
		else if(n < 0)
			printf("str_echo: read error");
}

int
main(int argc, char **argv)
{
	int i, maxi, listenfd, connfd, sockfd;
	int nready;
	ssize_t n;
	char buf[MAXLINE];
	socklen_t clilen;
	struct pollfd client[OPEN_MAX];
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for(i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1;
	maxi = 0;
	for( ; ; )
	{
		nready = poll(client, maxi + 1, INFTIM);
		if(client[0].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
			for(i = 1; i < OPEN_MAX; i++)
			{
				if(client[i].fd < 0)
				{
					client[i].fd = connfd;
					break;
				}
			}
			if(i == OPEN_MAX)
			{
				printf("too many clients\n");
				exit(-1);
			}
			client[i].events = POLLRDNORM;
			if(i > maxi)
				maxi = i;
			if(--nready <= 0)
				continue;
		}
		for(i = 1; i < maxi; i++)
		{
			if((sockfd = client[i].fd) < 0)
				continue;
			if(client[i].revents & (POLLRDNORM | POLLERR))
			{
				if((n = read(sockfd, buf, MAXLINE)) < 0)
				{
					if(errno == ECONNRESET)
					{
						close(sockfd);
						client[i].fd = -1;
					}else
						perror("read error\n");
				}else if(n == 0)
				{
					close(sockfd);
					client[i].fd = -1;
				}else
					writen(sockfd, buf, n);
				if(--nready <= 0)
				{
					break;
				}
			}
		}
	}
}
