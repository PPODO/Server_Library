#pragma once
#pragma comment(lib, "winmm.lib") 
#include "../CriticalSection/CriticalSection.hpp"
#include <Windows.h>
#include <time.h>

namespace SERVER {
	namespace FUNCTIONS {
		namespace FPS {
			class FPSMANAGER {
			public:
				FPSMANAGER() {
					timeBeginPeriod(1);

					m_iOldTime = timeGetTime();
				}

				~FPSMANAGER() {
					timeEndPeriod(1);
				}

				static bool Skip() {
					const int iCachedCurrentTime = timeGetTime() - m_iOldTime;
					const int iSleepTick = 20 - iCachedCurrentTime;
					if (iSleepTick > 0)
						Sleep(iSleepTick);

					m_iOldTime = timeGetTime();

					return iCachedCurrentTime >= 20;
				}

			private:
				static std::atomic_uint64_t m_iOldTime;

			};

			static FPSMANAGER g_FPSManagerInst;
		}
	}
}

__declspec(selectany) std::atomic_uint64_t SERVER::FUNCTIONS::FPS::FPSMANAGER::m_iOldTime(0);