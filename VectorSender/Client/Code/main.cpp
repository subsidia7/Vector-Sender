#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#define PORT 54000
#define SERVER_ADDRESS "127.0.0.1"
#define DELAY 10

void sendTo(SOCKET& client, std::string& message)
{
	int msg_size;
	msg_size = message.size();
	send(client, (char*)& msg_size, sizeof(int), 0);
	send(client, message.c_str(), msg_size, 0);
}

int recvFrom(SOCKET& client, char*& msg)
{
	int msg_size;
	recv(client, (char*)& msg_size, sizeof(int), 0);
	if (msg_size == 0)
	{
		return msg_size;
	}
	msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	recv(client, msg, msg_size, 0);
	return msg_size;
}

void openTheFile(std::ifstream& fin)
{
	std::string filename;
	while (true)
	{
		std::cout << "Enter the file path, don't forget about double backslash: ";
		std::cin >> filename;
		fin.open(filename);
		if (!fin.is_open())
		{
			std::cout << "Wrong filename" << std::endl;
		}
		else
		{
			std::cout << "File is open" << std::endl;
			return;
		}
	}
}

void ClientProcess(SOCKET& sock, std::ifstream& fin)
{
	openTheFile(fin);
	char* response = nullptr;
	std::string msg;
	int msg_size;
	while (getline(fin, msg))
	{
		msg += ' ';

		sendTo(sock, msg);

		recvFrom(sock, response);

		std::cout << msg << std::endl;

		std::cout << response << std::endl << std::endl;

		delete[] response;
		Sleep(DELAY);
	}
	msg_size = 0;
	msg.clear();
	sendTo(sock, msg);
}

int main(int argc, char *argv[])
{
	std::cout << "HELLO CLIENT" << std::endl;
	WSADATA wsData;
	if (WSAStartup(MAKEWORD(2,2), &wsData) != 0)
	{
		std::cout << "Fail with WSADATA" << std::endl;
		return 1;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Fail with creating socket" << std::endl;
		return 1;
	}
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDRESS, &hint.sin_addr);
	if (SOCKET_ERROR == connect(sock, (sockaddr*)& hint, sizeof(hint)))
	{
		std::cout << "Connection error" << std::endl;
		return 1;
	}
	std::ifstream fin;
	std::cout << "Connect is good" << std::endl << std::endl;
	ClientProcess(sock, fin);
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}