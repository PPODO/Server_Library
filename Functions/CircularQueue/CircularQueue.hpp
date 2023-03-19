#pragma once
#include "../CriticalSection/CriticalSection.hpp"
#include "../MemoryPool/MemoryPool.h"

namespace SERVER {
	namespace FUNCTIONS {
		namespace CIRCULARQUEUE {
			namespace QUEUEDATA {
				template<typename T, size_t ALLOC_BLOCK_SIZE = 50>
				struct BaseData : FUNCTIONS::MEMORYMANAGER::MemoryManager<T, ALLOC_BLOCK_SIZE> {
				public:
					BaseData() {};

				};
			}

			static const size_t MAX_QUEUE_LENGTH = 400;

			template<typename DATATYPE>
			class CircularQueue {
			private:
				CRITICALSECTION::CriticalSection m_lock;

			private:
				DATATYPE m_queueList[MAX_QUEUE_LENGTH];
				size_t m_iHead, m_iTail;

			public:
				CircularQueue() : m_iHead(0), m_iTail(0) { ZeroMemory(m_queueList, sizeof(DATATYPE) * MAX_QUEUE_LENGTH); }
				CircularQueue(const CircularQueue& rhs) : m_lock(rhs.m_lock), m_iHead(rhs.m_iHead), m_iTail(rhs.m_iTail) {
					CopyMemory(m_queueList, rhs.m_queueList, MAX_QUEUE_LENGTH);
				}

			public:
				const DATATYPE& Push(const DATATYPE& inData) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					size_t iDataInsertedIndex = (m_iTail + 1) % MAX_QUEUE_LENGTH;
					m_queueList[iDataInsertedIndex] = inData;
					m_iTail = iDataInsertedIndex;

					return inData;
				}

				bool Pop(DATATYPE& outData) {
					if (IsEmpty())
						return false;

					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					size_t iDataPopIndex = (m_iHead + 1) % MAX_QUEUE_LENGTH;
					outData = m_queueList[iDataPopIndex];
					m_iHead = iDataPopIndex;

					return true;
				}


				bool IsEmpty() {
					return m_iHead == m_iTail;
				}

			};
		}
	}
}