#pragma once
#include <Network/Session/NetworkSession/ServerSession/ServerSession.h>
#include <Functions/Functions/CriticalSection/CriticalSection.h>
#include <Functions/Functions/Uncopyable/Uncopyable.h>
#include <Functions/Functions/Log/Log.h>
#include <vector>
#include <thread>

namespace NETWORK {
	namespace UTIL {
		namespace IOCP {
			namespace DETAIL {
				inline bool CreateCompletionPort(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const HANDLE& hIOCP, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session);
			}

			inline bool RegisterIOCompletionPort(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const HANDLE& hIOCP, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session);
		}
	}

	namespace NETWORKMODEL {
		namespace IOCP {
			static const size_t MAX_CLIENT_COUNT = 1;

			class CIOCP : private FUNCTIONS::UNCOPYABLE::CUncopyable {
			private:
				UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				WSADATA m_WinSockData;

			private:
				HANDLE m_hIOCP;
				HANDLE m_hWaitForInitializedThread;

			private:
				std::vector<std::thread> m_WorkerThreads;

			private:
				std::shared_ptr<SESSION::NETWORKSESSION::SERVERSESSION::CServerSession> m_ServerSession;
				std::vector<std::shared_ptr<SESSION::NETWORKSESSION::SERVERSESSION::CServerSession>> m_ClientSessions;

			public:
				explicit CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CIOCP();

			private:
				bool InitializeHandle();
				bool InitializeWorkerThread();

			private:
				template<typename SERVERTYPE>
				bool InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);

			private:
				void ProcessWorkerThread();

			private:
				bool OnIOAccept(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner);
				bool OnIOTryDisconnect(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner);
				bool OnIODisconnect(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner);
				bool OnIORead(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner, const DWORD& RecvBytes);
				//bool OnIOWrite();

			public:
				template<typename SERVERTYPE>
				bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress);

			};

			template<typename SERVERTYPE>
			bool CIOCP::Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
				using namespace SESSION::NETWORKSESSION::SERVERSESSION;

				if (!std::is_base_of<CServerSession, SERVERTYPE>()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP : Type Must Be Derived From CServerSession");
					return false;
				}

				if (!InitializeHandle() || !InitializeWorkerThread()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP : Failed To Initialize IOCP!");
					return false;
				}

				if (!InitializeSession<SERVERTYPE>(BindAddress)) {
					return false;
				}

				return true;
			}

			template<typename SERVERTYPE>
			bool CIOCP::InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
				try {
					m_ServerSession = std::make_shared<SERVERTYPE>(m_ProtocolType);
					if (m_ServerSession->Initialize(BindAddress) && UTIL::IOCP::RegisterIOCompletionPort(m_ProtocolType, m_hIOCP, *m_ServerSession)) {
						if (m_ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
							// 
							for (size_t i = 0; i < MAX_CLIENT_COUNT; i++) {
								std::shared_ptr<NETWORK::SESSION::NETWORKSESSION::SERVERSESSION::CServerSession> Client(std::make_shared<SERVERTYPE>(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP));
								if (Client && Client->Initialize(*m_ServerSession)) {
									m_ClientSessions.emplace_back(Client);
									continue;
								}
								break;
							}
						}
						if (m_ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
							return m_ServerSession->ReadFrom();
						}
						return true;
					}
				}
				catch (const std::bad_alloc& Exception) {
					FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP : %s!", Exception.what());
					return false;
				}
				return true;
			}
		
		}
	}

	namespace UTIL {
		namespace IOCP {
			namespace DETAIL {
				inline bool CreateCompletionPort(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const HANDLE& hIOCP, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session) {
					::SOCKET Socket = UTIL::NETWORKSESSION::SERVERSESSION::GetSocketValue(ProtocolType, Session);
					if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), hIOCP, reinterpret_cast<const ULONG_PTR&>(Session), 0)) {
						if (WSAGetLastError() != ERROR_INVALID_PARAMETER) {
							return false;
						}
					}
					return true;
				}
			}

			inline bool RegisterIOCompletionPort(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const HANDLE& hIOCP, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session) {
				// TransmitFile함수로 소켓을 재활용해도 IOCP에 등록된 정보는 그대로 남음. CreateIoCompletionPort함수에서 ERROR_INVALID_PARAMETER는 이미 등록된 정보라는 뜻이므로
				// 해당 에러는 무시할 수 있도록 처리
				if (ProtocolType & BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
					if (!DETAIL::CreateCompletionPort(BASESOCKET::EPROTOCOLTYPE::EPT_TCP, hIOCP, Session)) { return false; }
				}
				if (ProtocolType & BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
					if (!DETAIL::CreateCompletionPort(BASESOCKET::EPROTOCOLTYPE::EPT_UDP, hIOCP, Session)) { return false; }
				}
				return true;
			}
		}
	}
}