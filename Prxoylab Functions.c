/* Socket API */
int socket(int domain, int type, int protocol);
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
int listen(int socket, int backlog);
int accept(int socket, struct sockaddr *address, socklen_t *address_len);
int connect(int socket, struct sockaddr *address, socklen_t address_len);
int close(int fd);
ssize_t read(int fd, void *buf, size_t nbyte);
ssize_t write(int fd, void *buf, size_t nbyte);


/* pthread_rwlock_* handles that for you */
pthread_rwlock_t lock;
pthread_rwlock_init(&lock,NULL);
pthread_rwlock_rdlock(&lock);
pthread_rwlock_wrlock(&lock);
pthread_rwlock_unlock(&lock);

pthread_mutex_t mutex;
pthread_mutex_init(&mutex, 0);
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);


pthread_rwlock_rdlock(&cache_rwlock);
pthread_rwlock_wrlock(&cache_rwlock);
pthread_rwlock_unlock(&cache_rwlock);

/* For proxy lab */
pthread_mutex_t reading, writing;
unsigned int reader_cnt = 0;

pthread_mutex_lock(&reading);
reader_cnt++;
if (reader_cnt == 1)
	pthread_mutex_lock(&writing);
pthread_mutex_unlock(&reading);
	/* reading */
pthread_mutex_lock(&reading);
reader_cnt--;
if (reader_cnt == 0)
	pthread_mutex_unlock(&writing)pthread_mutex_unlock(&reading);

pthread_mutex_lock(&writing);
	/* writing */
pthread_mutex_unlock(&writing);




/* Forwarding server response */

        /* Read and forward the first response line */
    /*    
        Rio_readlineb(&rio_server, buf, MAXLINE);
        Rio_writen(clientfd, buf, strlen(buf));

        char *content_size_ptr = NULL;
        unsigned int content_size = 0;
    */
        /* Read and forward subsequent response header lines */
    /*    while (strcmp(buf, "\r\n")) {
            Rio_readlineb(&rio_server, buf, MAXLINE);
            printf("%s", buf);
            if ((content_size_ptr = (char *) strcasestr(buf, 
                    content_len_str))) 
            {
                content_size_ptr += strlen(content_len_str);
                content_size = atoi(content_size_ptr);
            }
            Rio_writen(clientfd, buf, strlen(buf));
        }
    */

        /* Read and forward the content bytes */
    /*
        byte_count = Rio_readnb(&rio_server, usrbuf, content_size);
        //write(STDOUT_FILENO, usrbuf, content_size);

        Rio_writen(clientfd, usrbuf, content_size);
    */

