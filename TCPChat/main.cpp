#include <stdio.h>
#include <conio.h>
#include <string.h>
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

int main(void)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	int role = 1;
	printf("Choose my role: 1 - Client / 2 - Server\n> ");
	scanf("%d", &role);
	getchar();

	SOCKET s = INVALID_SOCKET;

	if (role == 1)
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET)
		{
			fprintf(stderr, "[!]Failed to create a socket!\n");
			return 1;
		}

		char sever_ip_address[20];
		printf("Sever IP address: ");
		scanf("%s", sever_ip_address);
		getchar();

		sockaddr_in sin = { 0 };
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(sever_ip_address);
		sin.sin_port = htons(5000);
		if (connect(s, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR)
		{
			fprintf(stderr, " [!] Failed to connect to the server! \n");
			return 1;
		}
	}
	else
	{
		SOCKET listener_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listener_socket == INVALID_SOCKET)
		{
			fprintf(stderr, "[!]Failed to create a listener socket!\n");
			return 1;
		}

		sockaddr_in sin = { 0 };
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		sin.sin_port = htons(5000);
		if (bind(listener_socket, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR)
		{
			fprintf(stderr, "[!] Failed to bind the listener socket!\n");
			return 1;
		}

		if (listen(listener_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			fprintf(stderr, "[!] Failed to make the listener socket in the listening mode!\n");
			return 1;
		}

		sockaddr_in client_addr;
		int client_addr_len = sizeof client_addr;
		s = accept(listener_socket, (sockaddr*)&client_addr, &client_addr_len);
		if (s == INVALID_SOCKET)
		{
			fprintf(stderr, "[!] Failed to accept an incoming connection request!\n");
			return 1;
		}

		closesocket(listener_socket);
	}

	for (;;)
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		if (select(0, &readfds, NULL, NULL, &tv) > 0)
		{
			char message[256];
			const int message_len = recv(s, message, sizeof message - 1, 0);
			if (message_len >= 0)
			{
				message[message_len] = '\0';
				printf("%s", message);
			}
		}
		if (_kbhit() && getch() == 13)
		{
			char message_to_send[256];
			printf("message> ");
			fgets(message_to_send, sizeof message_to_send, stdin);
			send(s, message_to_send, strlen(message_to_send), 0);
		}
	}

	return 0;
}