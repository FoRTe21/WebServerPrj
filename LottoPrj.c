#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 30
#define LOTTONUMBERS 46
#define TRUE 1
#define FALSE 0
#define SIX 6 

#define REQUEST_METHOD "REQUEST_METHOD"
#define QUERY_STRING "QUERY_STRING"
#define CONTENT_LENGTH "CONTENT_LENGTH"
#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define MAX_PARAMETER 1024

char *getParamString();
void printError();

int main(int argc, char *argv[])
{
	int i, j, k, l, turn;
	int count = 1; 
	int cnt = 0;
	char *oneLine;
	char *twoLine;
	int seven[SIX] = {0,};
	int temp_one[SIX] = {0,};
	int temp_two[SIX] = {0,};
	char *totalData = NULL;
	char *digit = NULL;


	//////////////////////////////////////////////////////////////
	// 앞 뒤로 차이의 평균을 구해서 마지막 숫자에 가감함.
	// //////////////////////////////////////////////////////////
	
	totalData = getParamString();
	totalData = strstr(totalData, "1,");		
	if(totalData == NULL)
	{
		printError();
		return 1; 
	}
	digit = strtok(totalData, ",");
	if(digit == NULL)
	{
		printError();
		return 1;	
	}
	for(i = 0; i < SIX; i++)
	{										// 후에 다음 데이터와 뺄셈으로 차이값을 누적함.
		digit = strtok(NULL, ",");		// 각 데이터를 1~7번으로 나누어서 저장.
		if(digit == NULL)
		{
			printError();
			return 1;
		}
		temp_one[i] = atoi(digit);	
	}
	digit = strtok(NULL, "\n");	
	if(digit == NULL)
	{
		printError();
		return 1;
	}

	while (1) {
		digit = strtok(NULL, ",");
		if(digit == NULL)
		{
			printError();
			return 1;
		}

		if(atoi(digit) == 0)
		{
			break;
		}

		for(i = 0; i < SIX; i++)
		{
			digit = strtok(NULL, ","); 				// 두번째 데이터를 뽑아냄
			if(digit == NULL)
			{
				printError();
				return 1;
			}
			temp_two[i] = atoi(digit);
		}

		strtok(NULL, "\n");	
		if(digit == NULL)
		{
			printError();
			return 1;
		}

		for(i = 0; i < SIX; i++)						// 앞과 뒤의 데이터 차이값을 누적
		{
			seven[i] += temp_two[i] - temp_one[i];
		}
		memcpy(temp_one, temp_two, sizeof(int) * SIX);
	}
	
	
	printf("Content-Type: text/html\n\n\n");
	printf("<HTML>\n<HEAD></HEAD>\n<BODY>\n");
	printf("average : ");
	for (i = 0; i < SIX; i++) {
		printf("%d ", seven[i]);	
	}
	printf("</br>");
	
	printf("lotto number : ");
	for(j = 0; j < SIX; j++)
	{
		printf("%d ", temp_two[j] + seven[j]);
	}
	printf("</br>");	
	printf("</BODY>\n</HTML>\n");
	return 0;
}

char *getParamString()
{
	char *param = NULL;
	char *buf = NULL;

	int length = 0;

	param = getenv(REQUEST_METHOD);	
	if(param == NULL)
	{
		perror("REQUEST_METHOD env. variable doesn't exist");
		return NULL;
	}

	if(strcmp(param, METHOD_GET) == 0)
	{
		buf = malloc(MAX_PARAMETER);
		param = getenv(QUERY_STRING);
		
		if(param == NULL || buf == NULL) {
			perror("Memory allocation failed");
			return NULL;
		}

		strncpy(buf, param, MAX_PARAMETER-1);
	}
	else if(strcmp(param, METHOD_POST) == 0)
	{
		param = getenv(CONTENT_LENGTH);
		if(param == NULL)
		{
			perror("CONTENT_LENGTH env. variable doesn't exist");
			return NULL;
		}
		
		length = atoi(param);
		if(length <= 0)
		{
			perror("Content Length is negative");
			return NULL;
		}

		buf = (char *)malloc(sizeof(char) * (length + 1));
			
		read(STDIN_FILENO, buf, length);
		buf[length] = '\0';
	}
	else 
	{
		perror("Requested method is unrecognizable");
		return NULL;
	}
	
	return buf;
}
	
void printError()
{
	printf("Content-Type: text/html\r\n\r\n");
	printf("<HTML>\n<HEAD></HEAD>\n<BODY>\n");

	printf("incorrect file format.<BR>\n");
	printf("======= file format ======<BR>\n");
	printf("index, data, data, data, data, data, data<BR>\n");

	printf("</BODY>\n</HTML>\n");

}
