#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>

#define BACKLOG 	100
#define RECEIVER_PORT 	5193
#define DB_FILE_NAME 	"/home/roma2lug/moe-receiver/users.sqlite"

void thread_receiver(void *sock);
void initialize_db();
void exec_query(sqlite3 * db, char *query);
void exec_statement(sqlite3 * db, sqlite3_stmt * statement);
void create_pthread(pthread_t * tid, void *startroutine, void *arg, int joinable_detached);
ssize_t readn(int fd, void *buf, size_t buf_dim);
