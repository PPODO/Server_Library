#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <Network/Socket/UDP/UDPSocket.h>
#include <Network/Session/ServerSession/ServerSession.h>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.h>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			struct CPacketQueueData : public QUEUEDATA::DETAIL::BaseData<CPacketQueueData, 500> {
			public:
				/*
				EventSelect - CEventSelect
				IOCP - CONNECTION
				*/
				void* m_Owner;
				NETWORK::PACKET::PACKET_STRUCTURE m_PacketStructure;

			public:
				CPacketQueueData() : m_Owner(nullptr) { ZeroMemory(&m_PacketStructure, sizeof(NETWORK::PACKET::PACKET_STRUCTURE)); };
				CPacketQueueData(void* const Owner, const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) : m_Owner(Owner), m_PacketStructure(PacketStructure) {};

			};
		}
	}
}

namespace NETWORKMODEL {
	namespace DETAIL {
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
				bool operator==(const FUNCTIONS::SOCKADDR::CSocketAddress& const Address) const {
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

		typedef std::map<uint8_t, std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>> PACKETPROCESSORLIST;

		class CNetworkModel {
		private:
			WSADATA m_WinSockData;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ProcessorListLock;
			PACKETPROCESSORLIST m_PacketProcessors;

		private:
			FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*> m_PacketQueue;

		protected:
			void PacketForwardingLoop(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber, void* const Owner = nullptr);

		public:
			explicit CNetworkModel(const PACKETPROCESSORLIST& ProcessorList = {});
			virtual ~CNetworkModel() = 0;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) = 0;
			virtual void Run() = 0;

		protected:
			virtual void Destroy() = 0;

		protected:
			inline FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const GetPacketDataFromQueue() {
				if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* Data; m_PacketQueue.Pop(Data) && Data) {
					return Data;
				}
				return nullptr;
			}
			inline std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)> GetProcessorFromList(const uint8_t& Key) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ProcessorListLock);
				if (auto ProcessorIt = m_PacketProcessors.find(Key); ProcessorIt != m_PacketProcessors.cend()) {
					return ProcessorIt->second;
				}
				return nullptr;
			}

		};
	}
}