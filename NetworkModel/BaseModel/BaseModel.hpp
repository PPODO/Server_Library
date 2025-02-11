#pragma once
#include "../../Network/UserSession/Server/User_Server.hpp"
#include "../../Network/Packet/BasePacket.hpp"
#include <map>
#include <functional>

using namespace SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET;

namespace SERVER {
	namespace NETWORKMODEL {
		namespace BASEMODEL {
			typedef std::unordered_map<uint8_t, std::function<void(NETWORK::PACKET::PacketQueueData* const)>> PACKETPROCESSOR;

			class BaseNetworkModel {
				typedef FUNCTIONS::CIRCULARQUEUE::CircularQueue<NETWORK::PACKET::PacketQueueData*, 1000> PACKETQUEUE;
			private:
				const PACKETPROCESSOR& m_packetProcessorMap;
				const int m_iPacketProcessorLoopCount;

				WSADATA m_wsaData;

				EPROTOCOLTYPE m_protocolType;

				FUNCTIONS::CRITICALSECTION::CriticalSection m_packetQueueLock;
				std::unique_ptr<PACKETQUEUE> m_pPacketProcessQueue;
				std::unique_ptr<PACKETQUEUE> m_pPacketStorageQueue;

			protected:
				virtual void Destroy() = 0;

			public:
				BaseNetworkModel(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap);
				virtual ~BaseNetworkModel();

			public:
				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) = 0 {
					m_protocolType = protocolType;

					return true;
				}
				virtual void Run();

				void ReceiveDataProcessing(const EPROTOCOLTYPE protocolType, char* const sReceiveBuffer, uint16_t& iReceiveBytes, int16_t& iLastReceivePacketNumber, void* const pOwner);

			public:
				inline EPROTOCOLTYPE GetProtocolType() const { return m_protocolType; }

			};
		}
	}
}