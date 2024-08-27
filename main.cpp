#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ws2_32")

#pragma pack(push, 1)
struct FPacket
{
	int Type;
	char UserID[10];
	char UserPassword[10];
};
#pragma pack(pop)

void GetLineEnd(char* str)
{
	size_t len = strlen(str);
	if (len > 0 && str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
}

int main()
{

	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in ServerAddress;
	memset(&ServerAddress, 0, sizeof(ServerAddress));

	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_port = htons(10880);
	ServerAddress.sin_family = AF_INET;

	int Result = bind(ServerSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));

	if (Result != 0)
	{
		printf("Bind Error");
		exit(-1);
	}

	Result = listen(ServerSocket, 5);

	if (Result != 0)
	{
		printf("listen Error");
		exit(-1);
	}

	fd_set ClientServerSockets;
	FD_ZERO(&ClientServerSockets);
	FD_SET(ServerSocket, &ClientServerSockets);

	fd_set CopySocketList;
	FD_ZERO(&CopySocketList);

	timeval Timer;
	Timer.tv_sec = 0;
	Timer.tv_usec = 10;

	char MainBuffer[1024] = { 0, };
	char Message[1024] = "OK";

	while (true)
	{
		CopySocketList = ClientServerSockets;
		int SelectedResult = select(0, &CopySocketList, 0, 0, &Timer);

		if (SelectedResult <= 0)
		{
			continue;
		}
		for (unsigned int i = 0; i < CopySocketList.fd_count; ++i)
		{
			if (CopySocketList.fd_array[i] == ServerSocket)
			{
				struct sockaddr_in ClientAddress;
				memset(&ClientAddress, 0, sizeof(ClientAddress));
				int ClientAddressSize = sizeof(ClientAddress);
				SOCKET ClientSocket = accept(ServerSocket, (sockaddr*)&ClientAddress, &ClientAddressSize);

				if (ClientSocket == INVALID_SOCKET)
				{
					printf("Accept Error\n");
					continue;
				}

				FD_SET(ClientSocket, &ClientServerSockets);
			}
			else
			{
				int ReciveLength = recv(CopySocketList.fd_array[i], MainBuffer, sizeof(MainBuffer), 0);


				if (ReciveLength <= 0)
				{
					printf("Recive Error\n");
				}

				else
				{
					printf("Recive Success\n");
					FPacket UserData;
					memcpy(&UserData, MainBuffer, sizeof(FPacket));
					int SendLength = 0;

					printf("%s , %s , %d\n", UserData.UserID, UserData.UserPassword, UserData.Type);

					FILE* txtFile;

					if(UserData.Type == 1)
					{

						txtFile = fopen("test.txt", "r");
						char Buffer[10] = "";
						const char* User = UserData.UserID;

						if(txtFile == nullptr)
						{
							printf("File open error\n");
							std::cerr << "File open error: " << strerror(errno) << std::endl;
							return false;
						}

						while(fgets(Buffer, 10, txtFile) != nullptr)
						{
							GetLineEnd(Buffer);

							if (strcmp(Buffer, User) == 0)
							{
								if (fgets(Buffer, 10, txtFile))
								{
									GetLineEnd(Buffer);
									User = UserData.UserPassword;

									if (strcmp(Buffer, User) == 0)
									{
										SendLength = send(CopySocketList.fd_array[i], Message, (int)strlen(Message), 0);
										printf("Send Reult to Client \n");
										break;
									}
								}
							}
						}

						fclose(txtFile);

					}

					else if(UserData.Type == 2)
					{
						txtFile = fopen("test.txt", "a");

						if (txtFile == nullptr)
						{
							printf("File open error\n");
							std::cerr << "File open error: " << strerror(errno) << std::endl;
							return false;
						}

						char Buffer[10] = "";
						fseek(txtFile, 0, SEEK_END);

						fprintf(txtFile, "\n");

						const char* text = UserData.UserID;
						fprintf(txtFile, "%s\n", text);

						text = UserData.UserPassword;
						fprintf(txtFile, "%s\n", text);

						SendLength = send(CopySocketList.fd_array[i], Message, (int)strlen(Message), 0);

						fseek(txtFile, 0, SEEK_SET);
						fclose(txtFile);
					}
					

					if (SendLength <= 0)
					{
						printf("Send Error");
						closesocket(CopySocketList.fd_array[i]);
						FD_CLR(CopySocketList.fd_array[i], &ClientServerSockets);
					}
					else
					{
						printf("Send Suceed");
					}

				}

			}
		}

	}


	closesocket(ServerSocket);
	WSACleanup();


	return 0;
}