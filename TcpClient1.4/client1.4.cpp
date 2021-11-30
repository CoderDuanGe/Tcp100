#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include <stdio.h>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	short dataLength;
	short cmd;
};

struct Login
{
	char userName[32];
	char PassWord[32];
};

struct LoginResult
{
	int result;

};

struct Logout
{
	char userName[32];
};

struct LogoutResult
{
	int result;
};


#pragma comment(lib,"ws2_32.lib")
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
	_sin.sin_port = htons(4567);
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
		//3.输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		//4.处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("access exit command\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login = { "demo","demo0" };
			DataHeader dh = { sizeof(login),CMD_LOGIN };

			//5.向服务器发送请求命令
			send(_sock, (const char*)&dh, sizeof(DataHeader), 0);
			send(_sock, (const char*)&login, sizeof(Login), 0);
			//接受服务器返回的数据
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(LoginResult), 0);
			printf("loginresult: %d", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout = { "demo" };
			DataHeader dh = { sizeof(logout),CMD_LOGOUT };

			//5.向服务器发送请求命令
			send(_sock, (const char*)&dh, sizeof(DataHeader), 0);
			send(_sock, (const char*)&logout, sizeof(Logout), 0);

			//接受服务器返回的数据
			DataHeader retHeader = {};
			LogoutResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
			printf("logoutresult: %d", logoutRet.result);
		}
		else {
			printf("access errno command,please again!\n");
		}

	}

	//	4. 关闭socket closesocket
	closesocket(_sock);
	//清楚windows socket环境
	WSACleanup();
	getchar();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}