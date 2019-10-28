#pragma once
#include <Network/Socket/Socket.h>
#include <Network/Packet/BasePacket.hpp>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <thread>
#include <memory>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			static const size_t MAX_RELIABLE_BUFFER_LENGTH = 2048;

			struct ReliableData : public DETAIL::BaseData<ReliableData> {
			public:
				NETWORK::PACKET::PACKET_STRUCTURE m_SendPacketStructure;
				FUNCTIONS::SOCKADDR::CSocketAddress m_SendAddress;

			public:
				explicit ReliableData() {};
				ReliableData(const NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure, const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress) : m_SendPacketStructure(SendPacketStructure), m_SendAddress(SendAddress) { };

			};
		}
	}
}

namespace NETWORK {
	namespace SOCKET {
		namespace UDPIP {
			static const size_t REPEAT_COUNT_FOR_RELIABLE_SEND = 12;

			struct PEERINFO {
			public:
				FUNCTIONS::SOCKADDR::CSocketAddress m_RemoteAddress;
				uint16_t m_LastPacketNumber;

			public:
				PEERINFO() : m_LastPacketNumber(0) {};
				PEERINFO(const FUNCTIONS::SOCKADDR::CSocketAddress& Address, const uint16_t& LastPacketNumber) : m_RemoteAddress(Address), m_LastPacketNumber(LastPacketNumber) {};

			};

			class CUDPIPSocket : public BASESOCKET::CBaseSocket {
			private:
				HANDLE m_hSendCompleteEvent;
				HANDLE m_hNewReliableDataEvent;
				HANDLE m_hWaitForInitializeThreadEvent;

			private:
				std::thread m_ReliableThreadHandle;

			private:
				int16_t m_ThreadRunState;

			private:
				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::ReliableData*> m_ReliableDataQueue;

			private:
				void ReliableThread();

			public:
				explicit CUDPIPSocket();
				virtual ~CUDPIPSocket() override;

			public:
				bool WriteToQueue(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure);
				bool WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure);
				bool WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const uint16_t& SendDataLength);

			public:
				bool ReadFrom(char* const ReceivedBuffer, uint16_t& RecvBytes);
				bool ReadFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped);

			public:
				virtual bool SendCompletion() final override;

			};
		}
	}

	namespace UTIL {
		namespace UDPIP {
			bool SendTo(const ::SOCKET& Socket, const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendBuffer, const uint16_t& SendBytes);

			bool ReceiveFrom(const ::SOCKET& Socket, char* const ReceivedBuffer, uint16_t& ReceivedBytes, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped);
		}
	}
}