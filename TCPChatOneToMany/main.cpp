#include <stdio.h>
#include <stdlib.h>		//exit()
#include <conio.h>
#include <string.h>
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

void client(void)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		fprintf(stderr, "[!]Failed to create a socket!\n");
		exit(1);
	}

	char sever_ip_address[20];
	printf("Sever IP address: ");
	scanf("%s", sever_ip_address);

	char nickname[32];
	printf("My nickname: ");
	scanf("%s", nickname);

	getchar();

	printf("Connecting to the server...\n");

	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(sever_ip_address);
	sin.sin_port = htons(5000);
	if (connect(s, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR)
	{
		fprintf(stderr, " [!] Failed to connect to the server! \n");
		exit(1);
	}

	printf("Connected to the server!\n");
	send(s, nickname, strlen(nickname), 0);

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
			char msg[1024];
			const int msg_len = recv(s, msg, sizeof msg - 1, 0);
			if (msg_len >= 0)
			{
				msg[msg_len] = '\0';
				printf("%s", msg);
			}
			else
			{
				fprintf(stderr, "[!] Disconnected from the server!\n");
				exit(1);
			}
		}

		if (_kbhit() && getch() == 13)
		{
			char msg_to_send[1024];
			printf("%s> ", nickname);
			fgets(msg_to_send, sizeof msg_to_send, stdin);
			send(s, msg_to_send, strlen(msg_to_send), 0);
		}
	}
}

void server(void)
{
	SOCKET listener_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//채팅 참가자 정보
	SOCKET parti[128];
	char parti_nick[128][32];
	int parti_count = 0;

	if (listener_socket == INVALID_SOCKET)
	{
		fprintf(stderr, "[!]Failed to create a listener socket!\n");
		exit(1);
	}

	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(5000);
	if (bind(listener_socket, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR)
	{
		fprintf(stderr, "[!] Failed to bind the listener socket!\n");
		exit(1);
	}

	if (listen(listener_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		fprintf(stderr, "[!] Failed to make the listener socket in the listening mode!\n");
		exit(1);
	}

	printf("Listening for incoming connection requests...\n");

	//감시 대상 fd들
	fd_set fds_to_monitor;
	FD_ZERO(&fds_to_monitor);
	FD_SET(listener_socket, &fds_to_monitor);

	for (;;)
	{
		fd_set readfds;
		readfds = fds_to_monitor;

		if (select(0, &readfds, NULL, NULL, NULL) > 0)
		{
			if (FD_ISSET(listener_socket, &readfds))
			{
				sockaddr_in client_sin;
				int client_sin_len = sizeof client_sin;
				SOCKET s = accept(listener_socket, (sockaddr*)&client_sin, &client_sin_len);
				if (s != INVALID_SOCKET)
				{
					parti[parti_count] = s;
					const int nickname_len = recv(s, parti_nick[parti_count], 32, 0);
					if (nickname_len >= 0)
					{
						FD_SET(s, &fds_to_monitor);
						parti_nick[parti_count][nickname_len] = '\0';
						++parti_count;

						printf("Server> %s entered the room!\n", parti_nick[parti_count - 1]);

						char msg[1024];
						sprintf(msg, "Server> The room participants: ");
						for (int i = 0; i < parti_count; ++i)
						{
							if (i > 0)strcat(msg, ",");
							strcat(msg, parti_nick[i]);
						}
						strcat(msg, "\n");
						send(s, msg, strlen(msg), 0);

						sprintf(msg, "Server> %s entered the room!\n", parti_nick[parti_count - 1]);
						for (int i = 0; i < parti_count; ++i)
						{
							send(parti[i], msg, strlen(msg), 0);
						}
					}
					else
					{
						closesocket(s);
					}
				}
			}
			for (int i = 0; i < parti_count; ++i)
			{
				if (FD_ISSET(parti[i], &readfds))
				{
					char msg[1024];
					const int msg_len = recv(parti[i], msg, sizeof msg - 1, 0);
					if (msg_len >= 0)
					{
						msg[msg_len] = '\0';
						printf("%s> %s", parti_nick[i], msg);

						char msg_with_nick[1024];
						sprintf(msg_with_nick, "%s>%s", parti_nick[i], msg);
						for (int j = 0; j < parti_count; ++j)
						{
							if (j != i)
							{
								send(parti[j], msg_with_nick, strlen(msg_with_nick), 0);
							}
						}
					}
					else
					{
						FD_CLR(parti[i], &fds_to_monitor);
						closesocket(parti[i]);

						printf("Server> %s left the room.\n", parti_nick[i]);

						char quit_msg[1024];
						sprintf(quit_msg, "Server> %s left the room.\n", parti_nick[i]);
						parti[i] = parti[parti_count - 1];
						strcpy(parti_nick[i], parti_nick[parti_count - 1]);
						--parti_count;

						for (int j = 0; j < parti_count; ++j)
						{
							send(parti[j], quit_msg, strlen(quit_msg), 0);

						}
					}
				}
			}
		}

	}
}

int main(void)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	int role = 1;
	printf("Choose my role: 1 - Client / 2 - Server\n> ");
	scanf("%d", &role);
	getchar();

	if (role == 1)
	{
		client();
	}
	else
	{
		server();
	}

	return 0;
}