#include "NetworkModel.hpp"

using namespace FUNCTIONS::LOG;

NETWORKMODEL::DETAIL::CNetworkModel::CNetworkModel(const PACKETPROCESSORLIST& ProcessorList) : m_PacketProcessors(ProcessorList) {
	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) != 0) {
		assert(false);
	}
}

NETWORKMODEL::DETAIL::CNetworkModel::~CNetworkModel() {
	WSACleanup();
}

void NETWORKMODEL::DETAIL::CNetworkModel::PacketForwardingLoop(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber, void* const Owner) {
	using namespace NETWORK::PACKET;

	while (true) {
		PACKET_STRUCTURE PacketStructure;
		int16_t RemainBytes = ReceivedBytes;

		if (RemainBytes >= NETWORK::PACKET::DETAIL::PACKET_INFORMATION::GetSize()) {
			PacketStructure.m_PacketInformation = *reinterpret_cast<NETWORK::PACKET::DETAIL::PACKET_INFORMATION*>(ReceivedBuffer);
			RemainBytes -= NETWORK::PACKET::DETAIL::PACKET_INFORMATION::GetSize();
		}

		if (RemainBytes >= PacketStructure.m_PacketInformation.m_PacketSize) {
			uint16_t TotalBytes = (PacketStructure.m_PacketInformation.GetSize() + PacketStructure.m_PacketInformation.m_PacketSize);
			if (PacketStructure.m_PacketInformation.m_PacketNumber == LastReceivedPacketNumber) {
				if (ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
					InterlockedIncrement16(&LastReceivedPacketNumber);
				}
				CopyMemory(PacketStructure.m_PacketData, ReceivedBuffer + PacketStructure.m_PacketInformation.GetSize(), PacketStructure.m_PacketInformation.m_PacketSize);

				m_PacketQueue.Push(new FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData(Owner, PacketStructure));
			}
			MoveMemory(ReceivedBuffer, ReceivedBuffer + TotalBytes, TotalBytes);
			ReceivedBytes -= TotalBytes;
		}
		else {
			break;
		}

	}
}