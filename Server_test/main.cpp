#include <iostream>
#include <Network/UserSession/Server/User_Server.hpp>
#include <Network/NetworkProtocol/UDPSocket.hpp>
#include <Network/NetworkProtocol/TCPSocket.hpp>
#include <thread>

int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// udp test
	{
		using namespace SERVER::NETWORK::PROTOCOL::UDP;

		UDPIPSocket udpSocket;
		SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress(3550);

		if (!udpSocket.Bind(bindAddress)) {
			std::cout << "UDP Bind Failed!\n";
			return -1;
		}

		std::thread t1([]() {
			UDPIPSocket clientSocket;
		SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress serverAddress(3550);
		char messageBuffer[] = { "Hello World!\n" };

		for (int i = 0; i < 100; i++)
			clientSocket.WriteTo(serverAddress, messageBuffer, strlen(messageBuffer));
			});

		char messageBuffer[1024] = { "\0" };
		uint16_t receivedBytes = 0;

		for (int i = 0; i < 100; i++) {
			udpSocket.ReadFrom(messageBuffer, receivedBytes);
			if (receivedBytes > 0)
				break;
		}

		std::cout << messageBuffer << std::endl;

		t1.join();
	}

	// tcp test
	{
		using namespace SERVER::NETWORK::PROTOCOL::TCP;

		TCPIPSocket tcpSocket;
		SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress(1350);

		tcpSocket.Bind(bindAddress);
		tcpSocket.Listen();

		std::thread t1([&]() {
			TCPIPSocket clientSocket;
		while (!clientSocket.Connect(bindAddress));

		char messageBuffer[] = { "Hello World!!!\n" };
		for (int i = 0; i < 10; i++) {
			clientSocket.Write(messageBuffer, strlen(messageBuffer));
		}
			});

		SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX overlap;
		TCPIPSocket clientSocket;
		while (!clientSocket.Accept(tcpSocket, overlap));

		char messageBuffer[1024] = { "\0" };
		uint16_t recvBytes = 0;
		for (int i = 0; i < 10; i++) {
			clientSocket.Read(messageBuffer, recvBytes);
		}

		std::cout << messageBuffer << std::endl;

		t1.join();
	}
	WSACleanup();
	return 0;
}