#pragma once
#pragma comment(lib, "mysqlcppconn.lib")
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/datatype.h>
#include <string>
#include <memory>
#include <vector>
#include "Functions/CriticalSection/CriticalSection.hpp"
#include "Functions/Uncopyable/Uncopyable.hpp"
#include "Functions/Log/Log.hpp"
#include <type_traits>

namespace SERVER {
	namespace FUNCTIONS {
		namespace MYSQL {
			namespace SQL {
				typedef std::pair<std::string, std::string> CColumnLabelValuePair;

				template<typename T>
				struct CSQL_ROW {
					T m_rawData;
					std::string m_sColumnLabel;

				public:
					CSQL_ROW(const std::string& sColumnLabel, const T& rawData) : m_sColumnLabel(sColumnLabel), m_rawData(rawData) {};

				};

				struct CBaseTable {
				protected:
					std::string MakeQueryForInsert(const std::string& sTableName, const std::string& sColumnLabels) {
						std::string sQuery;
						sQuery.append("INSERT INTO `" + sTableName + "` ");
						sQuery.append("(" + sColumnLabels + ")");
						sQuery.append(" VALUES (?, ?)");
						return sQuery;
					}

					std::string MakeQueryForSelect(const std::string& sTableName, const std::vector<std::string>& listOfField = {}, const CColumnLabelValuePair& condition = CColumnLabelValuePair()) {
						MakeQueryForSelect(sTableName, listOfField, { condition }, {});
					}

					std::string MakeQueryForSelect(const std::string& sTableName, const std::vector<std::string>& listOfField = {}, const std::vector<CColumnLabelValuePair>& listOfCondition = {}, const std::vector<std::string>& listOfConditionOperator = {}) {
						std::string sQuery;
						sQuery.append("SELECT ");
						if (listOfField.size() <= 0) sQuery.append("*");
						else {
							for (auto iterator = listOfField.begin(); iterator != listOfField.end(); ++iterator) {
								sQuery.append("`" + *iterator + "`");
								if ((iterator + 1) != listOfField.end()) sQuery.append(", ");
							}
						}
						
						sQuery.append(" FROM `" + sTableName + "`");
						
						if (listOfCondition.size() > 0) {
							sQuery.append(" WHERE ");
						}

						if (listOfCondition.size() > 0) {
							for (size_t i = 0; i < listOfCondition.size(); i++) {
								sQuery.append("`" + listOfCondition[i].first + "` = \"" + listOfCondition[i].second + "\"");
								if ((i + 1) != listOfCondition.size()) sQuery.append(listOfConditionOperator[i]);
							}
						}
						return sQuery;
					}

					virtual bool PreparedTableVariables(sql::PreparedStatement* pPreparedStatement) {
						pPreparedStatement->executeUpdate();

						return true;
					}

				};
			}

			using namespace std;

			class CMySQLPool : FUNCTIONS::UNCOPYABLE::Uncopyable {
			private:
				enum class ECONNECTIONSTATE : uint8_t { E_UNABLE, E_ABLE };

				struct CSQLConnection {
				public:
					ECONNECTIONSTATE m_currentConnectionState;
					sql::Connection* m_pConnection;

				public:
					CSQLConnection(sql::Connection* pNewConnection) : m_currentConnectionState(ECONNECTIONSTATE::E_UNABLE), m_pConnection(pNewConnection) {
						if (pNewConnection) {
							m_currentConnectionState = ECONNECTIONSTATE::E_ABLE;
						}
					}

					void Delete() {
						if (m_pConnection)
							delete m_pConnection;
					}

				};

				class CSQLConnectionGC {
				private:
					CMySQLPool* m_pOwner;
					uint16_t m_iConnectionListIndex;

				public:
					CSQLConnectionGC(CMySQLPool* pOwner, uint16_t iListIndex) : m_pOwner(pOwner), m_iConnectionListIndex(iListIndex) {};

