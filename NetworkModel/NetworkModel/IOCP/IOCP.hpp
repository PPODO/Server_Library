#pragma once
#include <NetworkModel/NetworkModel/BaseModel/BaseModel.hpp>
#include <vector>
#include <thread>

namespace SERVER {
	namespace NETWORKMODEL {
		namespace IOCP {
			struct CONNECTION {
			public:
				NETWORK::USER_SESSION::USER_SERVER::User_Server* m_pUser;

			public:
				CONNECTION() : m_pUser(nullptr) {};
				CONNECTION(NETWORK::USER_SESSION::USER_SERVER::User_Server* pUser) : m_pUser(pUser) {};

			};

			class IOCP : private BASEMODEL::BaseNetworkModel {
			private:
				HANDLE m_hIOCP;
				std::unique_ptr<NETWORK::USER_SESSION::USER_SERVER::User_Server> m_pServer;

				std::vector<std::thread> m_workerThreadList;

				FUNCTIONS::CRITICALSECTION::CriticalSection m_clientListLock;
				std::vector<CONNECTION*> m_clientList;

			public:
				IOCP(const BASEMODEL::PACKETPROCESSOR& packetProcessorMap, const int iPacketProcessorLoopCount);
				virtual ~IOCP() override;

			public:
				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override;
				virtual void Run() override;
				virtual void Destroy() override;

			private:
				void IOCPWorkerThread();

			};

		}
	}
}