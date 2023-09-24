#pragma once
#include "../CriticalSection/CriticalSection.hpp"
#include "../Log/Log.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

namespace SERVER {
	namespace FUNCTIONS {
		namespace MEMORYMANAGER {
			namespace MEMORYPOOL {
				class MemoryPool {
				private:
					std::vector<uint8_t*> m_memoryPoolList;
					size_t m_iCurrentIndex;

				private:
					const size_t m_iMaxPoolCount;
					const size_t m_iAllocBlockSize;

				private:
					void AllocateBlock() {
						m_memoryPoolList.resize(m_iCurrentIndex + m_iMaxPoolCount);

						try {
							for (auto& it : m_memoryPoolList) {
								if (uint8_t* pNewData = new uint8_t[m_iAllocBlockSize])
									it = pNewData;
							}
						}
						catch (const std::bad_alloc& exception) {
							LOG::Log::WriteLog(UTIL::MBToUni(exception.what()).c_str());
						}
					}

				public:
					MemoryPool(const size_t& iMaxPoolCount, const size_t& iAllocBlockSize) : m_iMaxPoolCount(iMaxPoolCount), m_iAllocBlockSize(iAllocBlockSize), m_iCurrentIndex(0) {}
					virtual ~MemoryPool() {
						for (auto& it : m_memoryPoolList)
							delete it;
					}

				public:
					void* const Allocate() {
						if (m_memoryPoolList.size() <= m_iCurrentIndex)
							AllocateBlock();

						auto pMemoryBlock = m_memoryPoolList[m_iCurrentIndex];
						m_memoryPoolList[m_iCurrentIndex++] = nullptr;

						return pMemoryBlock;
					}

					void Delocate(void* const pPointer) {
						if (m_iCurrentIndex <= 0)
							return;

						m_memoryPoolList[--m_iCurrentIndex] = static_cast<uint8_t*>(pPointer);
					}
				};
			}

			template<typename T, size_t POOL_MAX_SIZE = 50>
			class MemoryManager {
			private:
				static CRITICALSECTION::CriticalSection m_lock;

			private:
				static std::unique_ptr<MEMORYPOOL::MemoryPool> m_memoryPool;

			public:
				static void* operator new(std::size_t iAllocSize) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					if (!m_memoryPool) {
						try {
							m_memoryPool = std::make_unique<MEMORYPOOL::MemoryPool>(POOL_MAX_SIZE, sizeof(T));
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

					if (m_memoryPool)
						m_memoryPool->Delocate(pPointer);
				}
			};
		}
	}
}

template<typename T, size_t POOL_MAX_SIZE>
__declspec(selectany) SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection SERVER::FUNCTIONS::MEMORYMANAGER::MemoryManager<T, POOL_MAX_SIZE>::m_lock;

template<typename T, size_t POOL_MAX_SIZE>
__declspec(selectany) std::unique_ptr<SERVER::FUNCTIONS::MEMORYMANAGER::MEMORYPOOL::MemoryPool> SERVER::FUNCTIONS::MEMORYMANAGER::MemoryManager<T, POOL_MAX_SIZE>::m_memoryPool;