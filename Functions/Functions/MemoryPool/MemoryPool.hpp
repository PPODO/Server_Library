#pragma once
#include <Functions/Functions/Log/Log.hpp>
#include <Functions/Functions/CriticalSection/CriticalSection.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace FUNCTIONS {
	namespace MEMORYMANAGER {
		namespace MEMORYPOOL {
			class CMemoryPool {
			private:
				std::vector<uint8_t*> m_MemoryPoolList;
				size_t m_CurrentIndex;

			private:
				const size_t m_MaxPoolCount;
				const size_t m_AllocBlockSize;

			private:
				void AllocateBlock() {
					m_MemoryPoolList.resize(m_CurrentIndex + m_MaxPoolCount);

					try {
						for (auto& It : m_MemoryPoolList) {
							uint8_t* NewData = new uint8_t[m_AllocBlockSize];
							if (NewData) {
								It = NewData;
							}
							else {
								continue;
							}
						}

					}
					catch (const std::bad_alloc& exception) {
						LOG::CLog::WriteLog(exception.what());
					}

				}

			public:
				explicit CMemoryPool(const uint32_t& MaxPoolCount, const uint32_t& AllocBlockSize) : m_MaxPoolCount(MaxPoolCount), m_AllocBlockSize(AllocBlockSize), m_CurrentIndex(0) { }

				virtual ~CMemoryPool() {
					for (auto& It : m_MemoryPoolList) {
						delete It;
						It = nullptr;
					}
				}

			public:
				void* const Allocate() {
					if (m_MemoryPoolList.size() <= m_CurrentIndex) {
						AllocateBlock();
					}

					auto ReturnValue = m_MemoryPoolList[m_CurrentIndex];
					m_MemoryPoolList[m_CurrentIndex] = nullptr;
					m_CurrentIndex++;
					return ReturnValue;
				}

				void Delocate(void* const DeletePointer) {
					if (m_CurrentIndex <= 0) {
						return;
					}

					m_MemoryPoolList[m_CurrentIndex - 1] = static_cast<uint8_t*>(DeletePointer);
					--m_CurrentIndex;
				}

			};
		}

		template<typename T, size_t POOL_MAX_SIZE = 50>
		class CMemoryManager  {
		private:
			inline static CRITICALSECTION::DETAIL::CCriticalSection m_Lock;

		private:
			inline static std::unique_ptr<MEMORYPOOL::CMemoryPool> m_Pool = nullptr;


		public:
			static void* operator new(std::size_t AllocSize) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_Lock);

				if (!m_Pool) {
					try {
						m_Pool = std::make_unique<MEMORYPOOL::CMemoryPool>(POOL_MAX_SIZE, sizeof(T));
					}
					catch (const std::bad_alloc& exception) {
						LOG::CLog::WriteLog(exception.what());
						std::abort();
					}
				}

				if (m_Pool) {
					return m_Pool->Allocate();
				}
				return nullptr;
			}

			static void operator delete(void* const DeletePointer) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_Lock);

				if (DeletePointer && m_Pool) {
					m_Pool->Delocate(DeletePointer);
				}
			}

		};
	}
}