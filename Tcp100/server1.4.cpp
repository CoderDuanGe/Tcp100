#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa已经被废弃了，用这个宏来继续使用
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>


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
	//	1. 建立一个socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//	2. 绑定接受客户端连接的端口bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//htons(转换网络字节序)
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		printf("errno bind...\n");
	}
	else
	{
		printf("success bind...\n");
	}
	//	3. 监听网络端口 listen
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("errno listen...\n");
	}
	else
	{
		printf("success listen...\n");
	}

	//	4. 等待接受客户端连接accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;


	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("errno accept...\n");
	}
	printf("new server join: socket=%d IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
	while (true)
	{
		DataHeader header = {};
		//5.接受客户端的请求数据
		int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("client quit\n");
			break;
		}
		printf("access command :%s length: %d \n", header.cmd, header.dataLength);
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(_cSock, (char*)&login, sizeof(Login), 0);
			//判断用户名密码是否正确的过程忽略
			LoginResult ret = { 0 };

			send(_cSock, (char*)&header, sizeof(DataHeader), 0);
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout loginout = {};
			recv(_cSock, (char*)&loginout, sizeof(Logout), 0);
			//判断用户名密码是否正确的过程忽略
			LogoutResult ret = { 0 };

			send(_cSock, (char*)&header, sizeof(DataHeader), 0);
			send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		break;
		default:
		{
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (char*)&header, sizeof(LogoutResult), 0);
		}
		break;
		}


	}
	//	8. 关闭socket closesocket
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}