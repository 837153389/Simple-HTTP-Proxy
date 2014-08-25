/* structure of sbuf */
struct stat {
    dev_t     st_dev;     /* ID of device containing file */
    ino_t     st_ino;     /* inode number */
    mode_t    st_mode;    /* protection */
    nlink_t   st_nlink;   /* number of hard links */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    dev_t     st_rdev;    /* device ID (if special file) */
    off_t     st_size;    /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
    time_t    st_atime;   /* time of last access */
    time_t    st_mtime;   /* time of last modification */
    time_t    st_ctime;   /* time of last status change */
};

#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

/* Internet address structure */
struct in_addr {
	unsigned int s_addr; /* network byte order (big-endian) */
};

/* DNS host entry structure */
struct hostent {
	char   *h_name;
	char   **h_aliases;
	int    h_addrtype;
	int    h_length;
	char   **h_addr_list; /* null-terminated array of in_addr structs */
};

/* Generic socket address */
struct sockaddr {
	unsigned short  sa_family;    /* protocol family */
	char            sa_data[14];  /* address data.  */
};

typedef struct sockaddr SA;

/* Internet>specific socket address */
￼struct sockaddr_in  {
	unsigned short  sin_family;  /* address family (always AF_INET) */
	unsigned short  sin_port;    /* port num in network byte order */
	struct in_addr  sin_addr;    /* IP addr in network byte order */
	unsigned char   sin_zero[8]; /* pad to sizeof(struct sockaddr) */
};

#include ”csapp.h“
typedef struct {
    int *buf;		/* Buffer array */
    int n;			/* Maximum number of slots */
    int front;		/* buf[(front+1)%n] is first item */
    int rear;		/* buf[rear%n] is last item */
    sem_t mutex;	/* Protects accesses to buf */
    sem_t slots;	/* Counts available slots */
    sem_t items;	/* Counts available items */
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);