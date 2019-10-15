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
				FUNCTIONS::SOCKADDR::CSocketAddress m_SendAddress;
				NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_SendOverlapped;

			public:
				explicit ReliableData() : m_DataSize(0) { ZeroMemory(m_Data, MAX_RELIABLE_BUFFER_LENGTH); ZeroMemory(&m_SendOverlapped, sizeof(m_SendOverlapped)); };
				ReliableData(const char* const Data, const size_t& DataSize, const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) : m_DataSize(DataSize), m_SendAddress(SendAddress) { CopyMemory(m_Data, Data, DataSize); CopyMemory(&m_SendOverlapped, &SendOverlapped, sizeof(m_SendOverlapped)); };

			};
		}

		template<> bool CCircularQueue<std::unique_ptr<QUEUEDATA::ReliableData>>::Pop(std::unique_ptr<QUEUEDATA::ReliableData>& InData) {
			CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);

			size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
			if (TempHead == m_Head) {
				return false;
			}
			InData = std::move(m_Queue[TempHead]);
			m_Head = TempHead;

			return true;
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
				bool WriteToQueue(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);
				bool WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);

			public:
				bool ReadFrom(char* const ReceivedBuffer, uint16_t& RecvBytes);
				bool ReadFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped);

			};
		}
	}

	namespace UTIL {
		bool ReceiveFrom(const ::SOCKET& Socket, char* const ReceivedBuffer, uint16_t& ReceivedBytes, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped);
	}
}