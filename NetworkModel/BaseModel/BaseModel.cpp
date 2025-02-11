#include "BaseModel.hpp"
#include <cassert>
#include "../../Functions/Log/Log.hpp"

using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::FUNCTIONS::LOG;

BaseNetworkModel::BaseNetworkModel(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap) : m_iPacketProcessorLoopCount(iPacketProcessorLoopCount), m_packetProcessorMap(packetProcessorMap) {
	if (WSAStartup(WINSOCK_VERSION, &m_wsaData) != 0)
		assert(false);

	try {
		m_pPacketProcessQueue = std::make_unique<PACKETQUEUE>();
		m_pPacketStorageQueue = std::make_unique<PACKETQUEUE>();
	}
	catch (std::bad_alloc& exception) {
		Log::WriteLog(L"Bad Alloc! - %s", exception.what());
		assert(false);
	}
}

BaseNetworkModel::~BaseNetworkModel() {
	WSACleanup();
}

void BaseNetworkModel::Run() {
	for (int i = 0; i < m_iPacketProcessorLoopCount; i++) {
		NETWORK::PACKET::PacketQueueData* pPacketData = nullptr;
		if (m_pPacketProcessQueue->Pop(pPacketData)) {
			auto processor = m_packetProcessorMap.find(pPacketData->m_packetData->m_packetInfo.m_iPacketType);
			if (processor != m_packetProcessorMap.cend())
				processor->second(pPacketData);

			delete pPacketData;
		}
		else if (m_pPacketProcessQueue->IsEmpty() && !m_pPacketStorageQueue->IsEmpty()) {
			FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_packetQueueLock);

			m_pPacketProcessQueue.swap(m_pPacketStorageQueue);
			m_pPacketProcessQueue->EnableCriticalSection(false);
			m_pPacketStorageQueue->EnableCriticalSection(true);

			// 패킷 프로세스 큐는 메인 스레드에서만 접근함. 크리티컬 섹션을 사용할 필요가 없음.
			// 크리티컬 섹션은 상당한 자원을 소모한다. 필요없으면 안 쓰는 게 좋음 ㅇㅡㅇ
		}
	}
}

void BaseNetworkModel::ReceiveDataProcessing(const EPROTOCOLTYPE protocolType, char* const sReceiveBuffer, uint16_t& iReceiveBytes, int16_t& iLastReceivePacketNumber, void* const pOwner) {
	using namespace NETWORK::PACKET;

	const size_t PACKET_INFO_STRUCT_SIZE = PACKET_INFORMATION::GetStructSize();

	while (true) {
		PACKET_INFORMATION packetInfo;

		// 패킷 헤더만큼 수신 했는지 확인 후, 수신 했다면 헤더 가져옴
		if (iReceiveBytes > PACKET_INFO_STRUCT_SIZE) {
			packetInfo = *reinterpret_cast<PACKET_INFORMATION*>(sReceiveBuffer);

			if ((iReceiveBytes - PACKET_INFO_STRUCT_SIZE) >= packetInfo.m_iPacketDataSize) {
				uint16_t iPacketTotalBytes = PACKET_INFO_STRUCT_SIZE + packetInfo.m_iPacketDataSize;
				if (packetInfo.m_iPacketNumber == iLastReceivePacketNumber) { // for udp, tcp has always 0
					if (protocolType & EPROTOCOLTYPE::EPT_UDP)
						InterlockedIncrement16(&iLastReceivePacketNumber);
					auto newPacketStructure = new PACKET_STRUCT(packetInfo);
					CopyMemory(newPacketStructure->m_sPacketData, sReceiveBuffer + PACKET_INFO_STRUCT_SIZE, newPacketStructure->m_packetInfo.m_iPacketDataSize);

					FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_packetQueueLock);
					m_pPacketStorageQueue->Push(new PacketQueueData(pOwner, newPacketStructure));
				}
				iReceiveBytes -= iPacketTotalBytes;
				MoveMemory(sReceiveBuffer, sReceiveBuffer + iPacketTotalBytes, iReceiveBytes);
			}
			else
				break;
		}
		else
			break;
	}
}