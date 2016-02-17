#include "MainHeader.h"
#include "mySocket.h"
#include "myException.h"

void printRHeader(RequestHeader *rh);								// 헤더 정보 출력.
void clientProcess(int cliSock);									//클라이언트 처리 프로세스 
int whatMethod(char *method);										// method 분별.

int readSendingInformation(char **fileBuffer, FILE *fp);			// html 파일 읽어냄.

void fillBodyBuffer(int cliSock, RequestHeader *rh);			// post인 경우 추가 body 수신처리.
void getNpostProcessing(int cliSock, RequestHeader *rh);		// get, post의 동시 기능처리.
void getProcessing(int cliSock, RequestHeader *rh);				// method get에 대한 처리. 
void postProcessing(int cliSock, RequestHeader *rh);			// method post에 대한 처리.
int checkExtention(char *filename);									// 확장자 확인.

void fileProcess(int cliSock, char *filename);						// cgi 제외한 파일에 대한 프로세스.
void cgiProcess(int cliSock, char *filename, RequestHeader rh);	// cgi 프로그램에 대한 프로세스.

int parsingInfo(char *request, RequestHeader *reqHeader); // 요청 헤더 파싱.
void fillEnvInfo(RequestHeader *reqHeader);	// 환경 변수 설정..
int cgiExec(int cliSock, char *cginame, RequestHeader rh);			// cgi 프로그램 실행.	

				

int main(int argc, char* argv[])
{
	int servSock;
	int cliSock;
	int length;
	struct sockaddr_in cliAddr;
	char tmpBuffer[BUFSIZ] = {0,};

	if(argc != 2)
	{	
		printError("error - argc != 2\n");
	}

	servSock = makeSock(atoi(argv[1]));
	printf("HTTP serverlistening on port %d\n", atoi(argv[1]));
	signal(SIGCHLD, (void *)childHandler);
	while(1) {
		printf("Waiting for client's request...\n");

		if((cliSock = accept(servSock, (struct sockaddr *)&cliAddr, &length)) < 0)
		{
			printError("AcceptError\n");
		}	
		printf("Accepted\n");											// Accept 이후 클라이언트 처리관련 child phrocess 생성.
		if(fork() == 0)
		{
			clientProcess(cliSock);										// 이곳에서 클라이언트 요청 처리.
			close(cliSock);
			printf("client closed\n");
			return 0;
		}
		else
		{
			close(cliSock);
		}
	}
	return 0;
}



void clientProcess(int cliSock)
{
	char buffer[BUFSIZ] = {0,};
	char headerBuffer[HEADERBUFSIZE] = {0,};
	
	RequestHeader rh;													// 헤더 데이터.

	recv(cliSock, headerBuffer, HEADERBUFSIZE, 0);						// 일단 header(1024)를 recv함.
	printf("header : %s\n", headerBuffer);
	if(parsingInfo(headerBuffer, &rh) == FALSE)							// header 데이터를 파싱.
	{
		sendResponse(cliSock, "HTTP/1.1 400 Bad Request\r\n\r\n", NULL, 0);
		return;
	}
	
	//printRHeader(&rh);
	fillEnvInfo(&rh);													// 환경변수에 데이터 씀.

	switch(whatMethod(rh.m_method))		// method check
	{
	case 0:								// GET
	{
		getProcessing(cliSock, &rh);
		break;
	}
	case 1:								// POST
	{
		postProcessing(cliSock, &rh);
		break;
	}
	default:
	{
		sendResponse(cliSock, "HTTP/1.1 400 Bad Request\r\n\r\n", NULL, 0);
		break;
	}
	}
}

void fillBodyBuffer(int cliSock, RequestHeader *rh)
{
	int last = 0;
	int count = 0;
	char buffer[BUFSIZE] = {0,};

	last = atoi(rh->m_contentLength) - rh->m_bodyLengthInHeader;		// 초기 헤더에 같이 들어온 body를 제외한 만큼 더 recv 함.
	while(last > 0)
	{
		memset(buffer, 0, BUFSIZE);
		count = recv(cliSock, buffer, BUFSIZE, 0);
		strcat(rh->m_bodyContent, buffer);
		last -= count; 	
	}
	//printf("%s\n", rh->m_bodyContent);
}

