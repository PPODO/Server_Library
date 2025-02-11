#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <Windows.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <Functions/Log/Log.hpp>
#include <Functions/CriticalSection/CriticalSection.hpp>

namespace std {
	class sql_exception : public exception {
	private:
		wstring m_sData;

	public:
		sql_exception() {};
		sql_exception(const wstring& what) : m_sData(what) {};
		virtual ~sql_exception() {};

		const wchar_t* what() {
			return m_sData.c_str();
		}

	};

}

namespace SERVER {
	namespace FUNCTIONS {
		namespace UTIL {
			static void GetMSSQLErrorMessage(SQLSMALLINT iHandleType, SQLHANDLE hHandle) {
				SQLWCHAR sErrorMessage[1024] = { L"" };
				SQLWCHAR sSQLState[1024] = { L"" };

				SQLGetDiagRec(iHandleType, hHandle, 1, sSQLState, NULL, sErrorMessage, sizeof(sErrorMessage) / sizeof(SQLWCHAR), NULL);

				LOG::Log::WriteLog(sErrorMessage);
			}

		}

		namespace SQL {
			namespace MSSQL {
				const size_t CONNECTION_STRING_BUFFER_LENGTH = 256;

				class CMSSQLConnection {
					class CSTMTGC {
					public:
						CSTMTGC() {}

						void operator()(SQLHSTMT* hSTMT) {
							if (hSTMT) {
								SQLFreeHandle(SQL_HANDLE_STMT, *hSTMT);
								delete hSTMT;
							}
						}
					};

				public:
					typedef std::unique_ptr<CMSSQLConnection, CSTMTGC> CSQLRealConnection;


				private:
					SQLHDBC m_hDBC;

				public:
					CMSSQLConnection(const SQLHENV hEnv, WCHAR* const sConnectionStringBuffer) {
						SQLRETURN ret;
						ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &m_hDBC);
						if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
							throw std::sql_exception(L"Alloc DBC Failure!");
						ret = SQLSetConnectAttr(m_hDBC, SQL_ATTR_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER>(60), SQL_IS_UINTEGER);
						if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
							throw std::sql_exception(L"SQL Set Connection Attr  Failure!");
						ret = SQLDriverConnect(m_hDBC, NULL, sConnectionStringBuffer, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
						if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
							throw std::sql_exception(L"SQL Connection Failure!");
					}

					~CMSSQLConnection() {
						SQLDisconnect(m_hDBC);
						SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
					}

					bool ExecuteQuery(SQLHSTMT* pSTMT, const std::wstring& sQuery) {
						if (SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, pSTMT) == SQL_SUCCESS) {
							auto ret = SQLExecDirectW(*pSTMT, const_cast<SQLWCHAR*>(sQuery.c_str()), SQL_NTS);
							if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return true;
							else LOG::Log::WriteLog(L"SQL Error - '%d'", ret);

							UTIL::GetMSSQLErrorMessage(SQL_HANDLE_STMT, *pSTMT);
						}
						return false;
					}

					std::unique_ptr<SQLHSTMT, CSTMTGC> AllocSTMT() {
						SQLHSTMT* pSTMT = new SQLHSTMT;

						if (SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, pSTMT) == SQL_SUCCESS) {
							return std::unique_ptr<SQLHSTMT, CSTMTGC>(pSTMT, CSTMTGC());
						}
						return nullptr;
					}

					SQLHDBC GetHDBC() const { return m_hDBC; }

				};

				class CMSSQLPool : FUNCTIONS::UNCOPYABLE::Uncopyable {
				private:
					enum class ECONNECTIONSTATE : uint8_t { E_UNABLE, E_ABLE };

					struct CSQLConnection {
					public:
						ECONNECTIONSTATE m_currentConnectionState;
						CMSSQLConnection* m_pConnection;

					public:
						CSQLConnection(CMSSQLConnection* pNewConnection) : m_currentConnectionState(ECONNECTIONSTATE::E_UNABLE), m_pConnection(pNewConnection) {
							if (pNewConnection) m_currentConnectionState = ECONNECTIONSTATE::E_ABLE;
						}

