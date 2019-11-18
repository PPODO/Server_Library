#pragma once
#include <Functions/Functions/CriticalSection/CriticalSection.hpp>
#include <Functions/Functions/MemoryPool/MemoryPool.hpp>
#include <Functions/Functions/Uncopyable/Uncopyable.hpp>
#include <functional>
#include <chrono>
#include <thread>
#include <array>
#include <unordered_map>
#include <type_traits>

namespace FUNCTIONS {
	namespace TIMINGWHEEL {
		namespace DETAIL {
			struct TimerInformation {
			public:
				const bool m_bIsLoop;
				const size_t m_IntervalTime;
				const std::function<void()> m_Func;
				size_t m_RotataionCount{ 0 };
				bool m_bIsPause{ false };

			public:
				explicit TimerInformation(size_t IntervalTime, const std::function<void()> Function, bool bIsLoop = false) : m_IntervalTime(IntervalTime), m_Func(Function), m_bIsLoop(bIsLoop) {};


			};
		}

		// https://d2.naver.com/helloworld/267396 <- 해당 주소를 참고하여 만듦.
		/*
			TimingWheel의 기본적인 동작 과정은 슬롯 시간 간격(타이머 실행 주기)마다 배열을 순회하면서, 해당 배열에 담긴 타이머를 처리하는 방식
		*/

		template<size_t TIMESLOTCOUNT = 12>
		class CTimingWheel : private UNCOPYABLE::CUncopyable {
		private:
			int16_t m_TimingWheelThreadRunState{ 1 };
			std::thread m_ThreadHandle;

		private:
			// Millisecond, 해당 시간 간격마다 배열을 돌아 타이머를 처리함.
			const size_t m_SlotInterval;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_SlotArrayLock;
			std::array<std::list<std::pair<HANDLE, DETAIL::TimerInformation>>, TIMESLOTCOUNT> m_SlotArray;

		private:
			// 현재 처리되어야 하는 인덱스의 위치
			size_t m_CurrentIndex{ 0 };
			// 타이머가 호출 되기 위해서 배열을 몇 번 순회해야 하는지에 대한 값.
			size_t m_CurrentRotationCount{ 0 };

		public:
			explicit CTimingWheel(size_t SlotInterval) : m_SlotInterval(SlotInterval) {
				m_ThreadHandle = std::thread(&CTimingWheel::Update, this);
			};
			~CTimingWheel() {
				if (m_ThreadHandle.joinable()) {
					m_ThreadHandle.join();
				}
			}

		private:
			void Update() {
				using namespace std::chrono;

				auto StartPoint(system_clock::now());
				auto LastUpdatedTime(StartPoint);

				while (m_TimingWheelThreadRunState) {
					auto CurrentTime(system_clock::now());
					// 슬롯 시간 간격보다 더 많은 시간이 흘렀을 때
					if (auto Interval(duration_cast<milliseconds>(CurrentTime - LastUpdatedTime).count()); Interval >= m_SlotInterval) {
						FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
						// 처리해야할 인덱스를 구함
						auto Index = m_CurrentIndex;
						InterlockedIncrement(&m_CurrentIndex);

						for (auto Timer = m_SlotArray[Index].begin(); Timer != m_SlotArray[Index].end();) {
							if (!Timer->second.m_bIsPause && Timer->second.m_RotataionCount == m_CurrentRotationCount) {
								if (Timer->second.m_Func) {
									Timer->second.m_Func();
								}

								if (Timer->second.m_bIsLoop) {
									// 새로운 인덱스 위치, 순회 횟수를 구해줌.
									auto TotalIndexValue((Index + (Timer->second.m_IntervalTime / m_SlotInterval)));
									auto NewIndex(TotalIndexValue % TIMESLOTCOUNT);
									Timer->second.m_RotataionCount += (TotalIndexValue / TIMESLOTCOUNT);
									m_SlotArray[NewIndex].emplace_back(Timer->first, Timer->second);
								}
								Timer = m_SlotArray[Index].erase(Timer);
							}
							else {
								++Timer;
							}
						}

						// 현재 배열을 몇 번 순회했는지에 대한 값을 구해옴.
						if ((m_CurrentIndex / TIMESLOTCOUNT) >= 1) {
							InterlockedIncrement(&m_CurrentRotationCount);
						}
						InterlockedExchange(&m_CurrentIndex, m_CurrentIndex % TIMESLOTCOUNT);
						LastUpdatedTime = CurrentTime;
					}
				}
			}

