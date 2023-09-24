#include "UDPSocket.hpp"
#include "../UserSession/Server/User_Server.hpp"
#include <chrono>

using namespace SERVER::NETWORK::PROTOCOL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UDP::RELIABLE;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET;
using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;
using namespace SERVER::NETWORK::PACKET;

using namespace SERVER::FUNCTIONS::LOG;

ReliableUDP::ReliableUDP(UDPIPSocket* const pUDPSocket) : m_pUDPSocket(pUDPSocket), m_iThreadRunState(0), m_iSendCompletedPacketNumber(0) {
	InterlockedExchange16(&m_iThreadRunState, 1);
	m_reliableThread = std::thread(&ReliableUDP::ReliableProcessor, this);
	m_cacheReliableDataThread = std::thread(&ReliableUDP::CacheReliableDataProcessor, this);
}

ReliableUDP::~ReliableUDP() {
	InterlockedExchange16(&m_iThreadRunState, 0);

	m_newCachedDataNotify.notify_all();
	m_newReliableDataNotify.notify_all();

	if (m_cacheReliableDataThread.joinable())
		m_cacheReliableDataThread.join();

	if (m_reliableThread.joinable())
		m_reliableThread.join();
}

void ReliableUDP::ReliableProcessor() {
	std::unique_lock<std::mutex> mutex = std::unique_lock<std::mutex>(m_reliableProcessorMutex);

	while (m_iThreadRunState) {
		if (m_reliableDataQueue.IsEmpty())
			m_newReliableDataNotify.wait(mutex);

		ReliableData* data = nullptr; 
		if (m_reliableDataQueue.Pop(data)) {
			for (size_t i = 0; i < REPEAT_COUNT_FOR_RELIABLE_SEND; i++) {
				m_pUDPSocket->WriteTo(data->m_sendToAddress, data->m_packet);

				std::cv_status status = m_sendCompleteNotify.wait_for(mutex, std::chrono::seconds(1));
				if (status == std::cv_status::no_timeout) {
					if (data->m_packet.m_packetInfo.m_iPacketNumber == m_iSendCompletedPacketNumber)
						delete data;
					else
						m_newCachedDataNotify.notify_all();
					break;
				}
				m_cacheReliableDataList.insert(std::make_pair(data->m_packet.m_packetInfo.m_iPacketNumber, data));
			}
		}
	}
}

void ReliableUDP::CacheReliableDataProcessor() {
	std::unique_lock<std::mutex> mutex = std::unique_lock<std::mutex>(m_cacheReliableProcessorMutex);

	while (m_iThreadRunState) {
		m_newCachedDataNotify.wait(mutex);

		auto cachedReliableData = m_cacheReliableDataList.find(m_iSendCompletedPacketNumber);
		if (cachedReliableData != m_cacheReliableDataList.cend()) {
			delete cachedReliableData->second;
			m_cacheReliableDataList.erase(m_iSendCompletedPacketNumber);
			continue;
		}
		m_cacheReliableDataList.clear();
	}
}

UDPIPSocket::UDPIPSocket() : BaseSocket(UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_UDP), m_reliableProcessor(std::make_unique<ReliableUDP>(this)) {
}

UDPIPSocket::~UDPIPSocket() {
}

bool UDPIPSocket::WriteToReliable(const SocketAddress& sendAddress, const PACKET_STRUCT& sendPacketStructure) {
	if (m_reliableProcessor->AddReliableData(sendPacketStructure, sendAddress))
		return true;
	return false;
}

bool UDPIPSocket::WriteTo(const SocketAddress& sendAddress, const char* const sSendData, const uint16_t iDataLength) {
	return UTIL::UDP::SendTo(GetSocket(), sendAddress, sSendData, iDataLength);
}

bool UDPIPSocket::WriteTo(const SocketAddress& sendAddress, const PACKET_STRUCT& sendPacketStructure) {
	SetAckNumberToBuffer(sendPacketStructure);

	return UTIL::UDP::SendTo(GetSocket(), sendAddress, m_sSendMessageBuffer, sizeof(int32_t) + sizeof(PACKET_INFORMATION) + sendPacketStructure.m_packetInfo.m_iPacketDataSize);
}

bool UDPIPSocket::ReadFrom(char* const sReceiveBuffer, uint16_t& iRecvBytes) {
	OVERLAPPED_EX receiveOverlapped;
	return UTIL::UDP::ReceiveFrom(GetSocket(), sReceiveBuffer, iRecvBytes, receiveOverlapped);
}

bool UDPIPSocket::ReadFrom(OVERLAPPED_EX& receiveOverlapped) {
	uint16_t iRecvBytes = 0;
	return UTIL::UDP::ReceiveFrom(GetSocket(), GetReceiveBuffer(), iRecvBytes, receiveOverlapped);
}

bool UDPIPSocket::SendCompletion(const uint16_t iSendBytes) {
	m_reliableProcessor->Notify_SendComplete(iSendBytes);
	return true;
}

void UDPIPSocket::SetAckNumberToBuffer(const PACKET_STRUCT& sendPacketStructure) {
	ZeroMemory(m_sSendMessageBuffer, ::MAX_BUFFER_LENGTH);

	int16_t iAckNumber = 0;
	int32_t iPacketInfoHeader = (sendPacketStructure.m_packetInfo.m_iPacketNumber << 16) | iAckNumber;

	CopyMemory(m_sSendMessageBuffer, reinterpret_cast<const char*>(&iPacketInfoHeader), sizeof(int32_t));
	CopyMemory(m_sSendMessageBuffer + sizeof(int32_t), reinterpret_cast<const char*>(&sendPacketStructure.m_packetInfo), sizeof(PACKET_INFORMATION));
	CopyMemory(m_sSendMessageBuffer + sizeof(int32_t) + sizeof(PACKET_INFORMATION), sendPacketStructure.m_sPacketData, sendPacketStructure.m_packetInfo.m_iPacketDataSize);
}


