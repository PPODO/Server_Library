#pragma once
#include "../User.hpp"
#include <memory>

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			namespace USER_SERVER {
				enum class EIOTYPE : uint8_t {
					EIT_NONE,
					EIT_DISCONNECT,
					EIT_ACCEPT,
					EIT_READ,
					EIT_WRITE,
					EIT_READFROM,
					EIT_SENDTO
				};

				// Only Server Session :)
				struct OVERLAPPED_EX {
					// overlapped inst
					WSAOVERLAPPED m_wsaOverlapped;
					// ���� ����
					WSABUF m_wsaBuffer;
					// ���� ����Ʈ ��. ������ ���� �����͸� ���� ������ �ʰ� �ϱ� ����.
					int16_t m_iRemainReceiveBytes;
					// ������ ���� �޽��� ��ġ ���� ����
					char* m_sSocketMessage;

					// UDP������ ���
					FUNCTIONS::SOCKETADDRESS::SocketAddress m_remoteAddress;
					int16_t m_iLastReceivedPacketNumber;

					EIOTYPE m_IOType;
					void* m_pOwner;

				public:
					OVERLAPPED_EX() : m_iRemainReceiveBytes(0), m_sSocketMessage(nullptr), 
									  m_iLastReceivedPacketNumber(0), m_pOwner(nullptr),
									  m_IOType(EIOTYPE::EIT_NONE) {
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


				};
			}
		}
	}
}