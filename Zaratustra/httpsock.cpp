#include "httpsock.h"

httpSock::~httpSock()
{
	closesocket(connectSocket);
	connectSocket = INVALID_SOCKET;
	WSACleanup();
}

int httpSock::Connect()
{
	WSADATA wsaData;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	for (ptr = result;ptr != NULL;ptr = ptr->ai_next)
	{
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	return 0;
}

int httpSock::SendM(std::string message)
{
	return send(connectSocket, message.c_str(), message.size(), 0);
}

std::string httpSock::RecM()
{
	std::string message = "";
	std::string recvbuf;
	recvbuf.resize(DEFAULT_BUFLEN);
	int result;
	result = recv(connectSocket, &recvbuf[0], DEFAULT_BUFLEN, 0);
	//printf("Bytes Received: %d\n", result);
	if (result <= 0)
		return "ERROR\n";
	do
	{
		recvbuf[result] = '\0';
		message.append(recvbuf);
		result = recv(connectSocket, &recvbuf[0], DEFAULT_BUFLEN, 0);
		//printf("Bytes Received: %d\n", result);
	} while (result > 0);
	return message;
}

std::string httpSock::RecNChars(size_t chars)
{
	std::string message = "";
	std::string recvbuf;
	recvbuf.resize(DEFAULT_BUFLEN);
	int result;
	result = recv(connectSocket, &recvbuf[0], DEFAULT_BUFLEN, 0);
	int total = result;
	//printf("Bytes Received: %d\n", result);
	if (result <= 0)
		return "ERROR\n";
	do
	{
		recvbuf[result] = '\0';
		message.append(recvbuf);
		result = recv(connectSocket, &recvbuf[0], DEFAULT_BUFLEN, 0);
		total += result;
		//printf("Bytes Received: %d\n", result);
	} while (result > 0 && total < chars);
	return message;
}

bool httpSock::IsConnected()
{
	if (connectSocket == INVALID_SOCKET)
		return false;
	return true;
}

int httpSock::Get(std::string url)
{
	Connect();
	std::stringstream buffer;
	buffer << "GET " << url << " HTTP/1.1\r\n" <<
		"Host: " << host << "\r\n" <<
		"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:38.0) Gecko/20100101 Firefox/38.0\r\n" <<
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" <<
		"Accept-Language: en-US,en;q=0.5\r\n" <<
		"Accept-Encoding: none\r\n" <<
		"Connection: close\r\n\r\n";

	return SendM(buffer.str());
}


int httpSock::Post(std::string url, std::string data)
{
	std::stringstream buffer;
	buffer << "POST " << url << " HTTP/1.1\r\n"
		<< "Host: " << host << "\r\n"
		<< "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:38.0) Gecko/20100101 Firefox/38.0\r\n"
		<< "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
		<< "Accept-Language: en-US,en;q=0.5\r\n"
		<< "Accept-Encoding: none\r\n"
		<< "Connection: close\r\n"
		<< "Content-Type: application/x-www-form-urlencoded\r\n"
		<< "Content-Length: " << data.length() << "\r\n\r\n"
		<< data << "\r\n";

	return SendM(buffer.str());
}