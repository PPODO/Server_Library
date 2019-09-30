#pragma once
#include <iostream>
#include <Windows.h>
#include "../Uncopyable/Uncopyable.h"

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
			DETAIL::CCriticalSection& m_CriticalSection;

		public:
			explicit CCriticalSectionGuard(DETAIL::CCriticalSection& CriticalSection) : m_CriticalSection(CriticalSection) {
				m_CriticalSection.Lock();
			}

			~CCriticalSectionGuard() {
				m_CriticalSection.UnLock();
			}

		};
	}
}