#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#define TRUE 1
#define FALSE 0

#define STDIN 0
#define STDOUT 1
#define BUFSIZE BUFSIZ
#define HEADERBUFSIZE 1024

typedef struct RequestHeader
{
	char m_method[HEADERBUFSIZE];				// GET, POST, ...
	char m_filename[HEADERBUFSIZE];			 
	char m_httpVersion[HEADERBUFSIZE];			// HTTP version.
	char m_contentType[HEADERBUFSIZE];			// MIME.
	char m_boundary[HEADERBUFSIZE]; 			// body의 구분자.
	char m_contentLength[HEADERBUFSIZE];		// body의 길이.
	char *m_queryString;				// header 의 추가적인 데이터.
	char *m_bodyContent;				// header에 추가로 붙는 body.
	int m_bodyLengthInHeader;			// header에 포함된 body data의 길이.
} RequestHeader;