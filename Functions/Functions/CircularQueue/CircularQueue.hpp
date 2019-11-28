#pragma once
#include <Functions/Functions/CriticalSection/CriticalSection.hpp>
#include <Functions/Functions/MemoryPool/MemoryPool.hpp>

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {

		static const size_t MAX_QUEUE_LENGTH = 400;

		namespace QUEUEDATA {
			namespace DETAIL {
				template<typename Type, size_t ALLOC_BLOCK_SIZE = 50>
				struct BaseData : public FUNCTIONS::MEMORYMANAGER::CMemoryManager<Type, ALLOC_BLOCK_SIZE> {
				public:
					explicit BaseData() {};

				};
			}
		}

		template<typename DATATYPE>
		class CCircularQueue {
		private:
			int16_t m_bIsSyncOn;
			CRITICALSECTION::DETAIL::CCriticalSection m_SyncForQueue;

		private:
			DATATYPE m_Queue[MAX_QUEUE_LENGTH];
			size_t m_Head, m_Tail;

		public:
			explicit CCircularQueue() : m_Head(0), m_Tail(0) { ZeroMemory(m_Queue, MAX_QUEUE_LENGTH); };
			explicit CCircularQueue(const CCircularQueue& rhs) : m_SyncForQueue(rhs.m_SyncForQueue), m_Head(rhs.m_Head), m_Tail(rhs.m_Tail) {
				CopyMemory(m_Queue, rhs.m_Queue, MAX_QUEUE_LENGTH);
			}

			~CCircularQueue() {};

		public:
			const DATATYPE& Push(const DATATYPE& InData) {
				if (m_bIsSyncOn)
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_SyncForQueue);

				size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
				if (TempTail == m_Tail) {
					return nullptr;
				}
				m_Queue[TempTail] = InData;
				m_Tail = TempTail;

				return InData;
			}

			bool Pop() {
				if (IsEmpty()) {
					return false;
				}

				if (m_bIsSyncOn)
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_SyncForQueue);

				size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
				if (TempHead == m_Head) {
					return false;
				}
				m_Queue[TempHead] = DATATYPE();
				m_Head = TempHead;

				return true;
			}

			bool Pop(DATATYPE& InData) {
				if (IsEmpty()) {
					return false;
				}

				if (m_bIsSyncOn)
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_SyncForQueue);

				size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
				if (TempHead == m_Head) {
					return false;
				}
				InData = m_Queue[TempHead];
				m_Head = TempHead;

				return true;
			}

			bool IsEmpty() {
				if (m_bIsSyncOn)
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_SyncForQueue);

				if (m_Head == m_Tail) {
					return true;
				}
				return false;
			}

			void ExchangeSyncState(bool NewState) {
				InterlockedExchange16(&m_bIsSyncOn, NewState);
			}
		};
	}
}