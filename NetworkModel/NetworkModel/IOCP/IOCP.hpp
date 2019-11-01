#pragma once
#include <NetworkModel/NetworkModel/NetworkModel.h>

namespace NETWORKMODEL {
	namespace IOCP {
		static const size_t MAX_CLIENT_COUNT = 500;

		struct CONNECTION {
			struct TCPCONNECTION {
			public:
				NETWORK::SESSION::SERVERSESSION::CServerSession* const m_Session;
				FUNCTIONS::SOCKADDR::CSocketAddress m_Address;

			public:
				TCPCONNECTION() : m_Session(nullptr), m_Address() {};
				TCPCONNECTION(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) : m_Session(Session), m_Address() {};
				TCPCONNECTION(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, const FUNCTIONS::SOCKADDR::CSocketAddress& Address) : m_Session(Session), m_Address(Address) {};
				TCPCONNECTION(const TCPCONNECTION& lvalue) : m_Session(lvalue.m_Session), m_Address(lvalue.m_Address) {};
				TCPCONNECTION(const TCPCONNECTION&& rvalue) : m_Session(rvalue.m_Session), m_Address(rvalue.m_Address) {};

			public:
				bool operator==(const NETWORK::SESSION::SERVERSESSION::CServerSession* Session) const {
					if (m_Session == Session) { return true; } return false;
				}
				bool operator==(const FUNCTIONS::SOCKADDR::CSocketAddress& Address) const {
					if (m_Address == Address) { return true; } return false;
				}

			};
		public:
			TCPCONNECTION m_Client;
			NETWORK::SOCKET::UDPIP::PEERINFO m_Peer;

		public:
			CONNECTION() : m_Client(), m_Peer() {};
			CONNECTION(const TCPCONNECTION& Client) : m_Client(Client), m_Peer() {};
			CONNECTION(const NETWORK::SOCKET::UDPIP::PEERINFO& Peer) : m_Client(nullptr), m_Peer(Peer) {};
			CONNECTION(const TCPCONNECTION& Client, const NETWORK::SOCKET::UDPIP::PEERINFO& Peer) : m_Client(Client), m_Peer(Peer) {};
			CONNECTION(CONNECTION& lvalue) : m_Client(lvalue.m_Client), m_Peer(lvalue.m_Peer) {};
			CONNECTION(CONNECTION&& rvalue) noexcept : m_Client(rvalue.m_Client), m_Peer(rvalue.m_Peer) {};

		};

		class CIOCP : private DETAIL::CNetworkModel {
		private:
			HANDLE m_hIOCP;

		private:
			int16_t m_bIsRunMainThread;
			std::vector<std::thread> m_WorkerThread;

		private:
			std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession> m_Listener;

			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ClientListLock;
			std::vector<CONNECTION> m_Clients;

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
			CONNECTION* GetConnectionFromList(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ClientListLock);

				if (auto It = std::find_if(m_Clients.begin(), m_Clients.end(), [&Session](CONNECTION& Connection) { if (Connection.m_Client == Session) { return true; } return false; }); It != m_Clients.cend()) {
					return &(*It);
				}
				return nullptr;
			}
			CONNECTION* GetConnectionFromList(FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ClientListLock);

				if (auto It = std::find_if(m_Clients.begin(), m_Clients.end(), [&PeerAddress](CONNECTION& Connection) { if (Connection.m_Peer == PeerAddress) { return true; } return false; }); It != m_Clients.cend()) {
					return &(*It);
				}

				const auto& Iterator = std::find_if(m_Clients.begin(), m_Clients.end(), [&PeerAddress](const CONNECTION& Connection) -> bool { if (PeerAddress.IsSameAddress(Connection.m_Client.m_Address)) { return true; } return false; });
				if (Iterator == m_Clients.cend()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
					return &(m_Clients.emplace_back(CONNECTION::TCPCONNECTION(), NETWORK::SOCKET::UDPIP::PEERINFO(PeerAddress, 0)));
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