#include "EventSelect.hpp"
#include <iostream>

using namespace FUNCTIONS::LOG;

NETWORKMODEL::EVENTSELECT::CEventSelect::CEventSelect(const int PacketProcessLoopCount, const DETAIL::PACKETPROCESSORLIST& ProcessorList) : DETAIL::CNetworkModel(ProcessorList), m_PacketProcessLoopCount(PacketProcessLoopCount), m_hStopEvent(INVALID_HANDLE_VALUE), m_hTCPSelectEvent(INVALID_HANDLE_VALUE), m_hUDPSelectEvent(INVALID_HANDLE_VALUE), m_ThreadRunState(1), m_NextSendPacketNumber(0) {
}

NETWORKMODEL::EVENTSELECT::CEventSelect::~CEventSelect() {
	Destroy();
}

bool NETWORKMODEL::EVENTSELECT::CEventSelect::Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) {
	m_ServerAddress = ServerAddress;

	if (ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
		m_TCPIPSocket = std::make_unique<NETWORK::SOCKET::TCPIP::CTCPIPSocket>();
		m_TCPIPSocket->Connect(m_ServerAddress);
	}
	if (ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
		m_UDPIPSocket = std::make_unique<NETWORK::SOCKET::UDPIP::CUDPIPSocket>();
	}

	if (!InitializeEvent()) {
		return false;
	}
	return true;
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::Run() {
	for (int i = 0; i < m_PacketProcessLoopCount; i++) {
		if (auto PacketAndProcessor(GetPacketDataAndProcessorOrNull()); PacketAndProcessor && PacketAndProcessor->m_Packet && PacketAndProcessor->m_Processor) {
			PacketAndProcessor->m_Processor(PacketAndProcessor->m_Packet.get());
		}
	}
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::Destroy() {
	InterlockedExchange16(&m_ThreadRunState, 0);

	for (auto& Iterator : m_EventSelectThread) {
		SetEvent(m_hStopEvent);
	}
	for (auto& Iterator : m_EventSelectThread) {
		if (Iterator.joinable()) {
			Iterator.join();
		}
	}
	m_EventSelectThread.clear();

	if (m_hStopEvent) {
		CloseHandle(m_hStopEvent);
	}

	if (m_hTCPSelectEvent) {
		WSACloseEvent(m_hTCPSelectEvent);
	}
	if (m_hUDPSelectEvent) {
		WSACloseEvent(m_hUDPSelectEvent);
	}

}

bool NETWORKMODEL::EVENTSELECT::CEventSelect::InitializeEvent() {
	if (!(m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
		CLog::WriteLog(L"Failed To Initialize Thread Stop Event!");
		return false;
	}

	if (HANDLE hTCPEventSelect = WSACreateEvent()) {
		if (!hTCPEventSelect) {
			CLog::WriteLog(L"Failed To Initialize TCP Event Select!");
			return false;
		}
		m_hTCPSelectEvent = hTCPEventSelect;
		WSAEventSelect(m_TCPIPSocket->GetSocket(), m_hTCPSelectEvent, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
		m_EventSelectThread.emplace_back(std::thread(&NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForTCP, this, m_hTCPSelectEvent));
	}

	if (HANDLE hUDPEventSelect = WSACreateEvent()) {
		if (!hUDPEventSelect) {
			CLog::WriteLog(L"Failed To Initialize UDP Event Select!");
			return false;
		}
		m_hUDPSelectEvent = hUDPEventSelect;
		WSAEventSelect(m_UDPIPSocket->GetSocket(), m_hUDPSelectEvent, FD_READ | FD_WRITE);
		m_EventSelectThread.emplace_back(std::thread(&NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForUDP, this, m_hUDPSelectEvent));
	}

	return true;
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForTCP(const HANDLE& SelectEventHandle) {
	HANDLE hEvents[2] = { m_hStopEvent, SelectEventHandle };
	char ReceivedBuffer[NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE] = { "\0" };
	int16_t RemainReceivedBytes = 0;	
	int16_t LastReceivedPacketNumber = 0;

	while (m_ThreadRunState) {
		DWORD Result = WaitForMultipleObjects(2, hEvents, false, INFINITE);
		switch (WSANETWORKEVENTS NetworkEvent; Result) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_TCPIPSocket->GetSocket(), SelectEventHandle, &NetworkEvent);

			if (NetworkEvent.lNetworkEvents & FD_CONNECT) {
				FUNCTIONS::LOG::CLog::WriteLog(L"Connect To Server!");
			}
			else if (NetworkEvent.lNetworkEvents & FD_READ) {
				uint16_t RecvBytes = 0;
				if (m_TCPIPSocket->Read(ReceivedBuffer + RemainReceivedBytes, RecvBytes)) {
					RemainReceivedBytes += RecvBytes;
					PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, ReceivedBuffer, RemainReceivedBytes, LastReceivedPacketNumber, this);
				}
			}
			break;
		}
	}
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForUDP(const HANDLE& SelectEventHandle) {
	HANDLE hEvents[2] = { m_hStopEvent, SelectEventHandle };
	char ReceivedBuffer[NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE] = { "\0" };
	int16_t RemainReceivedBytes = 0;

	while (m_ThreadRunState) {
		DWORD Result = WaitForMultipleObjects(2, hEvents, false, INFINITE);
		switch (WSANETWORKEVENTS NetworkEvent; Result) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_UDPIPSocket->GetSocket(), SelectEventHandle, &NetworkEvent);

			if (NetworkEvent.lNetworkEvents & FD_READ) {
				uint16_t RecvBytes = 0;
				if (m_UDPIPSocket->ReadFrom(ReceivedBuffer + RemainReceivedBytes, RecvBytes) && NETWORK::UTIL::UDPIP::CheckAck(m_UDPIPSocket.get(), m_ServerAddress, ReceivedBuffer, RecvBytes, m_NextSendPacketNumber)) {
					RemainReceivedBytes += RecvBytes;
					PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP, ReceivedBuffer, RemainReceivedBytes, m_NextSendPacketNumber, this);
				}
			}
			break;
		}
	}
}

bool NETWORK::UTIL::UDPIP::CheckAck(NETWORK::SOCKET::UDPIP::CUDPIPSocket* const UDPSocket, const FUNCTIONS::SOCKADDR::CSocketAddress& RemoteAddress, char* const ReceviedBuffer, uint16_t& ReceivedBytes, int16_t& UpdatedPacketNumber) {
	if (ReceviedBuffer) {
		const int32_t ReadedValue = *reinterpret_cast<const int32_t*>(ReceviedBuffer);
		const int16_t AckValue = static_cast<int16_t>(ReadedValue);
		const int16_t PacketNumber = static_cast<int16_t>((ReadedValue >> 16));

		// TODO PacketNumber가 이전에 받은 Number보다 작을때 따로 처리해주는 로직이 필요
		if (AckValue == 9999) {
			InterlockedExchange16(&UpdatedPacketNumber, PacketNumber);
			ReceivedBytes -= sizeof(ReadedValue);
			return UDPSocket->SendCompletion();
		}
		else if (AckValue == 0) {
			int16_t AckNumber = 9999;
			int16_t PacketNumber = UpdatedPacketNumber + 1;
			int32_t Result = ((PacketNumber << 16) | AckNumber);
			ReceivedBytes -= sizeof(ReadedValue);
			MoveMemory(ReceviedBuffer, ReceviedBuffer + sizeof(ReadedValue), ReceivedBytes);
			return UDPSocket->WriteTo(RemoteAddress, reinterpret_cast<const char* const>(&Result), sizeof(int32_t));
		}
	}
	return false;
}