#include <iostream>
#include <NetworkModel/BaseModel/BaseModel.hpp>
#include <NetworkModel/IOCP/IOCP.hpp>
#include <thread>

void pk(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {

}

int main() {
/*	WSADATA wsaData;
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
	WSACleanup();*/

	{
		using namespace SERVER::NETWORKMODEL::IOCP;

		SERVER::NETWORKMODEL::BASEMODEL::PACKETPROCESSOR processor;
		processor.insert(std::make_pair<uint8_t, void(SERVER::NETWORK::PACKET::PacketQueueData*)>(1, pk));

		SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress address(1335);

		IOCP iocp(processor, 1);

		iocp.Initialize(SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_TCP, address);


		std::thread t1([&address]() {
			SERVER::NETWORK::PROTOCOL::TCP::TCPIPSocket tcpip;

			tcpip.Connect(address);

			char messageBuffer[1024];
			while (true) {
				std::cin >> messageBuffer;

				if (!tcpip.Write(messageBuffer, strlen(messageBuffer))) {
					std::cout << "send failed\n";
				}
			}
		});

		while (true) {
			iocp.Run();
		}

		t1.join();
		iocp.Destroy();
	}

	return 0;
}