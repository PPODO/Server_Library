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

		// https://d2.naver.com/helloworld/267396 <- �ش� �ּҸ� �����Ͽ� ����.
		/*
			TimingWheel�� �⺻���� ���� ������ ���� �ð� ����(Ÿ�̸� ���� �ֱ�)���� �迭�� ��ȸ�ϸ鼭, �ش� �迭�� ��� Ÿ�̸Ӹ� ó���ϴ� ���
		*/

		template<size_t TIMESLOTCOUNT = 12>
		class CTimingWheel : private UNCOPYABLE::CUncopyable {
		private:
			int16_t m_TimingWheelThreadRunState{ 1 };
			std::thread m_ThreadHandle;

		private:
			// Millisecond, �ش� �ð� ���ݸ��� �迭�� ���� Ÿ�̸Ӹ� ó����.
			const size_t m_SlotInterval;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_SlotArrayLock;
			std::array<std::list<std::pair<HANDLE, DETAIL::TimerInformation>>, TIMESLOTCOUNT> m_SlotArray;

		private:
			// ���� ó���Ǿ�� �ϴ� �ε����� ��ġ
			size_t m_CurrentIndex{ 0 };
			// Ÿ�̸Ӱ� ȣ�� �Ǳ� ���ؼ� �迭�� �� �� ��ȸ�ؾ� �ϴ����� ���� ��.
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
					// ���� �ð� ���ݺ��� �� ���� �ð��� �귶�� ��
					if (auto Interval(duration_cast<milliseconds>(CurrentTime - LastUpdatedTime).count()); Interval >= m_SlotInterval) {
						FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SlotArrayLock);
						// ó���ؾ��� �ε����� ����
						auto Index = m_CurrentIndex;
						InterlockedIncrement(&m_CurrentIndex);

						for (auto Timer = m_SlotArray[Index].begin(); Timer != m_SlotArray[Index].end();) {
							if (!Timer->second.m_bIsPause && Timer->second.m_RotataionCount == m_CurrentRotationCount) {
								if (Timer->second.m_Func) {
									Timer->second.m_Func();
								}

								if (Timer->second.m_bIsLoop) {
									// ���ο� �ε��� ��ġ, ��ȸ Ƚ���� ������.
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

						// ���� �迭�� �� �� ��ȸ�ߴ����� ���� ���� ���ؿ�.
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
				// (���� �ε��� + (Ÿ�̸Ӱ� ���� �� �ð�(Millisecond) / Ÿ�̸� ������Ʈ ����)) % ���� �ִ� ũ�� -> Ÿ�̸Ӱ� ����� �ε��� ��ġ
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