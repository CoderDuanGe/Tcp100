#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include <stdio.h>
#include <string.h>

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
#pragma comment(lib,"ws2_32.lib")


int processor(SOCKET _cSock)
{
	//���û�������������
	char szRecv[1024] = {};
	//���ܿͻ��˵���������
	int nLen = recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen < 0)
	{
		printf("client<socket = %d> quit\n", _cSock);
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
		printf("access<Socket =%d> CMD_LOGIN length: %d , UserName: %s ,Passwd : %s\n", _cSock, login->dataLength, login->userName, login->PassWord);
		//�ж��û��������Ƿ���ȷ�Ĺ��̺���
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* loginout = (Logout*)szRecv;
		printf("access<Socket =%d> CMD_LOGOUT length: %d , UserName: %s \n", _cSock, loginout->dataLength, loginout->userName);
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

	//	1. ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("sock bulid errno\n");
	}
	else
	{
		printf("sock bulid success\n");
	}
	//	2. ���ӷ�����connect
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
		//3.������������
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		//4.��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("access exit command\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "demo");
			strcpy(login.PassWord, "demo0");
			send(_sock, (const char*)&login, sizeof(Login), 0);
			//���ܷ��������ص�����
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(LoginResult), 0);
			printf("loginresult: %d", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "demo");
			send(_sock, (const char*)&logout, sizeof(Logout), 0);

			//���ܷ��������ص�����
			LogoutResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
			printf("logoutresult: %d", logoutRet.result);
		}
		else {
			printf("access errno command,please again!\n");
		}

	}

	//	4. �ر�socket closesocket
	closesocket(_sock);
	//���windows socket����
	WSACleanup();
	getchar();
	printf("exit,mission fish\n");
	getchar();
	return 0;
}