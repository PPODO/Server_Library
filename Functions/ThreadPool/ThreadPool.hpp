#pragma once
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/CriticalSection/CriticalSection.hpp>
#include <Functions/CircularLinkedList/CircularLinkedList.hpp>

namespace SERVER {
	namespace FUNCTIONS {
		namespace THREADPOOL {

			class ThreadPool {
				using future_func_type = std::function<void(void*)>;
				using future_list_type = SERVER::FUNCTIONS::CIRCULARLINKEDLIST::CircularLinkedList<std::future<void*>>;
			public:
				ThreadPool(future_func_type&& futureProcessingFunc, size_t iNumOfWorkerThread, const std::chrono::milliseconds& workerThreadDuration = std::chrono::milliseconds(250)) : m_workerThreadDuration(workerThreadDuration), m_bStopAllWorkerThread(false) {
					m_pFutureStockList = std::make_unique<future_list_type>();
					m_pFutureProcessingList = std::make_unique<future_list_type>();

					m_futureProcessingThread = std::thread(&ThreadPool::FutureProcessingThread, this, std::move(futureProcessingFunc));

					m_threadPool.reserve(iNumOfWorkerThread);
					for (size_t i = 0; i < iNumOfWorkerThread; i++)
						m_threadPool.emplace_back(std::thread(&ThreadPool::WorkerThread, this));
				}

				~ThreadPool() {
					m_bStopAllWorkerThread = true;
					m_cvWorkerThread.notify_all();

					for (auto& thread : m_threadPool)
						thread.join();

					m_futureProcessingThread.join();

					ClearAllFutureQueue();
				}

				template<typename Func, typename T>
				bool EnqueueJob(Func&& func, T&& arg) {
					if (m_bStopAllWorkerThread) return false;

					using return_type = typename std::result_of<Func(T)>::type;

					auto job = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Func>(func), std::forward<T>(arg)));
					std::future<return_type> futureResult = job->get_future();

					m_workerThreadMutex.lock();
					m_jobQueue.Push([job]() { (*job)(); });
					m_workerThreadMutex.unlock();

					m_cvWorkerThread.notify_one();

					m_futureStockQueueMutex.lock();
					m_pFutureStockList->push_back(std::move(futureResult));
					m_futureStockQueueMutex.unlock();

					return true;
				}

			private:
				void WorkerThread() {
					while (true) {
						std::unique_lock<std::mutex> lck(m_workerThreadMutex);
						m_cvWorkerThread.wait(lck, [this]() { return !m_jobQueue.IsEmpty() || m_bStopAllWorkerThread; });

						if (m_bStopAllWorkerThread)
							return;

						std::function<void()> job;
						if (m_jobQueue.Pop(job)) {
							lck.unlock();

							job();
						}
					}
				}

				void FutureProcessingThread(future_func_type&& func) {
					while (true) {
						std::unique_lock<std::mutex> lck(m_futureStockQueueMutex);

						if (m_cvFutureProcessingThread.wait_for(lck, m_workerThreadDuration, [this]() { return !m_pFutureStockList->is_empty() || !m_pFutureProcessingList->is_empty(); })) {
							if (m_pFutureProcessingList->is_empty() && !m_pFutureStockList->is_empty())
								m_pFutureProcessingList.swap(m_pFutureStockList);

							lck.unlock();

							for (auto it = m_pFutureProcessingList->begin(); it != m_pFutureProcessingList->end(); ++it) {
								if (it.m_pPtr && (*it)._Is_ready()) {
									func((*it).get());

									m_pFutureProcessingList->erase(it);
								}
							}
						}

						if (m_bStopAllWorkerThread)
							return;
					}
				}

				void ClearAllFutureQueue() {
					for (auto it = m_pFutureProcessingList->begin(); it != m_pFutureProcessingList->end(); ++it)
						m_pFutureProcessingList->erase(it);

					for (auto it = m_pFutureStockList->begin(); it != m_pFutureStockList->end(); ++it)
						m_pFutureStockList->erase(it);
				}

			private:
				const std::chrono::milliseconds m_workerThreadDuration;

				std::thread m_futureProcessingThread;
				std::vector<std::thread> m_threadPool;


				std::condition_variable m_cvWorkerThread;
				std::mutex m_workerThreadMutex;


				SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<std::function<void()>> m_jobQueue;


				std::unique_ptr<future_list_type> m_pFutureStockList;
				std::mutex m_futureStockQueueMutex;


				std::unique_ptr<future_list_type> m_pFutureProcessingList;
				std::condition_variable m_cvFutureProcessingThread;


				bool m_bStopAllWorkerThread;
			};
		}
	}
}