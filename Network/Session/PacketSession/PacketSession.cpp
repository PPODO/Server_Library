#include "PacketSession.h"
#include <Network/Session/NetworkSession/NetworkSession.h>

using namespace NETWORK::SESSION::PACKETSESSION;

CPacketSession::CPacketSession() {
}

CPacketSession::~CPacketSession() {
}

const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const NETWORK::SESSION::PACKETSESSION::CPacketSession::ReceivedDataDeSerialization(char* const DataBuffer, uint16_t& CurrentReceivedBytes) {
	using namespace NETWORK::PACKET;

	while (true) {
		try {
			if (m_LastPacketReceived.m_PacketInformation.m_PacketSize == 0) {
				// 현재 읽은 바이트가 패킷 정보 구조체 사이즈보다 큰지 검사를 합니다.
				if (UpdateLastReceivedPacketInformation(DataBuffer, CurrentReceivedBytes)) {
					ClearProcessedData(DataBuffer, DETAIL::PACKET_INFORMATION::GetSize(), DETAIL::PACKET_INFORMATION::GetSize(), CurrentReceivedBytes, DETAIL::PACKET_INFORMATION::GetSize());
				}
				else {
					break;
				}
			}

			// 패킷 정보 구조체의 패킷 크기가 0이 아니라면 정보를 읽어왔고, 데이터가 넘어올 것이라는 뜻이기에 그에 따른 처리를 해주자.
			if (Deserialization(DataBuffer, CurrentReceivedBytes)) {
				ZeroMemory(&m_LastPacketReceived, sizeof(PACKET_STRUCTURE));
				ClearProcessedData(DataBuffer, m_LastPacketReceived.m_PacketInformation.m_PacketSize, m_LastPacketReceived.m_PacketInformation.m_PacketSize, CurrentReceivedBytes, m_LastPacketReceived.m_PacketInformation.m_PacketSize);
			}
			else {
				break;
			}
		}
		catch (const std::bad_cast& Exception) {
			FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
			return nullptr;
		}
	}
	return nullptr;
}