void getNpostProcessing(int cliSock, RequestHeader *rh)		// method "GET"에 대한 처리
{
	switch(checkExtention(rh->m_filename))					// 확장자에 따라서 처리를 다르게.
	{
	case 0:
	{
		cgiProcess(cliSock, rh->m_filename, *rh);			
		break;	
	}
	case 1:
	{
		fileProcess(cliSock, rh->m_filename);	
		break;
	}
	default:
	{
		printf("Not exist : extension\n");
		sendResponse(cliSock, "HTTP/1.1 404 Not Found\r\n\r\n", NULL, 0);
		break;
	}
	}
}

void getProcessing(int cliSock, RequestHeader *rh)
{
	getNpostProcessing(cliSock, rh);	
}
	
void postProcessing(int cliSock, RequestHeader *rh)
{
	fillBodyBuffer(cliSock, rh);
	getNpostProcessing(cliSock, rh);
}

int checkExtention(char *filename)
{
	char tmpBuf[BUFSIZ] = {0,};
	char *tok = NULL;

	strcpy(tmpBuf, filename);
	tok = strtok(tmpBuf, ".");
	if(tok == NULL)
	{	
		return -1;
	}
	tok = strtok(NULL, " ");
	if(tok == NULL)
	{	
		return -1;	
	}

	if(strcmp(tok, "cgi") == 0)							// cgi 프로그램.
	{	
		return 0;
	}
	else if((strcmp(tok, "html") == 0) ||				// MIME에 해당하는 확장자들.
			(strcmp(tok, "htm") == 0) ||				// 추후에 확장자 추가 가능.
			(strcmp(tok, "jpeg") == 0) ||
			(strcmp(tok, "jpg") == 0) ||
			(strcmp(tok, "png") == 0))
	{
		return 1;
	}
	else 
	{	
		return -1;
	}
}

void fileProcess(int cliSock, char *filename)			// 기본적인 MIME 파일에 대한 처리.
{														// 단순히 파일을 읽어서 브라우저로 send한다.
	FILE *fp = NULL;
	char header[BUFSIZ] = {0,};
	char *buffer;
	int size;
	
	if((fp = fopen(filename, "r")) == NULL)
	{
		sendResponse(cliSock, "HTTP/1.1 404 Not Found\r\n\r\n", NULL, 0);
		return ;
	}
	
	size = readSendingInformation(&buffer, fp);		
	fclose(fp);

	sendResponse(cliSock, "HTTP/1.1 200 OK\r\n\r\n", buffer, size);

	free(buffer);
}

void cgiProcess(int cliSock, char *filename, RequestHeader rh)
{
	cgiExec(cliSock, filename, rh); 
}

void printRHeader(RequestHeader *rh)
{
	printf("==========HEADER=============\n");
	printf("method : %s\n", rh->m_method);
	printf("filename : %s\n", rh->m_filename);
	printf("httpVersion : %s\n", rh->m_httpVersion);
	printf("contentType : %s\n", rh->m_contentType);
	printf("boundary : %s\n", rh->m_boundary);
	printf("contentLength : %s\n", rh->m_contentLength);
	printf("bodyContent : %s\n", rh->m_bodyContent);
	printf("bodyLengthInHeader : %d\n", rh->m_bodyLengthInHeader);	
	printf("==========HEADER END=========\n");
}

int parsingInfo(char *request, RequestHeader *reqHeader) 
{
	char *tok = NULL;
	char *pBody = NULL;
	pBody = strstr(request, "\r\n\r\n");				// 먼저 strstr()로 body의 위치 확보.
	//printf("pBody : %s\n", pBody);

	tok = strtok(request, " ");							// method parsing.
	if(tok == NULL)
	{
		printf("method Error\n");
		return FALSE;
	}	

	if((strcmp(tok, "GET") != 0) && 
		(strcmp(tok, "POST") != 0))
	{
		printf("wrong method\n");
		return FALSE;
	}
	strcpy(reqHeader->m_method, tok); 

	tok = strtok(NULL, " ");
	if(tok == NULL)
	{
		printf("filename Error\n");
		return FALSE;
	}
	strcpy(reqHeader->m_filename, tok + 1);				// filename parsing.

	while((tok = strtok(NULL, " \r\n")) != NULL)		
	{
		//printf("%s\n", tok);	
		
		if(strcmp(tok, "Content-Type:") == 0)
		{
			tok = strtok(NULL, "\r\n");
			strcpy(reqHeader->m_contentType, tok);
		}
		if(strcmp(tok, "Content-Length:") == 0)			// content-length가 0이 아니면 body가 있다는 의미이므로
		{												// header에 포함된 body를 body buffer에 저장한다.
			tok = strtok(NULL, "\r\n");
			strcpy(reqHeader->m_contentLength, tok);
			reqHeader->m_bodyContent = (char *)malloc(sizeof(char) * atoi(reqHeader->m_contentLength));

			strcpy(reqHeader->m_bodyContent, (pBody + strlen("\r\n\r\n")));
			//printf("bodyContent : %s\n", reqHeader->m_bodyContent);
			reqHeader->m_bodyLengthInHeader = strlen(reqHeader->m_bodyContent);
		}
	}

	return TRUE;
}