/* UTIL */

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::SendTo(const::SOCKET hSocket, const SocketAddress& sendAddress, const char* const sSendBuffer, const uint16_t iDataLength) {
	SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX sendToOverlapped;
	DWORD iSendBytes;
	WSABUF wsaBuffer;
	wsaBuffer.buf = const_cast<char* const>(sSendBuffer);
	wsaBuffer.len = iDataLength;

	if (WSASendTo(hSocket, &wsaBuffer, 1, &iSendBytes, 0, &sendAddress, sendAddress.GetSize(), &sendToOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSALastErrorCode = GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA SendTo : Failed to WSASend! - %d", iWSALastErrorCode);
			return false;
		}
	}
	return true;
}

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::ReceiveFrom(const::SOCKET hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBytes, SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped) {
	DWORD iRecvBytes, iFlag = 0;
	sockaddr* pRemoteAddress = const_cast<sockaddr*>(&receiveOverlapped.m_remoteAddress);
	INT iAddressSize = SocketAddress::GetSize();

	receiveOverlapped.m_pReceiveBuffer = sReceiveBuffer;

	receiveOverlapped.m_wsaBuffer.buf = sReceiveBuffer + receiveOverlapped.m_iRemainReceiveBytes;
	receiveOverlapped.m_wsaBuffer.len = ::MAX_BUFFER_LENGTH - receiveOverlapped.m_iRemainReceiveBytes;

	if (WSARecvFrom(hSocket, &receiveOverlapped.m_wsaBuffer, 1, &iRecvBytes, &iFlag, pRemoteAddress, &iAddressSize, &receiveOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSALastErrorCode = GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA RecvFrom : Failed to RecvFrom! - %d", iWSALastErrorCode);
			return false;
		}
	}
	iReceiveBytes = iRecvBytes;
	return true;
}

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::CheckAck(OVERLAPPED_EX& overlapped) {
	USER_SESSION::User* pOwner = reinterpret_cast<USER_SESSION::User*>(overlapped.m_pOwner);
	if (pOwner && overlapped.m_pReceiveBuffer) {
		const int32_t iPacketInfoHeader = *reinterpret_cast<int32_t*>(overlapped.m_pReceiveBuffer);
		const int16_t iAckNumber = static_cast<int16_t>(iPacketInfoHeader);
		const int16_t iPacketNumber = static_cast<int16_t>(iPacketInfoHeader >> 16);

		if (iAckNumber == 9999) {
			if(overlapped.m_iLastReceivedPacketNumber <= iPacketNumber - 1)
				overlapped.m_iLastReceivedPacketNumber = iPacketNumber;
			overlapped.m_iRemainReceiveBytes -= sizeof(int32_t);

			return pOwner->SendCompletion(EPROTOCOLTYPE::EPT_UDP, iPacketNumber - 1);
		}
		else if (iAckNumber == 0) {
			int16_t iSendAckNumber = 9999;
			int16_t iSendPacketNumber = iPacketNumber + 1;
			int32_t iSendPacektInfoHeader = (iSendPacketNumber << 16) | iSendAckNumber;
			overlapped.m_iRemainReceiveBytes -= sizeof(int32_t);
			MoveMemory(overlapped.m_pReceiveBuffer, overlapped.m_pReceiveBuffer + sizeof(int32_t), overlapped.m_iRemainReceiveBytes);

			return pOwner->SendTo(overlapped.m_remoteAddress, reinterpret_cast<char* const>(&iSendPacektInfoHeader), sizeof(int32_t));
		}
	}
	return false;
}

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::CheckAck(NETWORK::PROTOCOL::UDP::UDPIPSocket* const pUdpSocket, const FUNCTIONS::SOCKETADDRESS::SocketAddress& remoteAddress, char* const sReceviedBuffer, uint16_t& iReceivedBytes, int16_t iUpdatedPacketNumber) {
	if (pUdpSocket && sReceviedBuffer) {
		const int32_t iPacketInfoHeader = *reinterpret_cast<int32_t*>(sReceviedBuffer);
		const int16_t iAckNumber = static_cast<int16_t>(iPacketInfoHeader);
		const int16_t iPacketNumber = static_cast<int16_t>(iPacketInfoHeader >> 16);

		if (iAckNumber == 9999) {
			if (iUpdatedPacketNumber <= iPacketNumber - 1)
				InterlockedExchange16(&iUpdatedPacketNumber, iPacketNumber);
			iReceivedBytes -= sizeof(int32_t);

			return pUdpSocket->SendCompletion(iPacketNumber - 1);
		}
		else if (iAckNumber == 0) {
			int16_t iSendAckNumber = 9999;
			int16_t iSendPacketNumber = iPacketNumber + 1;
			int32_t iSendPacektInfoHeader = (iSendPacketNumber << 16) | iSendAckNumber;
			iReceivedBytes -= sizeof(int32_t);
			MoveMemory(sReceviedBuffer, sReceviedBuffer + sizeof(int32_t), iReceivedBytes);

			return pUdpSocket->WriteTo(remoteAddress, reinterpret_cast<char* const>(&iSendPacektInfoHeader), sizeof(int32_t));
		}
	}
	return false;
}
