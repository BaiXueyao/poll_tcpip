# poll_tcpip
Using poll method to achieve a simple tcpip code.

poll use struct pollfd to maintain the fd.

struct pollfd{
	int fd;
	short events;
	short revents;
};

Then poll is same like select.
