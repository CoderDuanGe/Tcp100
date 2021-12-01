#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa已经被废弃了，用这个宏来继续使用
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include <vector>

#pragma comment(lib,"ws2_32.lib")



enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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

std::vector<SOCKET>g_clients;

int processor(SOCKET _cSock)
{
	//利用缓存来接受数据
	char szRecv[1024] = {};
	//接受客户端的请求数据
	int nLen = recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen < 0)
	{
		printf("client quit\n");
		return -1;
	}
	//if(nLen>=header->dataLength)

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//login数据在前面已经被读取过了，故而需要偏移
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("access CMD_LOGIN length: %d , UserName: %s ,Passwd : %s\n", login->dataLength, login->userName, login->PassWord);
		//判断用户名密码是否正确的过程忽略
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* loginout = (Logout*)szRecv;
		printf("access CMD_LOGOUT length: %d , UserName: %s \n", loginout->dataLength, loginout->userName);
		//判断用户名密码是否正确的过程忽略
		LogoutResult ret;
		send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
	{
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(LogoutResult), 0);
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


	while (true)
	{
		//伯克利 socket
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		//清空socket
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size()-1; n >=0; --n)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		//nfds 是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
		//既是所有文件描述符最大值+1，在window中这参数可以为0
		int ret=select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
		std::cout << "ret:" << ret << std::endl;
		if (ret < 0) 
		{
			printf("select quit\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			//	4. 等待接受客户端连接accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{
				printf("errno invalid socket....\n");
				continue;
			}
			g_clients.push_back(_cSock);
			printf("new server join: socket=%d IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		for (int n =0 ; n < fdRead.fd_count; ++n)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter=find(g_clients.begin(),g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		
	}


	for (size_t n = g_clients.size() - 1; n >= 0; --n)
	{
		closesocket(g_clients[n]);
	}
	//	8. 关闭socket closesocket
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}