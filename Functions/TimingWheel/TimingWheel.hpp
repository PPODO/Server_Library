#pragma once
#include "../Log/Log.hpp"
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <array>
#include <list>


namespace SERVER {
	namespace FUNCTION {
		namespace TIMINGWHEEL {
			typedef struct TimerDelegate {
			private:
				size_t m_RotationCount;
				std::function<void()> m_CallbackFunction;

			public:
				const size_t m_TimeInterval;

			public:
				TimerDelegate(const size_t TimeInterval) : m_TimeInterval(TimeInterval), m_RotationCount(0) {};
				TimerDelegate(const size_t TimeInterval, const std::function<void()>& CallbackFunction) : m_TimeInterval(TimeInterval), m_RotationCount(0), m_CallbackFunction(CallbackFunction) {};

			public:
				inline void SetRotationCount(const size_t& RotationCount) { m_RotationCount = RotationCount; }

			public:
				template<typename F, typename ...Argc>
				void BindDelegate(F&& Function, Argc&&... argc) {
					m_CallbackFunction = std::bind(std::forward<F>(Function), std::forward<Argc>(argc)...);
				}

			public:
				inline size_t GetRotationCount() const { return m_RotationCount; }
				inline bool CallbackFunctionCalling() { if (m_CallbackFunction) { m_CallbackFunction(); return true; } return false; };

			}TimerDelegate;

			class TimerClass {
				static const size_t MaxWheelSize = 7;
			private:
				volatile size_t m_CurrentIndex;
				volatile size_t m_CurrentRotationCount;
				const size_t m_SlotInterval;

			private:
				bool m_bIsStop;
				unsigned long long m_ElapsedTime;

				std::thread m_TimerThread;
				std::mutex m_TimerListMutex;
				std::array<std::list<TimerDelegate>, MaxWheelSize> m_TimingWheel;
				std::chrono::system_clock::time_point m_StartTime, m_LastIntervalTime;

			private:
				TimerClass(TimerClass& tc) : TimerClass(tc.m_SlotInterval) {}

			public:
				TimerClass(const size_t& SlotInterval) : m_SlotInterval(SlotInterval), m_CurrentIndex(0), m_CurrentRotationCount(0), m_bIsStop(false), m_ElapsedTime(0), m_StartTime(), m_LastIntervalTime() {
					m_StartTime = std::chrono::system_clock::now();

					m_TimerThread = std::thread([this]() {
						while (!m_bIsStop) {
							m_ElapsedTime = ((std::chrono::system_clock::now() - m_StartTime).count() / 10000);

							if (std::chrono::system_clock::now() - m_LastIntervalTime > std::chrono::duration<size_t>(m_SlotInterval / 1000)) {
								for (auto Iterator = m_TimingWheel[m_CurrentIndex].begin(); Iterator != m_TimingWheel[m_CurrentIndex].end();) {
									if (m_CurrentRotationCount == Iterator->GetRotationCount()) {
										if (!Iterator->CallbackFunctionCalling())
											SERVER::FUNCTIONS::LOG::Log::WriteLog(L"");
										m_TimerListMutex.lock();
										Iterator = m_TimingWheel[m_CurrentIndex].erase(Iterator);
										m_TimerListMutex.unlock();
									}
									else {
										Iterator++;
									}
								}

								if (m_CurrentIndex + 1 >= MaxWheelSize) {
									m_CurrentIndex = 0;
									m_CurrentRotationCount++;
								}
								else {
									m_CurrentIndex++;
								}
								m_LastIntervalTime = std::chrono::system_clock::now();
							}
						}
						});
				};

				~TimerClass() {
					m_bIsStop = true;
					m_TimerThread.join();
				}

			public:
				void AddNewTimer(TimerDelegate& ti) {
					size_t NewIndex = (m_CurrentIndex + (ti.m_TimeInterval / m_SlotInterval));
					ti.SetRotationCount(NewIndex >= MaxWheelSize ? m_CurrentRotationCount + 1 : m_CurrentRotationCount);

					m_TimerListMutex.lock();
					m_TimingWheel[NewIndex % MaxWheelSize].push_back(ti);
					m_TimerListMutex.unlock();
				}

			};

		}
	}
}