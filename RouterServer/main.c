#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define BACKLOG 1
#define FILENAME "/var/lib/dhcp/dhcpd.leases"
/*#define FILENAME "dhcpd.leases"*/
#define BUFFER_SIZE 500

static char line_buffer[BUFFER_SIZE+1];
static char address_buffer[20];

static inline void writen(int fd, void *buf, int length) {
	int size = 0, ret;
	if (length<0) return;
	while (size!=length) {
		ret = write(fd, buf, length-size);
		if (ret<0) return;
		else size += ret;
	}
}
static inline int starts_with(const char *pre, const char *str) {
	size_t lenpre, lenstr;
	lenpre = strlen(pre),
	lenstr = strlen(str);
	return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}
static inline int read_line(int fd) {
	int length=0, ret;
	char c='\0';
	while(c!='\n') {
		ret = read(fd, &c, 1);
		if (ret <= 0) break;
		assert(length<BUFFER_SIZE);
		line_buffer[length]=c;
		length++;
	}
	line_buffer[length]='\0';
	length++;
	return length;
}
static void send_ip(int sd) {
	int offset = 6, length = offset;
	while(line_buffer[length] != ' ') {
		assert(line_buffer[length]!='\0');
		address_buffer[length-offset] = line_buffer[length];
		length++;
	}
	address_buffer[length-offset] = 'x';
	length++;
	writen(sd, address_buffer, length-offset);
}
static void send_mac(int sd) {
	int offset = 20, length = offset;
	while(line_buffer[length] != ';') {
		assert(line_buffer[length]!='\0');
		address_buffer[length-offset] = line_buffer[length];
		length++;
	}
	address_buffer[length-offset] = 'y';
	length++;
	writen(sd, address_buffer, length-offset);
}
static void fn(int sd) {
	char c;
	int fd;

	fd = open(FILENAME, O_RDONLY);
	if (fd<0) goto err;

	while (read_line(fd) > 1) {
		if (starts_with("lease", line_buffer)) send_ip(sd);
		else if (starts_with("  hardware ethernet", line_buffer)) send_mac(sd);
	}

	close(fd);
	c = 'z';
	writen(sd, &c, 1);
	return;
err:
	c = 'h';
	writen(sd, &c, 1);
	printf("File not found!\n");
}

int main(int argc, char **argv) {
	int port, welcome_socket, data_socket, ret, opt = -1;
	struct sockaddr_in server_address;
	struct hostent *host;

	if (argc!=3) {
		printf("Usage: <ip> <port>\n");
		return EXIT_FAILURE;
	}
	printf("MOE (router side): %s <%s>\n", argv[1], argv[2]);

	if ((host = gethostbyname (argv[1])) == NULL) {
		printf("Invalid ip address.\n");	
		return EXIT_FAILURE;
	}

	port = atoi (argv[2]);
	
	welcome_socket = socket (AF_INET, SOCK_STREAM, 0);
	assert (welcome_socket>=0);
	memset (&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
	/*server_address.sin_addr.s_addr = htonl(INADDR_ANY);*/
	server_address.sin_port = htons (port);

	setsockopt (welcome_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	ret = bind (welcome_socket, (struct sockaddr*) &server_address, sizeof(struct sockaddr_in));
	assert (ret>=0);
	ret = listen (welcome_socket, BACKLOG);
	assert (ret>=0);
	
	while(1) {
		struct sockaddr_in client;
		int client_size = sizeof(struct sockaddr_in);
		data_socket = accept (welcome_socket, (struct sockaddr*) &client, (socklen_t *)&client_size);
		if (data_socket<0) continue;
		fn(data_socket);
		close(data_socket);
	}
	close(welcome_socket);
	return EXIT_SUCCESS;
}
