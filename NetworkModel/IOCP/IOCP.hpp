#pragma once
#include "../BaseModel/BaseModel.hpp"
#include <vector>
#include <thread>

using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;

namespace SERVER {
	namespace NETWORKMODEL {
		namespace IOCP {
			struct CONNECTION {
			public:
				User_Server* m_pUser;
				NETWORK::PROTOCOL::UDP::PeerInfo m_peerInformation;

			public:
				CONNECTION() : m_pUser(nullptr), m_peerInformation() {};
				CONNECTION(User_Server* pUser) : m_pUser(pUser), m_peerInformation() {};
				CONNECTION(User_Server* pUser, const NETWORK::PROTOCOL::UDP::PeerInfo& peerInfo) : m_pUser(pUser), m_peerInformation(peerInfo) {};

			};

			class IOCP : private BASEMODEL::BaseNetworkModel {
			private:
				HANDLE m_hIOCP;
				std::unique_ptr<User_Server> m_pServer;

				std::vector<std::thread> m_workerThreadList;
				size_t m_iWorkerThreadCount;

				FUNCTIONS::CRITICALSECTION::CriticalSection m_clientListLock;
				std::vector<CONNECTION*> m_clientList;

				bool bEnableUDPAckCheck;

			protected:
				User_Server* const GetServerInstance() const { return m_pServer.get(); }

			public:
				IOCP(const BASEMODEL::PACKETPROCESSOR& packetProcessorMap, const size_t iWorkerThreadCount = 0);
				virtual ~IOCP() override;

			public:
				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override;
				virtual void Run() override;
				virtual void Destroy() override;

				virtual CONNECTION* OnIOAccept(OVERLAPPED_EX* const pAcceptOverlapped);
				virtual CONNECTION* OnIODisconnect(User_Server* const pClient);
				virtual CONNECTION* OnIOWrite(User_Server* const pClient, const uint16_t iWriteBytes);
				virtual CONNECTION* OnIOReceive(OVERLAPPED_EX* const pReceiveOverlapped, const uint16_t iRecvBytes);
				virtual CONNECTION* OnIOReceiveFrom(OVERLAPPED_EX* const pReceiveFromOverlapped, const uint16_t iRecvBytes);
				virtual CONNECTION* OnIOTryDisconnect(User_Server* const pClient);

				__forceinline void EnableAckCheck(bool bValue) { bEnableUDPAckCheck = bValue; }

			private:
				void IOCPWorkerThread();

				CONNECTION* GetConnectionFromList(User_Server* const pUser);
				CONNECTION* GetConnectionFromList(FUNCTIONS::SOCKETADDRESS::SocketAddress& peerAddress);

			};

		}
	}
}