#include <iostream>
#include <Network/NetworkProtocol/TCPSocket.hpp>

int main() {
	using namespace SERVER::NETWORK::PROTOCOL::TCP;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2),  &wsaData);

	TCPIPSocket tcpipSocket;

	tcpipSocket;

	WSACleanup();
	return 0;
}