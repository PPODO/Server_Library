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
			// TCP
			std::vector<std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession>> m_Clients;
			// UDP
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ConnectionListLock;
			std::vector<NETWORK::SOCKET::UDPIP::PEERINFO> m_ConnectedPeers;

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
			virtual void OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
			virtual void OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes);

		private:
			void UpdatePeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress, const uint16_t& UpdatedPacketNumber) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

				if (const auto& Iterator = std::find_if(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), [&PeerAddress](const NETWORK::SOCKET::UDPIP::PEERINFO& Address) -> bool { if (PeerAddress == Address.m_RemoteAddress) { return true; } return false; }); Iterator != m_ConnectedPeers.end()) {
					Iterator->m_LastPacketNumber = UpdatedPacketNumber;
				}
				else {
					FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
					m_ConnectedPeers.emplace_back(PeerAddress, UpdatedPacketNumber);
				}
			}
			NETWORK::SOCKET::UDPIP::PEERINFO GetPeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

				if (const auto& Iterator = std::find_if(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), [&PeerAddress](const NETWORK::SOCKET::UDPIP::PEERINFO& Address) -> bool { if (PeerAddress == Address.m_RemoteAddress) { return true; } return false; }); Iterator != m_ConnectedPeers.end()) {
					return *Iterator;
				}
				else {
					FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
					return m_ConnectedPeers.emplace_back(PeerAddress, 0);
				}
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