#pragma once
#include <NetworkModel/NetworkModel/NetworkModel.hpp>
#include <array>

namespace NETWORKMODEL {
	namespace IOCP {
		static const size_t MAX_CLIENT_COUNT = 500;

		namespace DETAIL {
			struct CONNECTION : public FUNCTIONS::MEMORYMANAGER::CMemoryManager<CONNECTION, MAX_CLIENT_COUNT> {
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
			std::vector<std::thread> m_WorkerThread;

		private:
			NETWORK::SESSION::SERVERSESSION::CServerSession* m_Listener;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ClientListLock;
			std::vector<DETAIL::CONNECTION*> m_Clients;

		public:
			explicit CIOCP(const NETWORKMODEL::DETAIL::PACKETPROCESSORLIST& ProcessorList, const int PacketProcessLoopCount = 1);
			virtual ~CIOCP() override;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) override;
			virtual void Run() override;

		protected:
			virtual void Destroy() = 0;

			/*
				Can Return Null
			*/
			virtual DETAIL::CONNECTION* OnIOAccept(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const AcceptExOverlappedEx);
			virtual DETAIL::CONNECTION* OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
			virtual DETAIL::CONNECTION* OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, const uint16_t& SendBytes);
			virtual DETAIL::CONNECTION* OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
			virtual DETAIL::CONNECTION* OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes);
			virtual DETAIL::CONNECTION* OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);

		private:
			DETAIL::CONNECTION* GetConnectionFromListOrNull(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
				auto Pred = [&Session](DETAIL::CONNECTION* const Connection) {
					if (Connection->m_Session == Session) {
						return true;
					}
					return false;
				};
				if (auto Iterator = std::find_if(m_Clients.begin(), m_Clients.end(), Pred); Iterator != m_Clients.cend()) {
					return *Iterator;
				}
				return nullptr;
			}
			DETAIL::CONNECTION* GetConnectionFromListOrNull(FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
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
					return m_Clients.emplace_back(new DETAIL::CONNECTION(m_Listener, NETWORK::SOCKET::UDPIP::PEERINFO(PeerAddress, 0)));
				}
				return nullptr;
			}

		private:
			void WorkerThread();

		};
	}
}