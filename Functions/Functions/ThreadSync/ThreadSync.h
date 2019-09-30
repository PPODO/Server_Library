#pragma once
#include "../CriticalSection/CriticalSection.h"

namespace FUNCTIONS {
	namespace THREADSYNC {
		template<typename T>
		class CMultiThreadSync {
		private:
			static FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_Critical;

		public:
			class CThreadSync {
			public:
				CThreadSync() {
					T::m_Critical.Lock();
				}

				CThreadSync(CMultiThreadSync<T>::CThreadSync&) = delete;

				virtual ~CThreadSync() {
					T::m_Critical.UnLock();
				}
			};

		};

		template<typename T>
		FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection CMultiThreadSync<T>::m_Critical;
	}
}