						void Delete() {
							if (m_pConnection)
								delete m_pConnection;
						}

					};

					class CSQLConnectionGC {
					private:
						CMSSQLPool* m_pOwner;
						uint16_t m_iConnectionListIndex;

					public:
						CSQLConnectionGC(CMSSQLPool* pOwner, uint16_t iListIndex) : m_pOwner(pOwner), m_iConnectionListIndex(iListIndex) {};

						void operator()(CMSSQLConnection* pConnection) {
							if (pConnection && m_pOwner) {
								FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_pOwner->m_ciriticalSection);
								m_pOwner->m_sqlConnectionList[m_iConnectionListIndex].m_currentConnectionState = ECONNECTIONSTATE::E_ABLE;
							}
						}
					};

				public:
					typedef std::unique_ptr<CMSSQLConnection, CSQLConnectionGC> CSQLRealConnection;

				private:
					std::wstring m_sConnectionString;

					std::string m_sHostName;
					std::string m_sUserName;
					std::string m_sPassword;
					uint16_t m_iAllocatedConnectionCount;
					uint16_t m_iMaxConnectionCount;

					SQLHENV m_hENV;
					std::vector<CSQLConnection> m_sqlConnectionList;

					FUNCTIONS::CRITICALSECTION::CriticalSection m_ciriticalSection;

				public:
					CMSSQLPool(const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxConnectionCount)
						: m_sHostName(sHostName), m_sUserName(sUserName), m_sPassword(sPassword), m_iMaxConnectionCount(iMaxConnectionCount), m_iAllocatedConnectionCount(iMaxConnectionCount / 2) {

						m_sConnectionString = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=tcp:" + UTIL::MBToUni(sHostName) + L",1433;DATABASE=" + UTIL::MBToUni(sDBName) + L";UID=" + UTIL::MBToUni(sUserName) + L";PWD=" + UTIL::MBToUni(sPassword) + L";";

						m_sqlConnectionList.reserve(iMaxConnectionCount);

						SQLRETURN ret;
						try {
							ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &m_hENV);
							if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
								throw std::sql_exception(L"Alloc ENV Handle Failure!");

							ret = SQLSetEnvAttr(m_hENV, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
							if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
								throw std::sql_exception(L"Set ENV Attr Failure!");
						}
						catch (std::sql_exception& exception) {
							SERVER::FUNCTIONS::LOG::Log::WriteLog(exception.what());
						}

						for (uint16_t i = 0; i < m_iAllocatedConnectionCount; i++)
							m_sqlConnectionList.push_back(CSQLConnection(new CMSSQLConnection(m_hENV, const_cast<WCHAR*>(m_sConnectionString.c_str()))));
					}

					~CMSSQLPool() {
						for (auto& pConnection : m_sqlConnectionList)
							pConnection.Delete();

						m_sqlConnectionList.clear();

						SQLFreeHandle(SQL_HANDLE_ENV, m_hENV);
					}

				public:
					CSQLRealConnection GetConnection() {
						FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_ciriticalSection);

						for (size_t i = 0; i < m_sqlConnectionList.size(); i++) {
							if (m_sqlConnectionList[i].m_currentConnectionState == ECONNECTIONSTATE::E_ABLE) {
								m_sqlConnectionList[i].m_currentConnectionState = ECONNECTIONSTATE::E_UNABLE;

								return CSQLRealConnection(m_sqlConnectionList[i].m_pConnection, CSQLConnectionGC(this, i));
							}
						}

						// if there are no connections left
						if (m_iAllocatedConnectionCount < m_iMaxConnectionCount) {
							CSQLConnection newConnection(new CMSSQLConnection(m_hENV, const_cast<WCHAR*>(m_sConnectionString.c_str())));
							newConnection.m_currentConnectionState = ECONNECTIONSTATE::E_UNABLE;
							m_sqlConnectionList.push_back(newConnection);

							return CSQLRealConnection(newConnection.m_pConnection, CSQLConnectionGC(this, m_iAllocatedConnectionCount++));
						}
						return CSQLRealConnection(nullptr, CSQLConnectionGC(nullptr, 0));
					}


				};
			}
		}
	}
}