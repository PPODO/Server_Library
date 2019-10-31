#include "NetworkModel.h"

using namespace FUNCTIONS::LOG;

NETWORKMODEL::DETAIL::CNetworkModel::CNetworkModel(const PACKETPROCESSORLIST& ProcessorList) : m_PacketProcessors(ProcessorList) {
	try {
		if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) != 0) {
			throw FUNCTIONS::EXCEPTION::bad_wsastart();
		}
	}
	catch (std::exception & Exception) {
		CLog::WriteLog(L"%S", Exception.what());
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

				FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* QueueData = new FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData(Owner, PacketStructure);
				m_PacketQueue.Push(QueueData);
			}
			MoveMemory(ReceivedBuffer, ReceivedBuffer + TotalBytes, TotalBytes);
			ReceivedBytes -= TotalBytes;
		}
		else {
			break;
		}

	}
}