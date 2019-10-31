#pragma once
#include <NetworkModel/NetworkModel/NetworkModel.h>

namespace NETWORKMODEL {
	namespace IOCP {
		static const size_t MAX_CLIENT_COUNT = 500;

		class CIOCP : private DETAIL::CNetworkModel {
		private:
			HANDLE m_hIOCP;

		private:
			int16_t m_bIsRunMainThread;
			std::vector<std::thread> m_WorkerThread;

		private:
			std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession> m_Listener;

			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ClientListLock;
			std::vector<DETAIL::CONNECTION> m_Clients;

		private:
			FUNCTIONS::COMMAND::CCommand m_Command;

		public:
			explicit CIOCP(const DETAIL::PACKETPROCESSORLIST& ProcessorList);
			virtual ~CIOCP() override;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) override;
			virtual void Run() override;

		protected:
			virtual void Destroy() override;
			virtual void OnIOAccept(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const AcceptExOverlappedEx);
			virtual void OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
			virtual void OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes);

		private:
			DETAIL::CONNECTION* GetConnectionFromList(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ClientListLock);

				if (auto It = std::find_if(m_Clients.begin(), m_Clients.end(), [&Session](DETAIL::CONNECTION& Connection) { if (Connection.m_Client == Session) { return true; } return false; }); It != m_Clients.cend()) {
					return &(*It);
				}
				return nullptr;
			}
			DETAIL::CONNECTION* GetConnectionFromList(FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ClientListLock);

				if (auto It = std::find_if(m_Clients.begin(), m_Clients.end(), [&PeerAddress](DETAIL::CONNECTION& Connection) { if (Connection.m_Peer == PeerAddress) { return true; } return false; }); It != m_Clients.cend()) {
					return &(*It);
				}

				const auto& Iterator = std::find_if(m_Clients.begin(), m_Clients.end(), [&PeerAddress](const DETAIL::CONNECTION& Connection) -> bool { if (PeerAddress.IsSameAddress(Connection.m_Client.m_Address)) { return true; } return false; });
				if (Iterator == m_Clients.cend()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
					return &(m_Clients.emplace_back(DETAIL::CONNECTION::TCPCONNECTION(), NETWORK::SOCKET::UDPIP::PEERINFO(PeerAddress, 0)));
				}
				else {
					Iterator->m_Peer = NETWORK::SOCKET::UDPIP::PEERINFO(PeerAddress, 0);
				}
				return &(*Iterator);
			}

		private:
			bool InitializeSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
			inline bool InitializeHandles() {
				if (!(m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP Handle : Failed To Initialization IOCP Handle");
					return false;
				}
				return true;
			}
			inline void CreateWorkerThread() {
				const size_t WorkerThreadCount = std::thread::hardware_concurrency() * 2;
				for (size_t i = 0; i < WorkerThreadCount; i++) {
					m_WorkerThread.push_back(std::thread(&NETWORKMODEL::IOCP::CIOCP::WorkerThread, this));
				}

				FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP Session : Worker Thread Creation Succeeded! - %d", WorkerThreadCount);
			}

		private:
			void WorkerThread();

		};

	}
}