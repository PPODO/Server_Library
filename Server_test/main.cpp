#include <iostream>
#include <Network/UserSession/Server/User_Server.hpp>
#include <thread>

int main() {
	using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	User_Server server(SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_TCP);

	SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress address(1350);
	server.Initialize(address);

	std::thread t1([&]() {
		SERVER::NETWORK::PROTOCOL::TCP::TCPIPSocket client;
	
	while (!client.Connect(address));

	std::cout << "Connect!\n";

	char messageBuffer[1024] = { "\0" };
	uint16_t readbytes;
	while (true) {
		if (client.Read(messageBuffer, readbytes)) {
			std::cout << messageBuffer << std::endl;
			break;
		}
	}

	});

	User_Server client(SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_TCP);
	
	while (!client.Initialize(server));

	char message[1024] = "QWEQWE";
	while (true) {
		if (client.Send(message, 4)) break;
	}

	t1.join();
	WSACleanup();
	return 0;
}