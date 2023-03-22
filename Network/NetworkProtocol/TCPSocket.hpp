#pragma once
#include "Socket/Socket.hpp"
#include "../Packet/BasePacket.hpp"
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <memory>

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			namespace USER_SERVER {
				struct OVERLAPPED_EX;
			}
		}

		namespace PROTOCOL {
			namespace TCP {
				struct WSASendData : FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<WSASendData> {
				public:
					char m_sBuffer[PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE];
					uint16_t m_iDataLength;

				public:
					WSASendData() : m_iDataLength(0) { ZeroMemory(m_sBuffer, sizeof(char) * PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE); };
					WSASendData(const char* const sBuffer, const uint16_t iDataLength) : m_iDataLength(iDataLength) {
						CopyMemory(m_sBuffer, sBuffer, iDataLength);
					}
					WSASendData(const PACKET::PACKET_STRUCT& packetStructure) {
						CopyMemory(m_sBuffer, reinterpret_cast<const char*>(&packetStructure.m_packetInfo), sizeof(PACKET::PACKET_INFORMATION));
						m_iDataLength = sizeof(PACKET::PACKET_INFORMATION);
						CopyMemory(m_sBuffer + m_iDataLength, packetStructure.m_sPacketData, packetStructure.m_packetInfo.m_iPacketDataSize);
						m_iDataLength += packetStructure.m_packetInfo.m_iPacketDataSize;
					}

				};

				class TCPIPSocket : public BSD_SOCKET::BaseSocket {
				private:
					FUNCTIONS::CIRCULARQUEUE::CircularQueue<WSASendData*> m_sendMessageQueue;

				public:
					TCPIPSocket();
					virtual ~TCPIPSocket() override;

				public:
					bool Listen(const int32_t iBackLogCount = SOMAXCONN);
					bool Connect(const FUNCTIONS::SOCKETADDRESS::SocketAddress& connectAddress);
					bool Accept(const TCPIPSocket& listenSocket, USER_SESSION::USER_SERVER::OVERLAPPED_EX& acceptOverlapped);

					bool Write(const char* const sSendData, const uint16_t iDataLength);
					bool Write(const char* const sSendData, const uint16_t iDataLength, USER_SESSION::USER_SERVER::OVERLAPPED_EX& sendOverlapped);
					bool Write(const PACKET::PACKET_STRUCT& sendPacketStructure);
					bool Write(const PACKET::PACKET_STRUCT& sendPacketStructure, USER_SESSION::USER_SERVER::OVERLAPPED_EX& sendOverlapped);

					bool Read(char* const sReceiveBuffer, uint16_t& iReceiveBytes);
					bool Read(USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped);

					bool SocketRecycling(USER_SESSION::USER_SERVER::OVERLAPPED_EX& disconnectOverlapped);

					virtual bool SendCompletion(const uint16_t iSendBytes) override;

				};
			}

			namespace UTIL {
				namespace TCP {
					bool Send(const ::SOCKET& hSocket, char* const sSendBuffer, const uint16_t iSendBufferSize, USER_SESSION::USER_SERVER::OVERLAPPED_EX& sendOverlapped);
					bool Receive(const ::SOCKET& hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBufferSize, USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped);
				}
			}
		}
	}
}