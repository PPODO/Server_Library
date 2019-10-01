#pragma once
#include <Network/Socket/Socket.h>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <thread>
#include <memory>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			struct ReliableData : public BaseData<ReliableData> {
			public:
				void* const m_Data;
				size_t m_DataSize;

			public:
				explicit ReliableData() : m_Data(nullptr), m_DataSize(0) {};
				ReliableData(void* const Data, const size_t& DataSize) : m_Data(Data), m_DataSize(DataSize) {};

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
				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::ReliableData>> m_ReliableDataQueue;

			private:
				void ReliableThread();

			public:
				explicit CUDPIPSocket();
				virtual ~CUDPIPSocket() override;

			public:
				bool WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);

			};
		}
	}

	namespace UTIL {

	}
}