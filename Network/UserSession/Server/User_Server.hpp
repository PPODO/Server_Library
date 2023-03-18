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
					// 수신 버퍼
					WSABUF m_wsaBuffer;
					// 남은 바이트 수. 이전의 읽은 데이터를 덮어 씌우지 않게 하기 위함.
					int16_t m_iRemainReceiveBytes;
					// 이전의 읽은 메시지 위치 저장 변수
					char* m_sSocketMessage;

					// UDP에서만 사용
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