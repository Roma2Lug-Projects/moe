#include "header.h"

int main(int argc, char **argv)
{
	int sockfd;
	struct timeval timeout;
	struct sockaddr_in servaddr;
	char *mac_address;
	int delta_time;

	char *RECEIVER_ADDRESS;
	char *RECEIVER_PORT;

	RECEIVER_ADDRESS = getenv("RECEIVER_ADDRESS");
	RECEIVER_PORT = getenv("RECEIVER_PORT");

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		exit(EXIT_FAILURE);

	if (argc != 3) {
		fprintf(stderr, "Usage: %s Time MacAddress\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	delta_time = atol(argv[1]) - time(NULL);
	mac_address = argv[2];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(EXIT_FAILURE);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
		exit(EXIT_FAILURE);

	memset((void *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(RECEIVER_PORT));
	if (inet_pton(AF_INET, RECEIVER_ADDRESS, &servaddr.sin_addr) <= 0)
		exit(EXIT_FAILURE);

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		exit(EXIT_FAILURE);

	if (writen(sockfd, &delta_time, sizeof(long)) != sizeof(long))
		exit(EXIT_FAILURE);

	if (writen(sockfd, mac_address, sizeof(char) * 17) != sizeof(char) * 17)
		exit(EXIT_FAILURE);

	if (close(sockfd) < 0)
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}

ssize_t writen(int fd, void *buf, size_t buf_dim)
{
	size_t nleft;
	ssize_t nwritten;
	char *ptr;
	ptr = buf;
	nleft = buf_dim;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if ((nwritten < 0) && (errno == EINTR))
				nwritten = 0;
			else
				return (-1);
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (buf_dim - nleft);
}
