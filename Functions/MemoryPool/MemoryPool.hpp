#pragma once
#include "../CriticalSection/CriticalSection.hpp"
#include "../Log/Log.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
// 메모리풀 전체 수정
// 할당된 블럭이 부족하여 새로 할당할 경우 문제 발생
// 이전 방식의 메모리 풀 코드로 돌아갈 듯.
namespace SERVER {
	namespace FUNCTIONS {
		namespace MEMORYMANAGER {
			namespace MEMORYPOOL {
				class CMemoryPool {
				private:
					uint8_t* m_pMemoryHead;

				private:
					const size_t m_iMaxPoolCount;
					const size_t m_iAllocBlockSize;

				private:
					void AllocateBlock() {
						m_pMemoryHead = new uint8_t[m_iAllocBlockSize * m_iMaxPoolCount];

						uint8_t** pCurrentPtr = reinterpret_cast<uint8_t**>(m_pMemoryHead);
						uint8_t* pNextPtr = m_pMemoryHead;

						for (size_t i = 0; i < m_iMaxPoolCount - 1; i++) {
							pNextPtr += m_iAllocBlockSize;
							*pCurrentPtr = pNextPtr;
							pCurrentPtr = reinterpret_cast<uint8_t**>(pNextPtr);
						}
						*pCurrentPtr = nullptr;
					}

				public:
					CMemoryPool(const size_t& iMaxPoolCount, const size_t& iAllocBlockSize) : m_iMaxPoolCount(iMaxPoolCount), m_iAllocBlockSize(iAllocBlockSize), m_pMemoryHead(nullptr) {}
					virtual ~CMemoryPool() {
					}

				public:
					void* const Allocate() {
						if (!m_pMemoryHead)
							AllocateBlock();

						auto pMemoryBlock = m_pMemoryHead;
						m_pMemoryHead = *reinterpret_cast<uint8_t**>(pMemoryBlock);

						return pMemoryBlock;
					}

					void Delocate(void* const pPointer) {
						*reinterpret_cast<uint8_t**>(pPointer) = m_pMemoryHead;
						m_pMemoryHead = static_cast<uint8_t*>(pPointer);
					}
				};
			}

			template<typename T, size_t POOL_MAX_SIZE = 50>
			class CMemoryManager {
			private:
				static CRITICALSECTION::CriticalSection m_lock;

			private:
				static std::unique_ptr<MEMORYPOOL::CMemoryPool> m_memoryPool;

			public:
				static void* operator new(std::size_t iAllocSize) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					if (!m_memoryPool) {
						try {
							m_memoryPool = std::make_unique<MEMORYPOOL::CMemoryPool>(POOL_MAX_SIZE, sizeof(T));
						}
						catch (const std::bad_alloc& exception) {
							LOG::Log::WriteLog(UTIL::MBToUni(exception.what()).c_str());
							std::abort();
						}
					}

					if (m_memoryPool)
						return m_memoryPool->Allocate();

					return nullptr;
				}

				static void operator delete(void* const pPointer) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					if (!pPointer) return;

					if (m_memoryPool)
						m_memoryPool->Delocate(pPointer);
				}
			};
		}
	}
}

template<typename T, size_t POOL_MAX_SIZE>
__declspec(selectany) SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<T, POOL_MAX_SIZE>::m_lock;

template<typename T, size_t POOL_MAX_SIZE>
__declspec(selectany) std::unique_ptr<SERVER::FUNCTIONS::MEMORYMANAGER::MEMORYPOOL::CMemoryPool> SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<T, POOL_MAX_SIZE>::m_memoryPool;