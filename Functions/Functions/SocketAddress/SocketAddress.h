#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace FUNCTIONS {
	namespace UTIL {
		namespace SOCKETADDRESS {
			static const size_t MAX_HOST_NAME = 32;

			static in_addr* GetHostAddress() {
				char HostName[MAX_HOST_NAME] = { "\0" };

				if (gethostname(HostName, MAX_HOST_NAME) == SOCKET_ERROR) {

					return nullptr;
				}
				if (hostent * HostInformation = gethostbyname(HostName)) {
					return reinterpret_cast<in_addr*>(HostInformation->h_addr_list[0]);
				}

				return nullptr;
			}
		}
	}

	namespace SOCKADDR {
		class CSocketAddress {
		private:
			sockaddr m_Address;

		private:
			inline sockaddr_in* const GetSockAddress() { return reinterpret_cast<sockaddr_in* const>(&m_Address); }

		public:
			CSocketAddress() {
				ZeroMemory(&m_Address, GetSize());
			}

			explicit CSocketAddress(const unsigned short& Port) {
				if (auto Addr = UTIL::SOCKETADDRESS::GetHostAddress()) {
					GetSockAddress()->sin_addr = *Addr;
				}
				else {
					InetPtonA(AF_INET, "127.0.0.1", &GetSockAddress()->sin_addr);
				}
				GetSockAddress()->sin_family = AF_INET;
				GetSockAddress()->sin_port = htons(Port);
			}

			explicit CSocketAddress(const std::string& Address, const unsigned short& Port) : CSocketAddress(Port) {
				InetPtonA(AF_INET, Address.c_str(), &GetSockAddress()->sin_addr);
			}

			explicit CSocketAddress(const CSocketAddress& SocketAddress) {
				CopyMemory(&m_Address, &SocketAddress.m_Address, GetSize());
			}

		public:
			const CSocketAddress& operator=(const CSocketAddress& SocketAddress) {
				CopyMemory(&m_Address, &SocketAddress.m_Address, GetSize());

				return (*this);
			}

		public:
			operator sockaddr_in() {
				return *GetSockAddress();
			}

			 const sockaddr* operator&() const {
				return &m_Address;
			}

		public:
			static inline size_t GetSize() { return sizeof(m_Address); }

		};
	}
}