#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <type_traits>
#include <utility>
#include <Network/Socket/UDP/UDPSocket.hpp>
#include <Network/Session/ServerSession/ServerSession.hpp>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/Command/Command.hpp>

namespace NETWORKMODEL {
	namespace DETAIL {
		typedef std::map<uint8_t, std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>> PACKETPROCESSORLIST;
		
		struct PACKETPROCESSTYPE : public FUNCTIONS::MEMORYMANAGER::CMemoryManager<PACKETPROCESSTYPE, 100> {
		public:
			std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData> m_Packet;
			const std::function<void(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const)>& m_Processor;

		public:
			PACKETPROCESSTYPE(decltype(m_Packet) Packet, const decltype(m_Processor) Processor) : m_Packet(std::move(Packet)), m_Processor(Processor) {};

		};

		class CNetworkModel {
		private:
			WSADATA m_WinSockData;

		private:
			NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ProcessorListLock;
			const PACKETPROCESSORLIST& m_PacketProcessors;

		private:
			FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*> m_PacketQueue;

		protected:
			void PacketForwardingLoop(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber, void* const Owner = nullptr);

		public:
			explicit CNetworkModel(const PACKETPROCESSORLIST& ProcessorList = {});
			virtual ~CNetworkModel() = 0;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) = 0 {
				m_ProtocolType = ProtocolType;

				return true;
			}
			virtual void Run() = 0;

		protected:
			virtual void Destroy() = 0;

		protected:
			inline std::unique_ptr<PACKETPROCESSTYPE> GetPacketDataAndProcessorOrNull() {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_ProcessorListLock);

				if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* Data = nullptr; m_PacketQueue.Pop(Data) && Data) {
					if (auto Iterator = m_PacketProcessors.find(Data->m_PacketStructure.m_PacketInformation.m_PacketType); Iterator != m_PacketProcessors.cend()) {
						return std::unique_ptr<DETAIL::PACKETPROCESSTYPE>(new DETAIL::PACKETPROCESSTYPE(std::unique_ptr<std::remove_pointer<decltype(Data)>::type>(Data), Iterator->second));
					}
					delete Data;
				}
				return nullptr;
			}
			inline NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE GetProtocolType() const { return m_ProtocolType; };

		};
	}
}