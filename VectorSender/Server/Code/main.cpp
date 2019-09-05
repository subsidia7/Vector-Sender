#include <iostream>
#include <string>
#include <vector>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define MAX_VECTOR_SIZE 10
#define MIN_VECTOR_SIZE 5
#define PORT 54000

bool isDouble(const std::string & str)
{
	bool dotIsHere = false;
	int strLength = str.length();
	if (!(str[0] >= '0' && str[0] <= '9')
		|| !(str[strLength - 1] >= '0' && str[strLength - 1] <= '9'))
	{
		return false;
	}
	for (size_t i = 1; i < strLength - 1; i++)
	{
		if (!(str[i] >= '0' && str[i] <= '9'))
		{
			if (str[i] == '.')
			{
				if (dotIsHere)
				{
					return false;
				}
				dotIsHere = true;
				continue;
			}
			return false;
		}

	}
	return true;
}

bool stringToVector(const std::string& str, std::vector<double>& vec)
{
	std::string elemForVector;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == ' ')
		{
			if (isDouble(elemForVector))
			{
				vec.push_back(stod(elemForVector));
				if (vec.size() > MAX_VECTOR_SIZE)
				{
					return false;
				}
			}
			elemForVector.clear();
			continue;
		}
		elemForVector += str[i];
	}
	if (vec.size() < MIN_VECTOR_SIZE)
	{
		return false;
	}
	return true;
}

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

void ClientProcess(SOCKET& client)
{
	char* msg = nullptr;
	int requestNumber = 0;
	double sum;
	std::string response;
	std::vector<double> vec;
	while (true)
	{
		if (recvFrom(client, msg) == 0)
		{
			std::cout << "Session ends" << std::endl;
			break;
		}
		requestNumber++;
		sum = 0;
		std::cout << msg << std::endl;
		if (!stringToVector(msg, vec))
		{
			vec.clear();
			response = "Vector" + std::to_string(requestNumber) + " is not in 5..10 range";
			sendTo(client, response);
			continue;
		}
		for (std::vector<double>::iterator iter = vec.begin(); iter != vec.end(); iter++)
		{
			sum += *iter;
		}
		response = "Vector" + std::to_string(requestNumber) + " sum = " + std::to_string(sum);
		sendTo(client, response);
		vec.clear();
		delete[] msg;
		msg = nullptr;
	}
	if (msg)
	{
		delete[] msg;
	}
}

int main(int argc, char* argv[])
{
	WSADATA wsData;
	if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
	{
		std::cerr << "Failed to startup WSADATA" << std::endl;
		WSACleanup();
		return 1;
	}
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Failed to create listening socket" << std::endl;
		WSACleanup();
		return 1;
	}
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listening, (SOCKADDR*)& hint, sizeof(hint)) == -1)
	{
		std::cerr << "Bind error" << std::endl;
		WSACleanup();
		return 1;
	}
	if (listen(listening, SOMAXCONN) != 0)
	{
		std::cerr << "Listen error" << std::endl;
		WSACleanup();
		return 1;
	}
	sockaddr_in clientHint;
	int nClientHintSize = sizeof(clientHint);
	SOCKET client;

	if ((client = accept(listening, (SOCKADDR*)& clientHint, &nClientHintSize)) != INVALID_SOCKET)
	{
		ClientProcess(client);
	}
	closesocket(listening);
	closesocket(client);
	WSACleanup();
	system("pause");
	return 0;
}