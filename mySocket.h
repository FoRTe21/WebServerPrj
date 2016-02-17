int makeSock(int portNum);											//소켓 생성
void sendResponse(int cliSock, char *header, char *response, int len);	//응답 전송.
