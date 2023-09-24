#pragma once
#include "../User.hpp"
#include <memory>

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			namespace USER_SERVER {
				class User_Server;

				enum class EIOTYPE : uint8_t {
					EIT_NONE,
					EIT_DISCONNECT,
					EIT_ACCEPT,
					EIT_READ,
					EIT_WRITE,
					EIT_READFROM,
					EIT_WRITETO
				};

				// Only Server Session :)
				struct OVERLAPPED_EX {
					// overlapped inst
					WSAOVERLAPPED m_wsaOverlapped;
					// 수신 버퍼
					WSABUF m_wsaBuffer;
					// 남은 바이트 수. 이전의 읽은 데이터를 덮어 씌우지 않게 하기 위함.
					uint16_t m_iRemainReceiveBytes;
					// Receive Buffer의 시작 주소
					char* m_pReceiveBuffer;

					// UDP에서만 사용
					FUNCTIONS::SOCKETADDRESS::SocketAddress m_remoteAddress;
					int16_t m_iLastReceivedPacketNumber;

					EIOTYPE m_IOType;
					User_Server* m_pOwner;

				public:
					OVERLAPPED_EX() : m_iRemainReceiveBytes(0), m_pReceiveBuffer(nullptr),
									  m_iLastReceivedPacketNumber(0), m_pOwner(nullptr),
									  m_IOType(EIOTYPE::EIT_NONE), m_wsaOverlapped() {
						ZeroMemory(&m_wsaOverlapped, sizeof(WSAOVERLAPPED));
						ZeroMemory(&m_wsaBuffer, sizeof(WSABUF));
					}

					OVERLAPPED_EX(const EIOTYPE ioType, User_Server* const pOwner) : m_iRemainReceiveBytes(0), m_pReceiveBuffer(nullptr),
						m_iLastReceivedPacketNumber(0), m_pOwner(pOwner),
						m_IOType(ioType), m_wsaOverlapped() {
						ZeroMemory(&m_wsaOverlapped, sizeof(WSAOVERLAPPED));
						ZeroMemory(&m_wsaBuffer, sizeof(WSABUF));
					}
				};


				class User_Server : public USER_SESSION::User {
				private:
					OVERLAPPED_EX m_acceptOverlapped;
					OVERLAPPED_EX m_disconnectOverlapped;
					OVERLAPPED_EX m_receiveOverlapped;
					OVERLAPPED_EX m_receiveFromOverlapped;
					OVERLAPPED_EX m_sendOverlapped;

				public:
					User_Server(NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE protocolType);

				public:
					virtual bool Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress);
					bool Initialize(const User_Server& server);

					bool RegisterIOCompletionPort(const HANDLE& hIOCP);

				public:
					bool Receive();
					bool ReceiveFrom();

					bool Send(char* const sSendData, const uint16_t iDataLength);
					bool SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, char* const sSendData, const uint16_t iDataLength);

					bool Send(const PACKET::PACKET_STRUCT& sendPacketStructure);
					bool SendTo(const PeerInfo& peerInformation, PACKET::PACKET_STRUCT& sendPacketStructure);

					bool SocketRecycle();

				};
			}
		}
	}
}