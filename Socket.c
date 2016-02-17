#include "mySocket.h"

int makeSock(int portNum)										// socket init.
{
	int sock;
	int one = 1;
	struct sockaddr_in addr;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printError("error - socket\n");
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(portNum);

	if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		close(sock);
		printError("error - bind\n");
	}

	if(listen(sock, 5) == -1)
	{
		close(sock);
		printError("error - listen\n");
	}

	return sock;
}

void sendResponse(int cliSock, char *header, char *response, int len) // header만 보낼 땐 3번 parameter는 NULL,
{																		// 4번 parameter는 0.
	int ss = 0;
	if(header != NULL)													// 내용만 보낼 때는 2번 parameter를 NULL.
	{
		send(cliSock, header, strlen(header), 0);
	}
	if(response != NULL && len > 0)
	{
		ss = send(cliSock, response, len, 0);
		printf("ss : %d\n", ss);
		printf("len : %d\n", len);

	}
}
