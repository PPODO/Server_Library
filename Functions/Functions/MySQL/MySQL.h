#pragma once
#pragma comment(lib, "mysqlcppconn.lib")
#include <Functions/Functions/CriticalSection/CriticalSection.h>
#include <Functions/Functions/Log/Log.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <functional>
#include <string>
#include <vector>
#include <map>

namespace FUNCTIONS {
	namespace UTIL {
		namespace MYSQL {
			namespace DETAIL {
				struct INSERTDATA {
					std::string m_FieldName;
					std::string m_Value;

				public:
					INSERTDATA(const std::string& FieldName, int16_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, int32_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, int64_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, uint16_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, uint32_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, uint64_t Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, float Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, double Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, char Value) : m_FieldName(FieldName), m_Value(std::to_string(Value)) {};
					INSERTDATA(const std::string& FieldName, const std::string& Value) : m_FieldName(FieldName), m_Value(Value) {};

				};
			}

			enum class EQUERYTYPE {
				EQT_SELDB,
				EQT_NEWDATA,
				EQT_UPDATEDATA,
				EQT_DELDATA
			};

			std::string SelectDataBase(const std::string& DBName) {
				return std::string("use `" + DBName + "`");
			}

			std::string InsertNewDataToDB(const std::string& TableName, const std::vector<DETAIL::INSERTDATA>& Datas) {
				std::string FieldResult;
				std::string DataResult;
				for (auto Iterator = Datas.cbegin(); Iterator != Datas.cend(); ++Iterator) {
					FieldResult.append(std::string("`" + Iterator->m_FieldName + "`"));
					DataResult.append(std::string("'" + Iterator->m_Value + "'"));
					if ((Iterator + 1) != Datas.cend()) {
						FieldResult.append(",");
						DataResult.append(",");
					}
				}

				return std::string("insert into `" + TableName + "` (" + FieldResult + ") values (" + DataResult + ")");
			}
		}
	}

	namespace MYSQL {
		class CMySQLPool : FUNCTIONS::UNCOPYABLE::CUncopyable {
			enum class EDBPOOLSTATE { EDBS_FREE, EDBS_USED };
		private:
			const size_t m_MaxConnectionCount;
			const size_t m_AllocConnectionCount;
			const std::string m_URL;
			const std::string m_Username;
			const std::string m_Password;

		private:
			sql::Driver* m_Driver;

		private:
			FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ListLock;
			std::map<sql::Connection*, EDBPOOLSTATE> m_DBConnectionList;

		private:
			sql::Connection* CreateNewConnection() {
				try {
					return m_Driver->connect(m_URL, m_Username, m_Password);
				}
				catch (const sql::SQLException & Exception) {
					FUNCTIONS::LOG::CLog::WriteLog("SQL Error - %s", Exception.what());
					return nullptr;
				}
			}

		public:
			explicit CMySQLPool(const std::string& URL, const std::string& Username, const std::string& Password, const size_t& AllocConnectionCount, const size_t& MaxConnection) : m_URL(URL), m_Username(Username), m_Password(Password), m_Driver(nullptr), m_AllocConnectionCount(AllocConnectionCount), m_MaxConnectionCount(MaxConnection) {
				try {
					m_Driver = get_driver_instance();
				}
				catch (const sql::SQLException & Exception) {
					FUNCTIONS::LOG::CLog::WriteLog("SQL Error - %s", Exception.what());
				}

				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);
				for (size_t i = 0; i < m_AllocConnectionCount; i++) {
					m_DBConnectionList.insert(std::make_pair(CreateNewConnection(), EDBPOOLSTATE::EDBS_FREE));
				}
			}
			~CMySQLPool() {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);

				for (auto& Iterator : m_DBConnectionList) {
					if (Iterator.second == EDBPOOLSTATE::EDBS_FREE) {
						delete Iterator.first;
					}
				}
				m_DBConnectionList.clear();
			}

		public:
			sql::Connection* const GetConnectionFromPoolList() {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);

				for (auto& Iterator : m_DBConnectionList) {
					if (sql::Connection* ReturnValue = nullptr; Iterator.second == EDBPOOLSTATE::EDBS_FREE) {
						Iterator.second = EDBPOOLSTATE::EDBS_USED;
						ReturnValue = Iterator.first;
						return ReturnValue;
					}
				}

				if (sql::Connection* ReturnValue = nullptr; m_DBConnectionList.size() < m_MaxConnectionCount) {
					const size_t AllocCount = (m_DBConnectionList.size() + m_AllocConnectionCount) % m_MaxConnectionCount;
					ReturnValue = m_DBConnectionList.insert(std::make_pair(CreateNewConnection(), EDBPOOLSTATE::EDBS_USED)).first->first;
					for (size_t i = 1; i < AllocCount; i++) {
						m_DBConnectionList.insert(std::make_pair(CreateNewConnection(), EDBPOOLSTATE::EDBS_FREE));
					}
					return ReturnValue;
				}

				FUNCTIONS::LOG::CLog::WriteLog(L"SQL Error - Cannot Get SQL Connection!");
				return nullptr;
			}

			void ReleaseConnectionToPool(sql::Connection* const Connection) {
				if (Connection) {
					FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);

					if (auto const Iterator = m_DBConnectionList.find(Connection); Iterator != m_DBConnectionList.cend() && Iterator->second == EDBPOOLSTATE::EDBS_USED) {
						Iterator->second = EDBPOOLSTATE::EDBS_FREE;
					}
				}
			}

		};

		class CMySQLManager {
		private:
			inline static bool SendQuery(sql::Connection* const Connection, const std::string& Query, const std::function<void(sql::ResultSet* const)>& Processor) {
				try {
					if (sql::Statement* const Statement = Connection->createStatement(); Statement) {
						if (sql::ResultSet* const Result = Statement->executeQuery(Query); Result) {
							if (Processor) {
								Processor(Result);
							}
							delete Result;
						}
						delete Statement;
					}
				}
				catch (const sql::SQLException & Exception) {
					if (Exception.getErrorCode() != 0) {
						FUNCTIONS::LOG::CLog::WriteLog(L"SQL Exception - %d, %s", Exception.getErrorCode(), Exception.what());
						return false;
					}
				}
				return true;
			}

		public:
			static bool ExecuteQuery(sql::Connection* const Connection, const std::string& Query, std::function<void(sql::ResultSet* const)> Processor) {
				return SendQuery(Connection, Query, Processor);
			}
			static bool ExecuteQuery(sql::Connection* const Connection, const std::string& Query) {
				return SendQuery(Connection, Query, nullptr);
			}

		};
	}
}