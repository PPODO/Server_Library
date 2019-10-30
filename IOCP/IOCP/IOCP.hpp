#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <Network/Session/ServerSession/ServerSession.h>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.h>

namespace NETWORK {
	namespace NETWORKMODEL {
		namespace IOCP {
			static const size_t MAX_CLIENT_COUNT = 500;

			typedef std::map<uint8_t, std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>> PACKETPROCESSORLIST;

			class CIOCP {
			private:
				WSADATA m_WinSockData;

			private:
				HANDLE m_hIOCP;

			private:
				int16_t m_bIsRunMainThread;
				std::vector<std::thread> m_WorkerThread;

			private:
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ProcessorListLock;
				PACKETPROCESSORLIST m_PacketProcessors;

			private:
				std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession> m_Listener;
				// TCP
				std::vector<std::unique_ptr<NETWORK::SESSION::SERVERSESSION::CServerSession>> m_Clients;
				// UDP
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ConnectionListLock;
				std::vector<NETWORK::SOCKET::UDPIP::PEERINFO> m_ConnectedPeers;

			private:
				FUNCTIONS::COMMAND::CCommand m_Command;

				// TODO º¸·ù
				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*> m_Queue;

			public:
				explicit CIOCP();
				explicit CIOCP(const PACKETPROCESSORLIST& ProcessorList);
				virtual ~CIOCP();

			public:
				bool Initialize(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				void Run();

			protected:
				virtual void OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOReceive(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
				virtual void OnIOReceiveFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes);

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
				void Destroy();

			private:
				void InitializeWinSock();
				bool InitializeHandles();
				bool InitializeSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				void CreateWorkerThread();

			private:
				void WorkerThread();

			private:
				void PacketForwardingLoop(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx);

			public:
				void InsertNewPacketProcessor(const uint8_t& Key, const std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>& Value) {
					FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ProcessorListLock);
					m_PacketProcessors.insert(std::make_pair(Key, Value));
				}

			};

		}
	}
}