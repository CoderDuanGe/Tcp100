#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	short dataLength;
	short cmd;
};

struct Login :public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};

struct Logout :public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;

};

int processor(SOCKET _cSock)
{
	//利用缓存来接受数据
	char szRecv[1024] = {};
	//接受客户端的请求数据
	int nLen = recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen < 0)
	{
		printf("server dis connect <socket = %d> quit\n", _cSock);
		return -1;
	}
	//if(nLen>=header->dataLength)

	switch (header->cmd)
	{
			case CMD_LOGIN_RESULT:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				LoginResult* login = (LoginResult*)szRecv;
				printf("access server message <Socket =%d> LoginResult length: %d n", _cSock, login->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				LogoutResult* loginout = (LogoutResult*)szRecv;
				printf("access<Socket =%d> CMD_LOGOUT_RESULT length: %d\n", _cSock, loginout->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				NewUserJoin* userjoin = (NewUserJoin*)szRecv;
				printf("access<Socket =%d> CMD_NEW_USER_JOIN length: %d\n", _cSock, userjoin->dataLength);
			}
			break;
	}
}

int main()
{
	//启动windows sock2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//	1. 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("sock bulid errno\n");
	}
	else
	{
		printf("sock bulid success\n");
	}
	//	2. 连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(45678);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("connect errno\n");
	}
	else
	{
		printf("connect success\n");
	}


	while (true)
	{

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		timeval t = { 1,0 };

		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select mission fish 1!\n");
			break;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				printf("select mission fish 2!\n");
			}
		}
		
		printf("free time deal with somthing !\n");
		Login login;
		strcpy(login.userName, "demo");
		strcpy(login.PassWord, "demo0");
		send(_sock, (const char*)&login, sizeof(Login), 0);
		Sleep(1000);

	}
	//	4. 关闭socket closesocket
	closesocket(_sock);
	//清楚windows socket环境
	WSACleanup();
	getchar();
	printf("exit,fish\n");
	getchar();
	return 0;

}