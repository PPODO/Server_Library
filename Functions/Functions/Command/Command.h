#pragma once
#include <Functions/Functions/CriticalSection/CriticalSection.h>
#include <Functions/Functions/Log/Log.h>
#include <thread>
#include <iostream>
#include <functional>
#include <string>
#include <future>
#include <map>

namespace FUNCTIONS {
	namespace COMMAND {
		class CCommand {
		private:
			std::thread m_CommandThread;
			bool m_bIsRunThread;

		private:
			CRITICALSECTION::DETAIL::CCriticalSection m_ActionLocking;

		private:
			std::map<std::string, std::function<void()>> m_Actions;

		public:
			explicit CCommand() noexcept : m_CommandThread(&CCommand::CommandProcess, this), m_bIsRunThread(true) {
				AddNewAction("help", std::bind(&CCommand::PrintAllCommand, this));
			};

			~CCommand() {
				if (m_CommandThread.joinable()) {
					m_CommandThread.join();
				}
			}

		private:
			void CommandProcess() {

				while (m_bIsRunThread) {
					std::string Line; 
					std::getline(std::cin, Line);

					if (std::cin.fail()) {
						LOG::CLog::WriteLog(L"Command : Input Error! Clear Input Stream!");
						std::cin.clear();
						std::cin.ignore(1024);
					}

					size_t SubIndex = 0;
					if ((SubIndex = Line.find("/cd")) != std::string::npos) {
						CRITICALSECTION::CCriticalSectionGuard Lock(&m_ActionLocking);

						std::string Command = Line.substr(4, Line.length() - SubIndex);

						std::map<std::string, std::function<void()>>::const_iterator It;
						if ((It = m_Actions.find(Command)) != m_Actions.cend()) {
							It->second();
						}
						else {
							LOG::CLog::WriteLog(L"Command : The Command Doesn't Exist!");
						}
					}
					else {
						LOG::CLog::WriteLog(L"Command : Incorrect Command!");
					}
				}
			}

		private:
			void PrintAllCommand() {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_ActionLocking);

				for (const auto It : m_Actions) {
					std::cout << "/cd " << It.first << std::endl;
				}
			}

		public:
			template<typename F, typename ...A>
			void AddNewAction(const std::string& Key, F&& Function, A&&... Argc) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_ActionLocking);

				m_Actions.insert(std::make_pair(Key, std::bind(std::forward<F>(Function), std::forward<A>(Argc)...)));
			}

			void DeleteAction(const std::string& Key) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_ActionLocking);

				m_Actions.erase(Key);
			}

			void ModifyAction(const std::string& Key, const std::function<void()>& NewAction) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_ActionLocking);

				m_Actions[Key] = NewAction;
			}

		public:
			void Shutdown() {
				m_bIsRunThread = false;
			}

		};
	}
}