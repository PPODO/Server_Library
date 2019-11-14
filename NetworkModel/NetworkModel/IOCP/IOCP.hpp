#pragma once
#include <NetworkModel/NetworkModel/NetworkModel.hpp>
#include <array>

namespace NETWORKMODEL {
	namespace IOCP {
		static const size_t MAX_CLIENT_COUNT = 500;

		namespace DETAIL {
			struct CONNECTION {
			public:
				NETWORK::SESSION::SERVERSESSION::CServerSession* m_Session;
				NETWORK::SOCKET::UDPIP::PEERINFO m_PeerInformation;

			public:
				explicit CONNECTION() : m_Session(nullptr), m_PeerInformation() {};
				explicit CONNECTION(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, const NETWORK::SOCKET::UDPIP::PEERINFO& PeerInformation = NETWORK::SOCKET::UDPIP::PEERINFO()) : m_Session(Session), m_PeerInformation(PeerInformation) {};

			};
		}

		class CIOCP : private NETWORKMODEL::DETAIL::CNetworkModel {
		private:
			HANDLE m_hIOCP;

		private:
			int16_t m_bIsRunMainThread;
			std::vector<std::thread> m_WorkerThread;

		private:
			NETWORK::SESSION::SERVERSESSION::CServerSession* m_Listener;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ClientListLock;
			std::vector<DETAIL::CONNECTION*> m_Clients;

		private:
			FUNCTIONS::COMMAND::CCommand m_Command;

		public:
			explicit CIOCP(const NETWORKMODEL::DETAIL::PACKETPROCESSORLIST& ProcessorList);
			virtual ~CIOCP() override = 0;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) override;
			virtual void Run() override;

		protected:
			virtual void Destroy() override;
			virtual DETAIL::CONNECTION* OnIOAccept(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const AcceptExOverlappedEx);
			virtual void OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual void OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
			virtual void OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes);

		private:
			DETAIL::CONNECTION* GetConnectionFromList(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
				if (Session) {
					auto Pred = [&Session](DETAIL::CONNECTION* const Connection) {
						if (Connection->m_Session == Session) {
							return true;
						}
						return false;
					};
					if (auto Iterator = std::find_if(m_Clients.begin(), m_Clients.end(), Pred); Iterator != m_Clients.cend()) {
						return *Iterator;
					}
				}
				return nullptr;
			}
			DETAIL::CONNECTION* GetConnectionFromList(FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
				using namespace NETWORK::UTIL::BASESOCKET;

				auto Pred = [&PeerAddress](DETAIL::CONNECTION* const Connection) {
					if (PeerAddress.IsSameAddress(Connection->m_PeerInformation.m_RemoteAddress)) {
						return true;
					}
					return false;
				};
				if (auto Iterator = std::find_if(m_Clients.begin(), m_Clients.end(), Pred); Iterator != m_Clients.cend()) {
					return *Iterator;
				}
				else if (GetProtocolType() & EPROTOCOLTYPE::EPT_UDP) {
					DETAIL::CONNECTION* NewConnection = nullptr;
					try {
						NewConnection = new DETAIL::CONNECTION(m_Listener, NETWORK::SOCKET::UDPIP::PEERINFO(PeerAddress, 0));
					}
					catch (const std::exception& Exception) {
						FUNCTIONS::LOG::CLog::WriteLog(L"%S", Exception.what());
						return nullptr;
					}
					return m_Clients.emplace_back(NewConnection);
				}
				return nullptr;
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