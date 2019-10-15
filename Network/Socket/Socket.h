#pragma once
#include <Functions/Functions/SocketAddress/SocketAddress.h>
#include <Functions/Functions/Exception/Exception.h>
#include <Network/Error/ErrorCode.h>
#include <memory>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {
			class CServerSession;
		}
	}

	namespace SOCKET { 
		const short INVALID_SOCKET_VALUE = static_cast<::SOCKET>(0);
		namespace BASESOCKET {
			class CBaseSocket;
		}
	}

	namespace UTIL {
		namespace SESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					enum class EIOTYPE : uint8_t {
						EIT_NONE,
						EIT_DISCONNECT,
						EIT_ACCEPT,
						EIT_READ,
						EIT_WRITE,
						EIT_READFROM,
						EIT_WRITETO
					};

					// 서버에서만 사용 가능합니다.
					struct OVERLAPPED_EX {
						WSAOVERLAPPED m_Overlapped;
						// WSABUF에 읽은 데이터가 저장됨.
						WSABUF m_WSABuffer;
						// 처리하고 남은 바이트 수. 이전에 읽은 데이터가 남아 있을 경우, 데이터가 덮어씌워지는데 이를 방지하기 위한 변수임.
						int16_t m_RemainReceivedBytes;
						// 데이터가 덮어씌워지지 않도록 처리를 하였을데, 이전에 읽은 데이터가 남아있는 위치부터 현재 읽은 데이터까지 처리를 해야하는데, 이전에 읽은 데이터의 위치를 모르기 때문에, 그 위치를 저장하는 변수
						char* m_SocketMessage;
						// UDP에서 사용(RecvFrom)
						FUNCTIONS::SOCKADDR::CSocketAddress m_RemoteAddress;

						EIOTYPE m_IOType;
						NETWORK::SESSION::SERVERSESSION::CServerSession* m_Owner;

					public:
						OVERLAPPED_EX() : m_IOType(EIOTYPE::EIT_NONE), m_Owner(nullptr), m_RemainReceivedBytes(0), m_SocketMessage(nullptr) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); ZeroMemory(&m_WSABuffer, sizeof(WSABUF)); };
						OVERLAPPED_EX(const EIOTYPE& Type, NETWORK::SESSION::SERVERSESSION::CServerSession* Owner) : m_IOType(Type), m_Owner(Owner), m_RemainReceivedBytes(0), m_SocketMessage(nullptr) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); ZeroMemory(&m_WSABuffer, sizeof(WSABUF)); };

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
				::SOCKET NewSocket = WSASocketA(AF_INET, ProtocolType == EPROTOCOLTYPE::EPT_TCP ? SOCK_STREAM : SOCK_DGRAM, ProtocolType == EPROTOCOLTYPE::EPT_TCP ? IPPROTO_TCP : IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
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
				void CopyReceiveBuffer(char* const Buffer, const uint16_t& RecvSize);

			protected:
				inline char* const GetReceiveBufferPtr() { return m_ReceiveMessageBuffer; }

			public:
				explicit CBaseSocket(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CBaseSocket() = 0;

			public:
				bool Bind(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);

			public:
				inline ::SOCKET GetSocket() const { return m_Socket; }

			};
		}
	}

	namespace UTIL {
		namespace BASESOCKET {
			inline ::SOCKET GetSocketValue(const SOCKET::BASESOCKET::CBaseSocket& Socket) {
				return Socket.m_Socket;
			}

			inline NETWORK::ERRORCODE::ENETRESULT SetSockOption(const ::SOCKET& Socket, const int32_t& Level, const int32_t& OptionName, void* const OptionVariable, const size_t& OptionLen) {
				if (setsockopt(Socket, Level, OptionName, reinterpret_cast<char*>(OptionVariable), reinterpret_cast<const int&>(OptionLen)) == SOCKET_ERROR) {
					return NETWORK::ERRORCODE::ENETRESULT::ENETSOCKTOPFAIL;
				}
				return NETWORK::ERRORCODE::ENETRESULT::ENETSUCCESS;
			}
		}
	}
}

namespace FUNCTIONS {
	namespace EXCEPTION {
		struct bad_sockopt : public std::exception {
		public:
			 char* const what() {
				return const_cast<char* const>(std::string("Exception : Failed To Set Socket Option - " + WSAGetLastError()).c_str());
			}
		};
	}
}