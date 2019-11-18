#pragma once
#include <Network/Socket/Socket.hpp>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <memory>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			typedef struct CWSASendData : public DETAIL::BaseData<CWSASendData> {
			public:
				char m_Buffer[NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE];
				uint16_t m_Length;

			public:
				CWSASendData() : m_Length(0) { ZeroMemory(m_Buffer, NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE); };
				CWSASendData(const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) : m_Length(0) {
					CopyMemory(m_Buffer, reinterpret_cast<const char*>(&PacketStructure.m_PacketInformation), PacketStructure.m_PacketInformation.GetSize());
					m_Length += PacketStructure.m_PacketInformation.GetSize();
					CopyMemory(m_Buffer + m_Length, PacketStructure.m_PacketData, PacketStructure.m_PacketInformation.m_PacketSize);
					m_Length += PacketStructure.m_PacketInformation.m_PacketSize;
				};
				CWSASendData(const char* const Buffer, const uint16_t& Length) : m_Length(0) {
					CopyMemory(m_Buffer, Buffer, Length);
				}

			};
		}
	}
}

namespace NETWORK {
	namespace UTIL {
		namespace SESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					struct OVERLAPPED_EX;
				}
			}
		}
	}

	namespace SOCKET {
		namespace TCPIP {
			class CTCPIPSocket : public BASESOCKET::CBaseSocket {
			private:
				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CWSASendData*> m_SendMessageQueue;

			public:
				explicit CTCPIPSocket();
				virtual ~CTCPIPSocket() override;

			public:
				bool Listen(const int32_t& BackLogCount = SOMAXCONN);
				bool Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress);
				bool Accept(const CTCPIPSocket& ListenSocket, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped);

			public:
				bool Write(const char* const SendData, const uint16_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);
				bool Write(const PACKET::PACKET_STRUCTURE& PacketStructure, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);
				bool Write(const char* const SendData, const uint16_t& DataLength);
				bool Write(const PACKET::PACKET_STRUCTURE& PacketStructure);

			public:
				bool Read(char* const ReadBuffer, uint16_t& ReadedSize);
				bool Read(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped);

			public:
				bool SocketRecycling(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& DisconnectOverlapped);

			public:
				virtual bool SendCompletion(const uint16_t& SendBytes) final override;

			};
		}
	}

	namespace UTIL {
		namespace TCPIP {
			inline bool Send(const ::SOCKET& Socket, const char* const SendBuffer, const uint16_t& SendBufferSize, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);

			inline bool Receive(const ::SOCKET& Socket, char* const ReceiveBuffer, uint16_t& ReceiveBufferSize, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped);
		}
	}
}