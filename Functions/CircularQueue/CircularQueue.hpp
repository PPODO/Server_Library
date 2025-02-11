#pragma once
#include "../CriticalSection/CriticalSection.hpp"
#include "../MemoryPool/MemoryPool.h"

namespace SERVER {
	namespace FUNCTIONS {
		namespace CIRCULARQUEUE {
			namespace QUEUEDATA {
				template<typename T, size_t ALLOC_BLOCK_SIZE = 50>
				struct BaseData : FUNCTIONS::MEMORYMANAGER::CMemoryManager<T, ALLOC_BLOCK_SIZE> {
				public:
					BaseData() {};

				};
			}

			static const size_t MAX_QUEUE_LENGTH = 400;

			template<typename DATATYPE, size_t QUEUE_LENGTH = MAX_QUEUE_LENGTH>
			class CircularQueue {
			private:
				CRITICALSECTION::CriticalSection m_lock;

			private:
				DATATYPE m_queueList[QUEUE_LENGTH];
				int m_iHead, m_iTail;

				bool m_bIsEnableCriticalSection;

			public:
				CircularQueue() : m_iHead(0), m_iTail(0), m_bIsEnableCriticalSection(true) { ZeroMemory(m_queueList, sizeof(DATATYPE) * QUEUE_LENGTH); }
				CircularQueue(const CircularQueue& rhs) : m_lock(rhs.m_lock), m_iHead(rhs.m_iHead), m_iTail(rhs.m_iTail), m_bIsEnableCriticalSection(rhs.m_bIsEnableCriticalSection) {
					CopyMemory(m_queueList, rhs.m_queueList, QUEUE_LENGTH);
				}

				~CircularQueue() {
					ZeroMemory(m_queueList, sizeof(DATATYPE) * QUEUE_LENGTH);
				}

			public:
				const DATATYPE& Push(const DATATYPE& inData) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					size_t iDataInsertedIndex = (m_iTail + 1) % QUEUE_LENGTH;

					if (iDataInsertedIndex == m_iHead)
						return DATATYPE();

					m_queueList[iDataInsertedIndex] = inData;
					m_iTail = iDataInsertedIndex;

					return inData;
				}

				bool Pop(DATATYPE& outData) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					if (IsEmpty())
						return false;

					size_t iDataPopIndex = (m_iHead + 1) % QUEUE_LENGTH;
					outData = m_queueList[iDataPopIndex];
					m_iHead = iDataPopIndex;

					return true;
				}


				bool IsEmpty() {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					return m_iHead == m_iTail;
				}

				void EnableCriticalSection(bool newValue) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					m_bIsEnableCriticalSection = newValue;
				}

			};
		}
	}
}