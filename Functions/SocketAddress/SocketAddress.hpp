#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <utility>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace SERVER {
	namespace FUNCTIONS {
		namespace UTIL {
			static in_addr* GetHostAddress() {
				static const size_t MAX_HOST_NAME = 32;
				char sHostName[MAX_HOST_NAME] = { "\0" };

				if (gethostname(sHostName, MAX_HOST_NAME) != SOCKET_ERROR)
					if (hostent* pHostInformation = gethostbyname(sHostName))
						return reinterpret_cast<in_addr*>(pHostInformation);

				return nullptr;
			}
		}

		namespace SOCKETADDRESS {
			class SocketAddress {
			private:
				sockaddr m_address;

			private:
				__forceinline sockaddr_in* const GetSocketAddress() { return reinterpret_cast<sockaddr_in* const>(&m_address); }

			public:
				SocketAddress() { ZeroMemory(&m_address, sizeof(sockaddr)); }

				SocketAddress(const std::string& sAddress, const uint16_t& iPort) {
					if (sAddress.size() > 0)
						InetPtonA(AF_INET, sAddress.c_str(), &GetSocketAddress()->sin_addr);
					else if (auto address = UTIL::GetHostAddress())
							GetSocketAddress()->sin_addr = *address;

					GetSocketAddress()->sin_family = AF_INET;
					GetSocketAddress()->sin_port = htons(iPort);
				}

				SocketAddress(const uint16_t iPort) : SocketAddress("127.0.0.1", iPort) {}


				SocketAddress(const sockaddr& socketAddress) {
					CopyMemory(&m_address, &socketAddress, GetSize());
				}

				SocketAddress(const SocketAddress& socketAddress) : SocketAddress(socketAddress.m_address) {}
				
				SocketAddress(const sockaddr_in& socketAddress) : SocketAddress(reinterpret_cast<const sockaddr&>(socketAddress)) {}

			public:
				const SocketAddress& operator=(const sockaddr& socketAddress) {
					CopyMemory(&m_address, &socketAddress, GetSize());

					return (*this);
				}

				const SocketAddress& operator=(const SocketAddress& socketAddress) {
					return this->operator=(socketAddress.m_address);
				}

				const SocketAddress& operator=(const sockaddr_in& socketAddress) {
					return this->operator=(reinterpret_cast<const sockaddr&>(socketAddress));
				}

			public:
				operator sockaddr_in() {
					return *GetSocketAddress();
				}

				const sockaddr* operator&() const {
					return &m_address;
				}

				static inline size_t GetSize() { return sizeof(m_address); }

			public:
				std::pair<std::string, USHORT> GetIPAddressAndPort() {
					using namespace std;
					auto sockAddr = GetSocketAddress();

					return make_pair<string, USHORT>(move(inet_ntoa(sockAddr->sin_addr)), move(sockAddr->sin_port));
				}
			};

			static bool operator==(const SocketAddress& lhs, const SocketAddress& rhs) {
				auto lhsValue = const_cast<SocketAddress&>(lhs).GetIPAddressAndPort();
				auto rhsValue = const_cast<SocketAddress&>(rhs).GetIPAddressAndPort();

				return lhsValue == rhsValue;
			}
		}
	}
}