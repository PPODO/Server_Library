#include "EventSelect.hpp"
#include "../../Functions/Log/Log.hpp"

using namespace SERVER::NETWORKMODEL::EVENTSELECT;
using namespace SERVER::FUNCTIONS::LOG;

EventSelect::EventSelect(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap) : BaseNetworkModel(iPacketProcessorLoopCount, packetProcessorMap), m_hStopHandle(WSA_INVALID_EVENT), m_iNextSendPacketNumber(0), m_iThreadRunState(1) {
	ZeroMemory(&m_hEventSelect, sizeof(WSAEVENT) * 2);
}

EventSelect::~EventSelect() {
}

bool EventSelect::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	BaseNetworkModel::Initialize(protocolType, serverAddress);

	m_serverAddress = serverAddress;

	bool optionVar = true;
	if (m_client = std::make_unique<EventSelectClient>(protocolType)) {
		setsockopt(m_client->GetTCPSocketHandle(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&optionVar), sizeof(optionVar));
		if (!m_client->Connect(serverAddress)) return false;

		return InitializeEventHandle();
	}
	return false;
}

bool EventSelect::InitializeEventHandle() {
	m_hStopHandle = WSACreateEvent();
	if (m_hStopHandle == WSA_INVALID_EVENT) {
		Log::WriteLog(FUNCTIONS::UTIL::MBToUni("Failed To Initialize Thread Stop Event!").c_str());
		return false;
	}

	WSAEVENT hTcpEvent = WSACreateEvent();
	if (hTcpEvent == WSA_INVALID_EVENT) {
		Log::WriteLog(FUNCTIONS::UTIL::MBToUni("Failed To Initialize TCP Event Select!").c_str());
		return false;
	}
	// 0 is tcp, 1 is udp
	m_hEventSelect[0] = hTcpEvent;
	WSAEventSelect(m_client->GetTCPSocketHandle(), hTcpEvent, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
	m_threadForTCPSelectProcessor = std::thread(&EventSelect::EventSelectProcessorForTCP, this, m_hEventSelect[0]);

	WSAEVENT hUdpEvent = WSACreateEvent();
	if (hUdpEvent == WSA_INVALID_EVENT) {
		Log::WriteLog(FUNCTIONS::UTIL::MBToUni("Failed To Initialize UDP Event Select!").c_str());
		return false;
	}
	m_hEventSelect[1] = hUdpEvent;
	WSAEventSelect(m_client->GetUDPSocketHandle(), hUdpEvent, FD_READ | FD_WRITE);
	m_threadForUDPSelectProcessor = std::thread(&EventSelect::EventSelectProcessorForUDP, this, m_hEventSelect[1]);

	return true;
}

void EventSelect::EventSelectProcessorForTCP(const WSAEVENT& hEventHandle) {
	HANDLE hEvents[2] = { m_hStopHandle, hEventHandle };
	char sReceiveBuffer[NETWORK::PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE] = { "\0" };
	WSANETWORKEVENTS networkEvent;
	uint16_t iRemainReceivedBytes = 0;
	int16_t iLastReceivedPacketNumber = 0;

	while (m_iThreadRunState) {
		DWORD iResult = WaitForMultipleObjects(2, hEvents, false, INFINITE);

		switch(iResult) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_client->GetTCPSocketHandle(), hEventHandle, &networkEvent);

			if (networkEvent.lNetworkEvents & FD_CONNECT) {
				Log::WriteLog(FUNCTIONS::UTIL::MBToUni("Connected To Server!").c_str());
			}
			else if (networkEvent.lNetworkEvents & FD_READ) {
				uint16_t iRecvBytes = 0;
				if (m_client->Receive(sReceiveBuffer + iRemainReceivedBytes, iRecvBytes)) {
					iRemainReceivedBytes += iRecvBytes;
					BaseNetworkModel::ReceiveDataProcessing(EPROTOCOLTYPE::EPT_TCP, sReceiveBuffer, iRemainReceivedBytes, iLastReceivedPacketNumber, this);
				}
			}
			break;
		}
	}
}

void EventSelect::EventSelectProcessorForUDP(const WSAEVENT& hEventHandle) {
	HANDLE hEvents[2] = { m_hStopHandle, hEventHandle };
	char sReceiveBuffer[NETWORK::PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE] = { "\0" };
	WSANETWORKEVENTS networkEvent;
	uint16_t iRemainReceivedBytes = 0;

	while (m_iThreadRunState) {
		DWORD iResult = WaitForMultipleObjects(2, hEvents, false, INFINITE);

		switch (iResult) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_client->GetUDPSocketHandle(), hEventHandle, &networkEvent);

			if (networkEvent.lNetworkEvents & FD_READ) {
				uint16_t iRecvBytes = 0;
				if (m_client->ReceiveFrom(sReceiveBuffer + iRemainReceivedBytes, iRecvBytes) &&
					SERVER::NETWORK::PROTOCOL::UTIL::UDP::CheckAck(m_client->GetUDPInstance(), m_serverAddress, sReceiveBuffer, iRemainReceivedBytes, m_iNextSendPacketNumber)) {
					iRemainReceivedBytes += iRecvBytes;
					BaseNetworkModel::ReceiveDataProcessing(EPROTOCOLTYPE::EPT_UDP, sReceiveBuffer, iRemainReceivedBytes, m_iNextSendPacketNumber, this);
				}
			}
			break;
		}
	}
}

void EventSelect::Run() {
	BaseNetworkModel::Run();
}

void EventSelect::Destroy() {
	InterlockedExchange16(&m_iThreadRunState, 0);

	for (auto& it : m_hEventSelect)
		SetEvent(m_hStopHandle);

	if (m_threadForTCPSelectProcessor.joinable())
		m_threadForTCPSelectProcessor.join();
	if (m_threadForUDPSelectProcessor.joinable())
		m_threadForUDPSelectProcessor.join();

	if (m_hStopHandle != WSA_INVALID_EVENT)
		WSACloseEvent(m_hStopHandle);

	if (m_hEventSelect[0] != WSA_INVALID_EVENT)
		WSACloseEvent(m_hEventSelect[0]);
	if (m_hEventSelect[1] != WSA_INVALID_EVENT)
		WSACloseEvent(m_hEventSelect[1]);
}