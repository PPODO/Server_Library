#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <Network/Socket/UDP/UDPSocket.h>
#include <Network/Session/ServerSession/ServerSession.h>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.h>

namespace NETWORKMODEL {
	namespace DETAIL {
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