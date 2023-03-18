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


				};


				class User_Server : public USER_SESSION::User {
				private:
					OVERLAPPED_EX m_acceptOverlapped;
					OVERLAPPED_EX m_disconnectOverlapped;
					OVERLAPPED_EX m_receiveOverlapped;
					OVERLAPPED_EX m_receiveFromOverlapped;
					OVERLAPPED_EX m_sendOverlapped;

				public:
					virtual bool Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress);
					bool Initialize(const User_Server& client);

					bool RegisterIOCompletionPort(const HANDLE& hIOCP);


				};
			}
		}
	}
}