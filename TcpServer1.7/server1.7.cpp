#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa�Ѿ��������ˣ��������������ʹ��
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

//��Ϣͷ
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
	//���û�������������
	char szRecv[1024] = {};
	//���ܿͻ��˵���������
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

		//login������ǰ���Ѿ�����ȡ���ˣ��ʶ���Ҫƫ��
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("access CMD_LOGIN length: %d , UserName: %s ,Passwd : %s\n", login->dataLength, login->userName, login->PassWord);
		//�ж��û��������Ƿ���ȷ�Ĺ��̺���
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* loginout = (Logout*)szRecv;
		printf("access CMD_LOGOUT length: %d , UserName: %s \n", loginout->dataLength, loginout->userName);
		//�ж��û��������Ƿ���ȷ�Ĺ��̺���
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
	//����windows sock2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//	1. ����һ��socket �׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//	2. �󶨽��ܿͻ������ӵĶ˿�bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//htons(ת�������ֽ���)
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		printf("errno bind...\n");
	}
	else
	{
		printf("success bind...\n");
	}
	//	3. ��������˿� listen
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
		//������ socket
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		//���socket
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
		//nfds ��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ������������
		//���������ļ����������ֵ+1����window�����������Ϊ0
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
			//	4. �ȴ����ܿͻ�������accept
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
	//	8. �ر�socket closesocket
	closesocket(_sock);
	//���windows socket����
	WSACleanup();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}