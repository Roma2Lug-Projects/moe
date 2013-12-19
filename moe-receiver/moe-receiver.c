#include "header.h"

sqlite3 *database;
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	int listensd, connsd, err, optval;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len;
	pthread_t pthread;
	int *sock;

	if (argc != 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((err = sqlite3_open_v2(DB_FILE_NAME, &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) != SQLITE_OK) {
		fprintf(stderr, "Error in sqlite3_open_v2: %d : %s\n", err, sqlite3_errmsg(database));
		exit(EXIT_FAILURE);
	}

	initialize_db();

	if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error in socket: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	optval = 1;
	if (setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
		fprintf(stderr, "Error in setsockopt: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset((void *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(RECEIVER_PORT);

	if ((bind(listensd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
		fprintf(stderr, "Error in bind: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(listensd, BACKLOG) < 0) {
		fprintf(stderr, "Error in listen: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	for (;;) {
		len = sizeof(cliaddr);
		if ((connsd = accept(listensd, (struct sockaddr *) &cliaddr, &len)) < 0) {
			fprintf(stderr, "Error in accept: %d : %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		if ((sock = malloc(sizeof(int))) == NULL) {
			fprintf(stderr, "Error in malloc\n");
			exit(EXIT_FAILURE);
		}

		*sock = connsd;
		create_pthread(&pthread, thread_receiver, (void *) sock, PTHREAD_CREATE_DETACHED);
	}
}

void thread_receiver(void *sock)
{
	int n, fd, connsd, err;
	char *mac_address;
	int64_t delta_time;
	int64_t lease_time;
	int64_t time_now;
	struct timeval timeout;
	char *query;
	sqlite3_stmt *statement;
	struct flock lock;

	connsd = *((int *) sock);
	free(sock);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (setsockopt(connsd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
		fprintf(stderr, "Error in setsockopt: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((n = readn(connsd, &delta_time, sizeof(int64_t))) != sizeof(int64_t)) {
		fprintf(stderr, "Error in readn\n");
		exit(EXIT_FAILURE);
	}

	mac_address = alloca(sizeof(char) * 17);
	if ((n = readn(connsd, mac_address, sizeof(char) * 17)) != sizeof(char) * 17) {
		fprintf(stderr, "Error in readn\n");
		exit(EXIT_FAILURE);
	}

	if ((err = pthread_mutex_lock(&thread_mutex)) != 0) {
		fprintf(stderr, "Error in pthread_mutex_lock: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((fd = open(DB_FILE_NAME, O_WRONLY)) < 0) {
		fprintf(stderr, "Error in open: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if (fcntl(fd, F_SETLKW, &lock) < 0) {
		fprintf(stderr, "%s is locked\n", DB_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	query = "UPDATE Users SET LastIn = ?1 WHERE MacAddr = ?2;";
	if (sqlite3_prepare_v2(database, query, -1, &statement, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error in sqlite3_prepare_v2: %s\n", sqlite3_errmsg(database));
		exit(EXIT_FAILURE);
	}
	
	time_now = (int64_t)time(NULL);
	lease_time = delta_time + time_now;

	if (sqlite3_bind_int64(statement, 1, (sqlite3_int64)lease_time) != SQLITE_OK) {
		fprintf(stderr, "Error in sqlite3_bind_blob: %s\n", sqlite3_errmsg(database));
		exit(EXIT_FAILURE);
	}

	if (sqlite3_bind_text(statement, 2, mac_address, sizeof(char) * 17, SQLITE_TRANSIENT) != SQLITE_OK) {
		fprintf(stderr, "Error in sqlite3_bind_blob: %s\n", sqlite3_errmsg(database));
		exit(EXIT_FAILURE);
	}

	exec_statement(database, statement);


	if (close(connsd) < 0) {
		fprintf(stderr, "Error in socket: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	lock.l_type = F_UNLCK;

	if (fcntl(fd, F_SETLK, &lock) < 0) {
		fprintf(stderr, "error to unlock %s\n", DB_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	if ((fd = close(fd)) < 0) {
		fprintf(stderr, "Error in close: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((err = pthread_mutex_unlock(&thread_mutex)) != 0) {
		fprintf(stderr, "Error in pthread_mutex_unlock: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

}

void initialize_db()
{
	char *query;

	query = "CREATE TABLE IF NOT EXISTS Users	(MacAddr TEXT PRIMARY KEY, \
							 Name TEXT NOT NULL,\
							 LastIn INTEGER NOT NULL);";

	exec_query(database, query);
}

void exec_query(sqlite3 * db, char *query)
{
	int result;
	sqlite3_stmt *statement;

	while (strlen(query) != 0) {
		if (sqlite3_prepare_v2(db, query, -1, &statement, (const char **) &query) != SQLITE_OK) {
			fprintf(stderr, "Error in sqlite3_prepare_v2: %s\n", sqlite3_errmsg(db));
			exit(EXIT_FAILURE);
		}

		while ((result = sqlite3_step(statement)) != SQLITE_DONE) {
			if (result != SQLITE_ROW) {
				fprintf(stderr, "Error in sqlite3_step_v2: %s\n", sqlite3_errmsg(db));
				exit(EXIT_FAILURE);
			}
		}

		if (sqlite3_finalize(statement) != SQLITE_OK) {
			fprintf(stderr, "Error in sqlite3_finalize: %s\n", sqlite3_errmsg(db));
			exit(EXIT_FAILURE);
		}
	}
}

void exec_statement(sqlite3 * db, sqlite3_stmt * statement)
{
	int result;

	while ((result = sqlite3_step(statement)) != SQLITE_DONE) {
		if (result != SQLITE_ROW) {
			fprintf(stderr, "Error in sqlite3_step_v2: %s\n", sqlite3_errmsg(db));
			exit(EXIT_FAILURE);
		}
	}

	if (sqlite3_finalize(statement) != SQLITE_OK) {
		fprintf(stderr, "Error in sqlite3_finalize: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
}

void create_pthread(pthread_t * tid, void *startroutine, void *arg, int joinable_detached)
{
	int err;
	pthread_attr_t attr;

	if ((err = pthread_attr_init(&attr)) != 0) {
		fprintf(stderr, "Error in pthread_attr_init: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((err = pthread_attr_setdetachstate(&attr, joinable_detached)) != 0) {
		fprintf(stderr, "Error in pthread_attr_setdetachstate: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((err = pthread_create(tid, &attr, startroutine, arg)) != 0) {
		fprintf(stderr, "Error in pthread_create: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((err = pthread_attr_destroy(&attr)) != 0) {
		fprintf(stderr, "Error in pthread_attr_destroy: %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}
}

ssize_t readn(int fd, void *buf, size_t buf_dim)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;
	ptr = buf;
	nleft = buf_dim;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) <= 0) {
			if (nread == 0)
				break;
			else if ((nread < 0) && (errno == EINTR))
				nread = 0;
			else
				return (-1);
		}
		nleft -= nread;
		ptr += nread;
	}
	return (buf_dim - nleft);
}
