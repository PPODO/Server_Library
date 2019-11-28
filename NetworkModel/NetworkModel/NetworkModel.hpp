#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <Network/Socket/UDP/UDPSocket.hpp>
#include <Network/Session/ServerSession/ServerSession.hpp>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.hpp>

namespace NETWORKMODEL {
	namespace DETAIL {
		typedef std::map<uint8_t, std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>> PACKETPROCESSORLIST;
		
		class CNetworkModel {
		private:
			WSADATA m_WinSockData;

		private:
			NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

		private:
			const int m_PacketProcessLoopCount;

		private:
			const PACKETPROCESSORLIST& m_PacketProcessors;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_SyncForQueueExchange;
			std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*>> m_ProcessQueue;
			std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*>> m_PacketStorage;

		protected:
			void PacketForwardingLoop(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber, void* const Owner = nullptr);

		public:
			explicit CNetworkModel(const int PacketProcessLoopCount = 1, const PACKETPROCESSORLIST& ProcessorList = {});
			virtual ~CNetworkModel() = 0;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) = 0 {
				m_ProtocolType = ProtocolType;

				return true;
			}
			virtual void Run();

		protected:
			virtual void Destroy() = 0;

		protected:
			inline NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE GetProtocolType() const { return m_ProtocolType; };

		};
	}
}