#pragma once
#include <Network/Socket/Socket.h>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <thread>
#include <memory>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			static const size_t MAX_RELIABLE_BUFFER_LENGTH = 2048;

			struct ReliableData : public DETAIL::BaseData<ReliableData> {
			public:
				char m_Data[MAX_RELIABLE_BUFFER_LENGTH];
				size_t m_DataSize;
				FUNCTIONS::SOCKADDR::CSocketAddress m_Address;
				NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_OverlappedEx;

			public:
				explicit ReliableData() : m_DataSize(0) { ZeroMemory(m_Data, MAX_RELIABLE_BUFFER_LENGTH); ZeroMemory(&m_OverlappedEx, sizeof(m_OverlappedEx)); };
				ReliableData(const char* const Data, const size_t& DataSize, const FUNCTIONS::SOCKADDR::CSocketAddress& Address, const NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& OverlappedEx) : m_DataSize(DataSize), m_Address(Address) { CopyMemory(m_Data, Data, MAX_RELIABLE_BUFFER_LENGTH); CopyMemory(&m_OverlappedEx, &OverlappedEx, sizeof(m_OverlappedEx)); };

			};
		}
	}
}

namespace NETWORK {
	namespace SOCKET {
		namespace UDPIP {
			static const size_t REPEAT_COUNT_FOR_RELIABLE_SEND = 12;

			class CUDPIPSocket : public BASESOCKET::CBaseSocket {
			private:
				HANDLE m_hSendCompleteEvent;
				HANDLE m_hNewReliableDataEvent;
				HANDLE m_hWaitForInitializeThreadEvent;

			private:
				std::thread m_ReliableThreadHandle;
				std::thread m_HeartbeatThread;

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
				bool WriteToQueue(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendToOverlapped);
				bool WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);

			public:
				bool ReadFrom(char* const RecvBuffer, uint16_t& ReceivedBytes);
				bool ReadFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvFromOverlapped);

			};
		}
	}

	namespace UTIL {
		namespace UDPIP {
			
			bool ReceiveFrom(const ::SOCKET& Socket, char* const RecvBuffer, uint16_t& ReceivedBytes, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvFromOverlapped);
		}
	}
}