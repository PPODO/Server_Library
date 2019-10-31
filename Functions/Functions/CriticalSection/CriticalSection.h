#pragma once
#include <iostream>
#include <Windows.h>
#include <Functions/Functions/Uncopyable/Uncopyable.h>

namespace FUNCTIONS {
	namespace CRITICALSECTION {
		namespace DETAIL {
			class CCriticalSection : private UNCOPYABLE::CUncopyable {
			private:
				CRITICAL_SECTION m_Lock;

			public:
				explicit CCriticalSection() {
					InitializeCriticalSection(&m_Lock);
				}

				~CCriticalSection() {
					DeleteCriticalSection(&m_Lock);
				}

			public:
				inline void Lock() {
					EnterCriticalSection(&m_Lock);
				}

				inline void UnLock() {
					LeaveCriticalSection(&m_Lock);
				}

			};
		}

		class CCriticalSectionGuard : private UNCOPYABLE::CUncopyable {
		private:
			DETAIL::CCriticalSection* const m_CriticalSection = nullptr;

		public:
			explicit CCriticalSectionGuard(DETAIL::CCriticalSection* const CriticalSection) : m_CriticalSection(CriticalSection) {
				if (m_CriticalSection) {
					m_CriticalSection->Lock();
				}
			}

			~CCriticalSectionGuard() {
				if (m_CriticalSection) {
					m_CriticalSection->UnLock();
				}
			}

		};
	}
}