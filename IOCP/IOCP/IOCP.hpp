#pragma once
#include <map>
#include <vector>
#include <thread>
#include <Network/Session/ServerSession/ServerSession.h>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.h>

namespace NETWORK {
	namespace NETWORKMODEL {
		namespace IOCP {
			static const size_t MAX_CLIENT_COUNT = 500;

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
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_PeerLocking;
				std::map<int, int> m_Peers;

			private:
				std::vector<std::thread> m_WorkerThread;

			private:
				FUNCTIONS::COMMAND::CCommand m_Command;

				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*> m_Queue;

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
				virtual void OnIOReceive(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);
				virtual void OnIOWriteTo(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session);
				virtual void OnIOReceiveFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes);

			private:
				void Destroy();

			private:
				bool InitializeHandles();
				bool InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				void CreateWorkerThread();

			private:
				void WorkerThread();

			private:
				void PacketForwardingLoop(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx);

			};

		}
	}
}