#pragma once
#include <Functions/Functions/SocketAddress/SocketAddress.h>
#include <memory>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				class CServerSession;
			}
		}
	}

	namespace SOCKET { 
		const short INVALID_SOCKET_VALUE = static_cast<::SOCKET>(-1);
		namespace BASESOCKET {
			class CBaseSocket;
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					enum class EIOTYPE : uint8_t {
						EIT_NONE,
						EIT_DISCONNECT,
						EIT_ACCEPT,
						EIT_READ,
						EIT_WRITE,
					};

					// 서버에서만 사용 가능합니다.
					typedef struct OVERLAPPED_EX {
						WSAOVERLAPPED m_Overlapped;
						EIOTYPE m_IOType;
						SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* m_Owner;

					public:
						OVERLAPPED_EX() : m_IOType(EIOTYPE::EIT_NONE), m_Owner(nullptr) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); };
						OVERLAPPED_EX(const EIOTYPE& Type, SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* Owner) : m_IOType(Type), m_Owner(Owner) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); };

					};
				}
			}
		}

		namespace BASESOCKET {
			enum class EPROTOCOLTYPE {
				EPT_TCP = (1 << 0),
				EPT_UDP = (1 << 1),
				EPT_BOTH = (EPT_TCP | EPT_UDP)
			};

			inline ::SOCKET GetSocketValue(const SOCKET::BASESOCKET::CBaseSocket& Socket);

			inline bool operator&(const EPROTOCOLTYPE& lhs, const EPROTOCOLTYPE& rhs) {
				return (static_cast<int>(lhs) & static_cast<int>(rhs));
			}

			static inline ::SOCKET CreateSocketByProtocolType(const EPROTOCOLTYPE& ProtocolType) {
				::SOCKET NewSocket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
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
				friend ::SOCKET UTIL::BASESOCKET::GetSocketValue(const CBaseSocket& Socket);
			private:
				::SOCKET m_Socket;

			private:
				char m_ReceiveMessageBuffer[MAX_RECEIVE_BUFFER_SIZE];

			private:
				void Destroy();

			protected:
				inline char* const GetReceiveBufferPtr() { return m_ReceiveMessageBuffer; }
				inline ::SOCKET GetSocketHandle() const { return m_Socket; }

			public:
				explicit CBaseSocket(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CBaseSocket() = 0;

			public:
				bool Bind(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				bool CopyReceiveBuffer(char* const Buffer, const uint16_t& RecvSize);

			};
		}
	}

	namespace UTIL {
		namespace BASESOCKET {
			inline ::SOCKET GetSocketValue(const SOCKET::BASESOCKET::CBaseSocket& Socket) {
				return Socket.m_Socket;
			}
		}
	}
}