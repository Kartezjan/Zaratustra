#pragma once

#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>


#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 65536
#define DEFAULT_HTTP_PORT "80"

class httpSock
{
public:
	httpSock(std::string aHost, std::string aPort) {
		host = aHost;
		port = aPort;
		connectSocket = INVALID_SOCKET;
	};
	~httpSock();
	int Connect();
	int SendM(std::string message);
	std::string RecM();
	std::string RecNChars(size_t bytes);
	bool IsConnected();
	void Disconnect();
	std::string getHost() { return host; };
	std::string getPort() { return port; };;
	int Get(std::string);
	int Post(std::string, std::string);
private:
	SOCKET connectSocket;
	int lastRecvMSize;
	std::string host;
	std::string port;
};

#pragma once
