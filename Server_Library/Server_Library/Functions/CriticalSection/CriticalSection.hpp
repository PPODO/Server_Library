#pragma once
#include <iostream>
#include <Windows.h>
#include "../Uncopyable/Uncopyable.hpp"

namespace SERVER {
	namespace FUNCTIONS {
		namespace CRITICALSECTION {
			class CriticalSection : private UNCOPYABLE::Uncopyable {
			private:
				CRITICAL_SECTION m_criticalSection;

			public:
				CriticalSection(DWORD dwSpinCount = 0) {
					InitializeCriticalSectionEx(&m_criticalSection, dwSpinCount, 0);
				}

				~CriticalSection() {
					DeleteCriticalSection(&m_criticalSection);
				}

			public:
				__forceinline void Lock() {
					EnterCriticalSection(&m_criticalSection);
				}

				__forceinline void UnLock() {
					LeaveCriticalSection(&m_criticalSection);
				}
			};

			class CriticalSectionGuard : private UNCOPYABLE::Uncopyable {
			private:
				CriticalSection& m_CriticalSection;

			public:
				CriticalSectionGuard(CriticalSection& inst) : m_CriticalSection(inst) {
					m_CriticalSection.Lock();
				}

				~CriticalSectionGuard() {
					m_CriticalSection.UnLock();
				}
			};
		}
	}
}