#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

int main(void)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	char ip_address[20];
	printf("My IP address: ");
	scanf("%s", ip_address);

	unsigned short port;
	printf("My port #: ");
	scanf("%hu", &port);

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		fprintf(stderr, "[!]Failed to create a socket!\n");
		return 1;
	}

	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ip_address);
	sin.sin_port = htons(port);
	if (bind(s, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR)
	{
		fprintf(stderr, "[!] Failed to bind the socket!\n");
		return 1;
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
			sockaddr_in sin_from;
			int fromlen = sizeof sin_from;
			const int message_len = recvfrom(s, message, sizeof message - 1, 0, (sockaddr*)&sin_from, &fromlen);
			if (message_len >= 0)
			{
				message[message_len] = '\0';
				printf("From %s:%hu: %s", inet_ntoa(sin_from.sin_addr), ntohs(sin_from.sin_port), message);
			}
		}
		if (_kbhit() && getch() == 13)
		{
			char target_ip_addr[20];
			unsigned short target_port;
			printf("IP port message> ");
			scanf("%s %hu ", target_ip_addr, &target_port);

			char message_to_send[256];
			fgets(message_to_send, sizeof message_to_send, stdin);

			sockaddr_in sin_to = { 0 };
			sin_to.sin_family = AF_INET;
			sin_to.sin_addr.s_addr = inet_addr(target_ip_addr);
			sin_to.sin_port = htons(target_port);
			sendto(s, message_to_send, strlen(message_to_send), 0, (const sockaddr*)&sin_to, sizeof sin_to);
		}
	}

	return 0;
}