void fillEnvInfo(RequestHeader *reqHeader)				// 필요한 정보를 환경변수에 저장.
{
	setenv("REQUEST_METHOD", reqHeader->m_method, 1);
	setenv("CONTENT_LENGTH", reqHeader->m_contentLength, 1);
}

int cgiExec(int cliSock, char *cginame, RequestHeader rh)
{
	FILE *fp = NULL;
	pid_t childPid;
	int pipe1[2];
	int pipe2[2];
	if((fp = fopen(cginame, "r")) == NULL)						// cgi파일이 존재하는지 확인함.
	{
		printf("cgi file - not exist\n");
		sendResponse(cliSock, "HTTP/1.1 404 Not Found\r\n\r\n", NULL, 0);
		return FALSE;
	}

	fclose(fp);

	if(strcmp(rh.m_method, "GET") == 0)							// get으로 접근했을 때 예외처리.
	{															// ps. 왜 해줬지....?
		sendResponse(cliSock, "HTTP/1.1 400 Bad Request\r\n\r\n", NULL, 0);
		return FALSE;
	}
	if(pipe(pipe1) == -1)										// parent, child간 통신을 위한 밑작업.
	{
		close(cliSock);
		printError("Error - pipe\n");
	}
	if(pipe(pipe2) == -1)
	{
		close(cliSock);
		printError("Error - pipe\n");
	}
	
	if((childPid = fork()) == -1)
	{
		close(cliSock);
		printError("error - fork\n");
	}
	else if(childPid == 0)										 
	{
		close(cliSock);
		close(pipe1[1]);			// write pipe closed
		if(dup2(pipe1[0], STDIN) == -1)		// read pipe -> stdin 
		{
			close(cliSock);
			printError("error - dup2\n");
		}

		close(pipe2[0]);			// read pipe closed
		if(dup2(pipe2[1], STDOUT) == -1)			// write pipe -> stdout
		{
			close(cliSock);
			printError("error - dup2\n");	
		}

		execl(cginame, NULL);
	}
	else
	{
		int flag = FALSE;
		char buffer[BUFSIZ] = {0,};
		int state;
		int n;
		close(pipe1[0]);			// read pipe closed
		if(dup2(pipe1[1], STDOUT) == -1)			// write pipe -> stdout
		{
			close(cliSock);
			printError("error - dup2\n");	
		}

		close(pipe2[1]);			// write pipe closed
		if(dup2(pipe2[0], STDIN) == -1)		// read pipe -> stdin 
		{
			close(cliSock);
			printError("error - dup2\n");
		}
		
		sendResponse(cliSock, "HTTP/1.1 200 OK\r\n", NULL, 0);
		printf("%s\n", rh.m_bodyContent);						// 표준입출력으로 cgi 프로그램에 body 전달.

		while((n = read(STDIN_FILENO, buffer, BUFSIZE)) > 0)	// cgi에서의 출력데이터를 클라이언트로 전송.
		{
			//printf("read : %s\n", buffer);
			sendResponse(cliSock, NULL, buffer, n);
		}

		wait(&state);
	}
}

int whatMethod(char *method)
{
	if(strcmp(method, "GET") == 0)
	{	
		return 0;
	}
	else if(strcmp(method, "POST") == 0)
	{
		return 1;
	}
	else
	{	
		return -1;
	}
}



int readSendingInformation(char **fileBuffer, FILE *fp)			// 파일을 읽어 들임.
{
	int size;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);	

	*fileBuffer = (char *)malloc(sizeof(char)*(size));

	fread(*fileBuffer, sizeof(char) * (size) , 1, fp);
	//(*fileBuffer)[size] = EOF;

	return size;
}




