#pragma once
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <unordered_map>
#include "../UUID/UUID.hpp"
#include "../MemoryPool/MemoryPool.h"

namespace SERVER {
	namespace FUNCTIONS {
		namespace TIMER {
			using TIMERHANDLE = uint16_t;

			struct FTimerInformation : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FTimerInformation> {
			public:
				boost::asio::steady_timer m_timer;
				std::function<void()> m_callback;

				bool m_bIsLoop;
				uint64_t m_iDuration;

			public:
				FTimerInformation(boost::asio::io_context& ioContext, const std::function<void()>& callback, const bool bIsLoop, const uint64_t iDuration) : m_timer(ioContext), m_callback(callback), m_bIsLoop(bIsLoop), m_iDuration(iDuration) {};

			};

			class CTimerSystem {
			public:
				CTimerSystem() : m_bThreadRunState(true) {
					m_timerThread = std::thread([&]() {
						SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Timer IO Context Start!");
						m_ioContext.run();
						});
				}

				~CTimerSystem() {
					m_bThreadRunState = false;
					m_ioContext.stop();

					if (m_timerThread.joinable())
						m_timerThread.join();
				}

			public:
				TIMERHANDLE BindTimer(const std::function<void()>& newCallback, const uint64_t iSeconds, const bool bIsLoop = false) {
					m_csForTimerList.Lock();

					TIMERHANDLE hTimerHandle = SERVER::FUNCTIONS::UUID::UUIDGenerator::Generate();
					auto timerInfo = m_timerList.emplace(hTimerHandle, new FTimerInformation(m_ioContext, newCallback, bIsLoop, iSeconds));
					
					if (timerInfo.second)
						BindTimer(timerInfo.first->second);
					else {
						SERVER::FUNCTIONS::LOG::Log::WriteLog(L"[Error] - Timer Binding Failed!");

						return MAXDWORD;
					}

					m_csForTimerList.UnLock();
					return hTimerHandle;
				}

				void ClearTimer(const TIMERHANDLE hTimerHandle) {
					m_csForTimerList.Lock();

					auto findResult = m_timerList.find(hTimerHandle);

					if (findResult != m_timerList.cend()) {
						findResult->second->m_timer.cancel();
						delete findResult->second;

						m_timerList.erase(findResult);
					}

					m_csForTimerList.UnLock();
				}

			private:
				boost::asio::io_context m_ioContext;

				SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection m_csForTimerList;
				std::unordered_map<TIMERHANDLE, FTimerInformation*> m_timerList;

				std::atomic_bool m_bThreadRunState;
				std::thread m_timerThread;

			private:
				void BindTimer(FTimerInformation* pTimerInformation) {
					if (pTimerInformation) {
						pTimerInformation->m_timer.expires_after(boost::asio::chrono::seconds(pTimerInformation->m_iDuration));
						pTimerInformation->m_timer.async_wait(boost::bind(&CTimerSystem::OnTimerExpiredCallback, this, boost::asio::placeholders::error, pTimerInformation));
					}
					else
						SERVER::FUNCTIONS::LOG::Log::WriteLog(L"[Error] - Timer Binding Failed! Invalid Timer Information Ptr");
				}

				void OnTimerExpiredCallback(const boost::system::error_code& errorCode, FTimerInformation* pTimerInformation) {
					if (pTimerInformation && m_bThreadRunState && !errorCode.failed()) {
						pTimerInformation->m_callback();

						BindTimer(pTimerInformation);
					}
				}

			};
		}
	}
}