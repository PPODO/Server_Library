#include "PacketSession.h"
#include <Network/Session/NetworkSession/NetworkSession.h>

using namespace NETWORK::SESSION::PACKETSESSION;

CPacketSession::CPacketSession() {
}

CPacketSession::~CPacketSession() {
}

// if the casting failure, this function throws std::bad_cast!
const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const NETWORK::SESSION::PACKETSESSION::CPacketSession::ReceivedDataDeSerialization(char* const DataBuffer, uint16_t& CurrentReceivedBytes) {
	using namespace NETWORK::PACKET;

	while (1) {
		try {
			if (m_LastPacketReceived.m_PacketInformation.m_PacketSize == 0) {
				// ���� ���� ����Ʈ�� ��Ŷ ���� ����ü ������� ū�� �˻縦 �մϴ�.

				if (UpdateLastReceivedPacketInformation(DataBuffer, CurrentReceivedBytes)) {
					ClearProcessedData(DataBuffer, DETAIL::PACKET_INFORMATION::GetSize(), DETAIL::PACKET_INFORMATION::GetSize(), CurrentReceivedBytes, DETAIL::PACKET_INFORMATION::GetSize());
				}
			}

			// ��Ŷ ���� ����ü�� ��Ŷ ũ�Ⱑ 0�� �ƴ϶�� ������ �о�԰�, �����Ͱ� �Ѿ�� ���̶�� ���̱⿡ �׿� ���� ó���� ������.
			if (CurrentReceivedBytes >= m_LastPacketReceived.m_PacketInformation.m_PacketSize) {
				ZeroMemory(&m_LastPacketReceived, sizeof(PACKET_STRUCTURE));
				ClearProcessedData(DataBuffer, m_LastPacketReceived.m_PacketInformation.m_PacketSize, m_LastPacketReceived.m_PacketInformation.m_PacketSize, CurrentReceivedBytes, m_LastPacketReceived.m_PacketInformation.m_PacketSize);
			}
		}
		catch (const std::bad_cast& Exception) {
			FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
			return nullptr;
		}
	}
	return nullptr;
}