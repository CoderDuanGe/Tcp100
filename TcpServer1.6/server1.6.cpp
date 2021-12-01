#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa已经被废弃了，用这个宏来继续使用
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>


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

struct Login:public DataHeader
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
		//利用缓存来接受数据
		char szRecv[1024] = {};
		//5.接受客户端的请求数据
		int nLen = recv(_cSock, (char*)&szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0)
		{
			printf("client quit\n");
			break;
		}
		//if(nLen>=header->dataLength)
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			//login数据在前面已经被读取过了，故而需要偏移
			recv(_cSock, szRecv +sizeof(DataHeader), header->dataLength- sizeof(DataHeader), 0);
			Login *login = (Login*)szRecv;
			printf("access CMD_LOGIN length: %d , UserName: %s ,Passwd : %s\n", login->dataLength,login->userName,login->PassWord);
			//判断用户名密码是否正确的过程忽略
			LoginResult ret ;
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
	//	8. 关闭socket closesocket
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}