					void operator()(sql::Connection* pConnection) {
						if (pConnection && m_pOwner)
							m_pOwner->m_sqlConnectionList[m_iConnectionListIndex].m_currentConnectionState = ECONNECTIONSTATE::E_ABLE;
					}
				};

			public:
				typedef std::unique_ptr<sql::Connection, CSQLConnectionGC> CSQLRealConnection;

			private:
				string m_sHostName;
				string m_sUserName;
				string m_sPassword;
				uint16_t m_iAllocatedConnectionCount;
				uint16_t m_iMaxConnectionCount;

				sql::Driver* m_pSQLDriver;
				std::vector<CSQLConnection> m_sqlConnectionList;

				FUNCTIONS::CRITICALSECTION::CriticalSection m_ciriticalSection;

			public:
				CMySQLPool(const string& sHostName, const string& sUserName, const string& sPassword, const uint16_t iMaxConnectionCount)
					: m_sHostName(sHostName), m_sUserName(sUserName), m_sPassword(sPassword), m_iMaxConnectionCount(iMaxConnectionCount), m_iAllocatedConnectionCount(iMaxConnectionCount / 2),
					m_pSQLDriver(nullptr) {
					try {
						m_pSQLDriver = get_driver_instance();

						m_sqlConnectionList.reserve(iMaxConnectionCount / 2);

						for (uint16_t i = 0; i < m_iAllocatedConnectionCount; i++)
							m_sqlConnectionList.push_back(CSQLConnection(m_pSQLDriver->connect(sHostName, sUserName, sPassword)));
					}
					catch (sql::SQLException& exception) {
						FUNCTIONS::LOG::Log::WriteLog(L"SQL Error - %s : %d", exception.what(), exception.getErrorCode());
					}
				}

				~CMySQLPool() {

					for (auto& pConnection : m_sqlConnectionList)
						pConnection.Delete();

					m_sqlConnectionList.clear();
				}

			public:
				CSQLRealConnection GetConnection(const string& sSchemaName) {
					FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_ciriticalSection);

					for (size_t i = 0; i < m_sqlConnectionList.size(); i++) {
						if (m_sqlConnectionList[i].m_currentConnectionState == ECONNECTIONSTATE::E_ABLE) {
							try {
								m_sqlConnectionList[i].m_currentConnectionState = ECONNECTIONSTATE::E_UNABLE;
								m_sqlConnectionList[i].m_pConnection->setSchema(sSchemaName);
							}
							catch (const sql::SQLException& exception) {
								FUNCTIONS::LOG::Log::WriteLog(L"SQL Error - Wrong Schema Name!");
								return CSQLRealConnection(nullptr, CSQLConnectionGC(nullptr, 0));
							}
							return CSQLRealConnection(m_sqlConnectionList[i].m_pConnection, CSQLConnectionGC(this, i));
						}
					}

					// if there are no connections left
					if (m_iAllocatedConnectionCount < m_iMaxConnectionCount) {
						if (m_sqlConnectionList.capacity() < m_iMaxConnectionCount)
							m_sqlConnectionList.reserve(m_iMaxConnectionCount);

						try {
							CSQLConnection newConnection(m_pSQLDriver->connect(m_sHostName, m_sUserName, m_sPassword));
							newConnection.m_currentConnectionState = ECONNECTIONSTATE::E_UNABLE;
							m_sqlConnectionList.push_back(newConnection);

							return CSQLRealConnection(newConnection.m_pConnection, CSQLConnectionGC(this, m_iAllocatedConnectionCount++));
						}
						catch (sql::SQLException& exception) {
							FUNCTIONS::LOG::Log::WriteLog(L"SQL Error - %s : %d", exception.what(), exception.getErrorCode());
						}
					}
					return CSQLRealConnection(nullptr, CSQLConnectionGC(nullptr, 0));
				}


			};

		}
	}
}