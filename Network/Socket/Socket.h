#pragma once
#include <Functions/Functions/SocketAddress/SocketAddress.h>

namespace NETWORK {
	namespace SOCKET { const short INVALID_SOCKET_VALUE = static_cast<::SOCKET>(-1); }

	namespace UTIL {
		namespace BASESOCKET {
			enum EPROTOCOLTYPE {
				EPT_TCP = (1 << 0),
				EPT_UDP = (1 << 1),
			};

			enum class EIOTYPE : uint8_t {
				EIT_NONE,
				EIT_ACCEPT,
				EIT_READ,
				EIT_WRITE,
			};

			typedef struct OVERLAPPED_EX {
				WSAOVERLAPPED m_Overlapped;
				EIOTYPE m_IOType;
				void* m_Owner;

			public:
				OVERLAPPED_EX() : m_IOType(EIOTYPE::EIT_NONE), m_Owner(nullptr) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); };
				OVERLAPPED_EX(const EIOTYPE& Type, void* const Owner) : m_IOType(Type), m_Owner(Owner) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); };

			};

			inline ::SOCKET CreateSocketByProtocolType(const EPROTOCOLTYPE& ProtocolType) {
				::SOCKET NewSocket = ProtocolType == EPROTOCOLTYPE::EPT_TCP ? WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED) : WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
				if (NewSocket == SOCKET::INVALID_SOCKET_VALUE) {
					return SOCKET::INVALID_SOCKET_VALUE;
				}
				return NewSocket;
			}
		}
	}

	namespace SOCKET {
		namespace BASESOCKET {
			static const size_t MAX_RECEIVE_BUFFER_SIZE = 1024;

			class CBaseSocket {
			private:
				::SOCKET m_Socket;

			private:
				char m_ReceiveMessageBuffer[MAX_RECEIVE_BUFFER_SIZE];

			private:
				void Destroy();

			protected:
				inline char* const GetReceiveBufferPtr() { return m_ReceiveMessageBuffer; }

			public:
				explicit CBaseSocket(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CBaseSocket() = 0;

			public:
				bool Bind(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);

			public:
				inline ::SOCKET GetSocketHandle() const { return m_Socket; }

			};
		}
	}
}