		public:
			void Shutdown() {
				InterlockedExchange16(&m_TimingWheelThreadRunState, 0);
			}

		public:
			HANDLE AddNewTimer(DETAIL::TimerInformation& TimerInformation) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				// (현재 인덱스 + (타이머가 실행 될 시간(Millisecond) / 타이머 업데이트 간격)) % 슬롯 최대 크기 -> 타이머가 저장될 인덱스 위치
				auto TotalIndexValue(m_CurrentIndex + (TimerInformation.m_IntervalTime / m_SlotInterval));
				auto SlotIndex(TotalIndexValue % TIMESLOTCOUNT);
				
				if (SlotIndex < m_SlotArray.size()) {
					TimerInformation.m_RotataionCount = (TotalIndexValue / TIMESLOTCOUNT) + m_CurrentRotationCount;
					return m_SlotArray[SlotIndex].emplace_back(CreateEvent(nullptr, false, false, nullptr), TimerInformation).first;
				}
				return INVALID_HANDLE_VALUE;
			}
			HANDLE AddNewTimer(DETAIL::TimerInformation&& TimerInformation) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				auto TotalIndexValue(m_CurrentIndex + (TimerInformation.m_IntervalTime / m_SlotInterval));
				auto SlotIndex(TotalIndexValue % TIMESLOTCOUNT);

				if (SlotIndex < m_SlotArray.size()) {
					TimerInformation.m_RotataionCount = (TotalIndexValue / TIMESLOTCOUNT) + m_CurrentRotationCount;
					return m_SlotArray[SlotIndex].emplace_back(CreateEvent(nullptr, false, false, nullptr), TimerInformation).first;
				}
				return INVALID_HANDLE_VALUE;
			}
			bool RemoveTimer(const HANDLE& Handle) {
				if (Handle == INVALID_HANDLE_VALUE) {
					return false;
				}

				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				bool bIsFind = false;
				for (auto& const Slot : m_SlotArray) {
					Slot.remove_if([&Handle, &bIsFind](const auto& Value) { if (Value.first == Handle) { bIsFind = true; return true; } return false; });
					if (bIsFind) {
						return true;
					}
				}
				return false;
			}
			void PauseTimer(const HANDLE& Handle) {
				if (Handle == INVALID_HANDLE_VALUE) {
					return false;
				}

				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				for (auto& const Slot : m_SlotArray) {
					if (auto& const Timer = std::find_if(Slot.begin(), Slot.end(), [&Handle](const auto& Value) { if (Value.first == Handle) { return true; } return false; }); Timer != Slot.cend()) {
						Timer.m_bIsPause = true;
						break;
					}
				}
			}
			void UnPauseTimer(const HANDLE& Handle) {
				if (Handle == INVALID_HANDLE_VALUE) {
					return false;
				}

				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				for (auto& const Slot : m_SlotArray) {
					if (auto& const Timer = std::find_if(Slot.begin(), Slot.end(), [&Handle](const auto& Value) { if (Value.first == Handle) { return true; } return false; }); Timer != Slot.cend()) {
						Timer.m_bIsPause = false;
						break;
					}
				}
			}
			bool IsPause(const HANDLE& Handle) {
				if (Handle == INVALID_HANDLE_VALUE) {
					return false;
				}

				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
				for (auto& const Slot : m_SlotArray) {
					if (auto& const Timer = std::find_if(Slot.begin(), Slot.end(), [&Handle](const auto& Value) { if (Value.first == Handle) { return true; } return false; }); Timer != Slot.cend()) {
						return Timer.m_bIsPause;
					}
				}
				return false;
			}
			
		};
}
}