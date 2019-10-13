#pragma once
#include <vector>
#include <thread>
#include <Network/Session/ServerSession/ServerSession.h>

namespace NETWORK {
	namespace NETWORKMODEL {
		namespace IOCP {
			static const size_t MAX_CLIENT_COUNT = 256;

			class CIOCP {
			private:
				const UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				WSADATA m_WinSockData;

			private:
				HANDLE m_hIOCP;

			private:
				int16_t m_bIsRunMainThread;

			private:
				std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession> m_Listener;
				std::vector<std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession>> m_Clients;

			private:
				std::vector<std::thread> m_WorkerThread;

			public:
				explicit CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CIOCP();

			public:
				bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				void Run();

			protected:
				virtual void OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOReceive(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, DWORD ReceiveBytes);

			private:
				bool InitializeHandles();
				bool InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				void CreateWorkerThread();

			private:
				void WorkerThread();

			};

		}
	}
}