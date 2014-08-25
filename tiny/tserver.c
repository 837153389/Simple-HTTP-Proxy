/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "csapp.h"

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";
    
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

void parse_request(int fd);
void parse_request2(int fd);


int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    while (1) {
    	clientlen = sizeof(clientaddr);
    	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    	//doit(connfd);

        /* Parse and forward request the original server */
        parse_request(connfd);

        /* Send the serer reponse to the requesting client */

    	Close(connfd);
    }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
  
    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) { 
        clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
    	clienterror(fd, filename, "404", "Not found",
    		    "Tiny couldn't find this file");
    	return;
    }

    if (is_static) { /* Serve static content */
    	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
    	    clienterror(fd, filename, "403", "Forbidden",
    			"Tiny couldn't read the file");
    	    return;
    	}
    	serve_static(fd, filename, sbuf.st_size);
    }
    else { /* Serve dynamic content */
    	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
    	    clienterror(fd, filename, "403", "Forbidden",
    			"Tiny couldn't run the CGI program");
    	    return;
    	}
    	serve_dynamic(fd, filename, cgiargs);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {
    	Rio_readlineb(rp, buf, MAXLINE);
    	printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */
    	strcpy(cgiargs, "");
    	strcpy(filename, ".");
    	strcat(filename, uri);
    	if (uri[strlen(uri)-1] == '/')
    	    strcat(filename, "home.html");
    	return 1;
    }
    else {  /* Dynamic content */
    	ptr = index(uri, '?');
    	if (ptr) {
    	    strcpy(cgiargs, ptr+1);
    	    *ptr = '\0';
    	}
    	else 
    	    strcpy(cgiargs, "");
    	strcpy(filename, ".");
    	strcat(filename, uri);
    	return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	   strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	   strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
	   strcpy(filetype, "image/jpeg");
    else
	   strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* child */
    	/* Real server would set all CGI vars here */
    	setenv("QUERY_STRING", cgiargs, 1); 
    	Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
    	Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
void parse_request2(int fd) {
    rio_t rio;
    char buf[MAXLINE];


    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(&rio, buf, MAXLINE);
        printf("%s", buf);
    }
    printf("--END--");
}



void parse_request(int fd) {
    rio_t rio;
    rio_t rio2;
    int clientfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char new_request_line[MAXLINE];
    char *colon;
    char host[MAXLINE], hostname[MAXLINE];
    int port;
    char *ptr;
    char newuri[MAXLINE];
    const char *content_len_str = "Content-Length: ";

    printf("--Forward Request--\n");

    Rio_readinitb(&rio, fd);

    /* Read the HTTP request line */
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    /* Ignore none-GET methods */
    if (strcmp(method, "GET")) {
        char error[MAXLINE];
        sprintf(error, "Unable to handle method type: %s\r\n", method);
        printf("%s", error);
        printf("--END--\n\n");
        Rio_writen(fd, error, strlen(error));
        return;
    }

    /* Extract host from uri */
    if ((ptr = strstr(uri, "://"))) {
        strcpy(host, ptr + 3);
    } else {
        strcpy(host, uri);
    }
    if ((ptr = strstr(host, "/")))
        strcpy(newuri, ptr);    /* Extract the new uri */
        *ptr = '\0';            /* cut the trailing part of the host url */

    printf("Host extracted: %s\n", host);

    /* Reasemble new HTTP/1.0 GET request */
    strcpy(new_request_line, "GET ");
    strcat(new_request_line, newuri);
    strcat(new_request_line, " HTTP/1.0\r\n");

    printf("New Request Line: %s", new_request_line);

    /* Separate the hostname and hostport */
    strcpy(hostname, host);
    if ((colon = strstr(hostname, ":"))) {
        port = atoi(colon+1);
        *colon = '\0';
    } 
    else {
        port = 80;
    }
    printf("Hostname: %s,   Port: %d\n", hostname, port);

    /* Open a client socket with the server */
    clientfd = Open_clientfd_r(hostname, port);
    Rio_readinitb(&rio2, clientfd);

    /* Forward the new_request_line */
    Rio_writen(clientfd, new_request_line, strlen(new_request_line));

    /* Read the host_line */
    Rio_readlineb(&rio, buf, MAXLINE);

    /* send the host_line */
    Rio_writen(clientfd, buf, strlen(buf));
    printf("[OK] Hostline: %s", buf);

    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(&rio, buf, MAXLINE);

        if (strstr(buf, "User-Agent")) {
            strcpy(buf, user_agent_hdr);
        } else if (strstr(buf, "Accept:")) {
            strcpy(buf, accept_hdr);
        } else if (strstr(buf, "Accept-Encoding:")) {
            strcpy(buf, accept_encoding_hdr);
        } else if (strstr(buf, "Connection:")) {
            strcpy(buf, connection_hdr);
            Rio_writen(clientfd, buf, strlen(buf));
            printf("%s", buf);

            strcpy(buf, proxy_conn_hdr);
            Rio_writen(clientfd, buf, strlen(buf));
            printf("%s", buf);
            continue;
        } else if (strstr(buf, "Proxy-Connection:")) {
            continue;
        }

        Rio_writen(clientfd, buf, strlen(buf));
        printf("%s", buf);
    }

    printf("-----Ready to read response ----\n");

    /* Read and forward the first response line */
    Rio_readlineb(&rio2, buf, MAXLINE);
    Rio_writen(fd, buf, strlen(buf));

    char* content_size_ptr = NULL;
    unsigned int content_size = 0;

    /* Read and forward subsequent response header lines */
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(&rio2, buf, MAXLINE);
        printf("%s", buf);
        if ((content_size_ptr = strcasestr(buf, content_len_str))) {
            content_size_ptr += strlen(content_len_str);
            content_size = atoi(content_size_ptr);
            ///printf("\tcontent size: %d\n", content_size);
        }
        Rio_writen(fd, buf, strlen(buf));
    }

    char usrbuf[102400];
    unsigned num = 0;
    /* Read and forward the content bytes */

    num = Rio_readnb(&rio2, usrbuf, content_size);
    //write(STDOUT_FILENO, usrbuf, content_size);
    //printf("%s", usrbuf);
    Rio_writen(fd, usrbuf, content_size);
/*
    while ((k = Rio_readn(clientfd, usrbuf, 1024))) {
        Rio_writen(fd, usrbuf, 1024);
        num += k;
        printf("%s", usrbuf);
    }
*/
    printf("\n#### %d bytes have been read.\n", num);
    Close(clientfd);
    printf("\n---END---\n\n");
}
