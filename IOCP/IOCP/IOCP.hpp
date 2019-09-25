#pragma once
#include <Network/Session/NetworkSession/ClientSession/ClientSession.h>
#include <Network/Session/NetworkSession/ServerSession/ServerSession.h>
#include <Functions/Functions/CriticalSection/CriticalSection.h>
#include <Functions/Functions/Uncopyable/Uncopyable.h>
#include <Functions/Functions/Log/Log.h>
#include <vector>
#include <thread>

namespace NETWORK {
	namespace NETWORKMODEL {
		namespace IOCP {
			using namespace FUNCTIONS::LOG;

			static const size_t MAX_CLIENT_COUNT = 256;

			template<typename SERVERTYPE, typename CLIENTTYPE>
			class CIOCP : private FUNCTIONS::UNCOPYABLE::CUncopyable {
			private:
				UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				WSADATA m_WinSockData;

			private:
				HANDLE m_hIOCP;

			private:
				std::vector<std::thread> m_WorkerThreads;

			private:
				std::shared_ptr<SESSION::SERVERSESSION::CServerSession> m_ServerSession;
				std::vector<std::shared_ptr<SESSION::CLIENTSESSION::CClientSession>> m_ClientSessions;

			public:
				explicit CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CIOCP();

			private:
				bool InitializeHandle();
				bool InitializeWorkerThread();

			private:
				void ProcessWorkerThread();

			public:
				bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);
				
			};
			
			template<typename SERVERTYPE, typename CLIENTTYPE>
			CIOCP<SERVERTYPE, CLIENTTYPE>::CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_ProtocolType(ProtocolType), m_hIOCP(INVALID_HANDLE_VALUE) {
				m_ClientSessions.resize(MAX_CLIENT_COUNT);

				if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
					CLog::WriteLog(L"");
					std::abort();
				}
			}

			template<typename SERVERTYPE, typename CLIENTTYPE>
			CIOCP<SERVERTYPE, CLIENTTYPE>::~CIOCP() {
				m_ClientSessions.clear();

				for (auto& Iterator : m_WorkerThreads) {
					if (Iterator.joinable()) {
						PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
						Iterator.join();
					}
				}
				m_WorkerThreads.clear();

				if (m_hIOCP || m_hIOCP != INVALID_HANDLE_VALUE) {
					CloseHandle(m_hIOCP);
					m_hIOCP = INVALID_HANDLE_VALUE;
				}

				WSACleanup();
			}

			template<typename SERVERTYPE, typename CLIENTTYPE>
			bool CIOCP<SERVERTYPE, CLIENTTYPE>::Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
				using namespace SESSION::SERVERSESSION;
				using namespace SESSION::CLIENTSESSION;

				if (!std::is_base_of<CServerSession, SERVERTYPE>() || !std::is_base_of<CClientSession, CLIENTTYPE>()) {
					CLog::WriteLog(L"");
					return false;
				}

				if (!InitializeHandle() || !InitializeWorkerThread()) {
					CLog::WriteLog(L"Initialize IOCP : Failed To Initialize IOCP!");
					return false;
				}
				
				try {
					m_ServerSession = std::make_shared<SERVERTYPE>(m_ProtocolType);

					if (m_ServerSession->Initialize(BindAddress) && UTIL::IOCP::RegisterIOCompletionPort(m_ProtocolType, m_hIOCP, m_ServerSession) && (m_ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP)) {
						for (auto& Iterator : m_ClientSessions) {
							std::shared_ptr<CClientSession> Client = std::make_shared<CLIENTTYPE>(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP);
							if (Client && Client->Initialize(m_ServerSession)) {
								Iterator = Client;
							}
							else {
								return false;
							}
						}
					}
				}
				catch (const std::bad_alloc& Exception) {
					CLog::WriteLog(L"Initialize IOCP : %s!", Exception.what());
					return false;
				}
				return true;
			}

			template<typename SERVERTYPE, typename CLIENTTYPE>
			bool CIOCP<SERVERTYPE, CLIENTTYPE>::InitializeHandle() {
				if ((m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0)) == NULL) {
					CLog::WriteLog(L"Initialize IOCP Handle : Failed To Initialize IOCP Handle!");
					return false;
				}

				return true;
			}

			template<typename SERVERTYPE, typename CLIENTTYPE>
			bool CIOCP<SERVERTYPE, CLIENTTYPE>::InitializeWorkerThread() {
				SYSTEM_INFO SysInfo;
				GetSystemInfo(&SysInfo);

				size_t NumberOfProcessor = SysInfo.dwNumberOfProcessors * 2;
				for (size_t i = 0; i < NumberOfProcessor; i++) {
					m_WorkerThreads.push_back(std::thread(&CIOCP<SERVERTYPE, CLIENTTYPE>::ProcessWorkerThread, this));
				}
				return true;
			}

			template<typename SERVERTYPE, typename CLIENTTYPE>
			void CIOCP<SERVERTYPE, CLIENTTYPE>::ProcessWorkerThread() {
				using namespace UTIL::BASESOCKET;

				DWORD RecvBytes = 0;
				LPOVERLAPPED Overlapped = nullptr;
				void* CompletionKey = nullptr;
				
				while (true) {
					bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), &Overlapped, INFINITE);

					if (!CompletionKey) {
						CLog::WriteLog(L"Shutdown!");
						return;
					}

					if (!Succeed || RecvBytes == 0) {
						if (!Succeed) {

						}
						// Processing Shutdown
					}

					OVERLAPPED_EX* OverlappedEx = reinterpret_cast<OVERLAPPED_EX*>(Overlapped);

					if (OverlappedEx) {
						if (!Succeed || (Succeed && RecvBytes == 0)) {
							if (OverlappedEx->m_IOType == EIOTYPE::EIT_ACCEPT) {
								CLog::WriteLog(L"Accept!");
							}
							else {

							}
							continue;
						}

						switch (OverlappedEx->m_IOType) {
						case EIOTYPE::EIT_READ:

							break;
						case EIOTYPE::EIT_WRITE:

							break;
						default:
							break;
						}
					}
				}
			}

		}
	}

	namespace UTIL {
		namespace IOCP {
			namespace DETAIL {
				inline bool CreateCompletionPort(const ::SOCKET& Socket, const HANDLE& hIOCP, const ULONG_PTR& CompletionPort) {
					return static_cast<bool>(CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), hIOCP, CompletionPort, 0));
				}
			}

			inline bool RegisterIOCompletionPort(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const HANDLE& hIOCP, const std::shared_ptr<SESSION::NETWORKSESSION::CNetworkSession> Session) {
			
				return SOCKET::INVALID_SOCKET_VALUE;
			}
		}
	}
}