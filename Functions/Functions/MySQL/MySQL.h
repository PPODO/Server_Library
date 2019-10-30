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

namespace std {
	std::string to_string(const char* const _Val) {
		return std::string(_Val);
	}
}

namespace FUNCTIONS {
	namespace EXCEPTION {
		struct vector_range : public std::exception {
		public:
			vector_range(const char* const Message) : std::exception(Message) {};
		};

	}

	namespace UTIL {
		namespace MYSQL {
			namespace DETAIL {
				struct ROW {
				public:
					std::string m_FieldName;
					std::string m_Value;

				public:
					ROW() : m_FieldName(), m_Value() {};
					ROW(const ROW& lvalue) : m_FieldName(lvalue.m_FieldName), m_Value(lvalue.m_Value) {};
					ROW(ROW&& rvalue) : m_FieldName(std::move(rvalue.m_FieldName)), m_Value(std::move(rvalue.m_Value)) {};
					ROW(const std::string& FieldName, int16_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, int32_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, int64_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, uint16_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, uint32_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, uint64_t Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, float Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, double Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, char Value) : m_FieldName(FieldName), m_Value(std::move(std::to_string(Value))) {};
					ROW(const std::string& FieldName, const std::string& Value) : m_FieldName(FieldName), m_Value(Value) {};

				};

				enum class ELOGICALTYPE {
					ELT_NONE,
					ELT_OR,
					ELT_AND
				};

				enum class ECONDITIONTYPE {
					ECT_NONE,
					ECT_LIKE,
					ECT_BETAND,
					ECT_IN,
					ECT_NOTNULL,
					ECT_NULL
				};

				struct CONDITION {
				public:
					ECONDITIONTYPE m_ConditionType;
					std::string m_FieldName;
					std::vector<std::string> m_Values;

				public:
					CONDITION() : m_ConditionType(ECONDITIONTYPE::ECT_NONE) , m_FieldName() {};
					CONDITION(const ECONDITIONTYPE& Type, const std::string& FieldName) : m_ConditionType(Type), m_FieldName(FieldName) {};
					CONDITION(const ECONDITIONTYPE& Type, const std::string& FieldName, std::vector<std::string>& Values) : m_ConditionType(Type), m_FieldName(FieldName), m_Values(std::move(Values)) {};
					CONDITION(const CONDITION& lvalue) : m_ConditionType(lvalue.m_ConditionType), m_FieldName(lvalue.m_FieldName), m_Values(lvalue.m_Values) {};
					CONDITION(CONDITION&& rvalue) : m_ConditionType(rvalue.m_ConditionType), m_FieldName(std::move(rvalue.m_FieldName)), m_Values(std::move(rvalue.m_Values)) {};

				public:
					// can throw exception
					std::string GetConditionResult() {
						switch (m_ConditionType) {
						case ECONDITIONTYPE::ECT_BETAND:
							if (m_Values.size() >= 2) {
								auto It = m_Values.cbegin();
								return std::string('`' + m_FieldName + "` BETWEEN '" + *It + "' AND '" + *(It + 1) + "'");
							}
							throw FUNCTIONS::EXCEPTION::vector_range("the size is less than 2.");
						case ECONDITIONTYPE::ECT_IN:
							if (m_Values.size() >= 2) {
								auto It = m_Values.cbegin();
								return std::string('`' + m_FieldName + "` IN ('" + *It + "', '" + *(It + 1) + "')");
							}
							throw FUNCTIONS::EXCEPTION::vector_range("the size is less than 2.");
						case ECONDITIONTYPE::ECT_LIKE:
							return std::string('`' + m_FieldName + "` LIKE '" + m_Values.front() + "'");
						case ECONDITIONTYPE::ECT_NOTNULL:
							return std::string('`' + m_FieldName + "` NOT IS NULL");
						case ECONDITIONTYPE::ECT_NULL:
							return std::string('`' + m_FieldName + "` IS NULL");
						}
						return std::string();
					}

				};

				struct INSERTDATA {
				public:
					ROW m_Row;

				public:
					INSERTDATA() : m_Row() {};
					INSERTDATA(const ROW& Row) : m_Row(Row) {};
					INSERTDATA(ROW&& Row) : m_Row(std::move(Row)) {};

				};

				struct SELECTDATA {
				public:
					std::vector<std::string> m_FieldNames;
					std::vector<CONDITION> m_Conditions;

				public:
					SELECTDATA() : m_FieldNames(), m_Conditions() {};
					SELECTDATA(const SELECTDATA& lvalue) : m_FieldNames(lvalue.m_FieldNames), m_Conditions(lvalue.m_Conditions) {};
					SELECTDATA(SELECTDATA&& rvalue) : m_FieldNames(std::move(rvalue.m_FieldNames)), m_Conditions(std::move(rvalue.m_Conditions)) {};
					SELECTDATA(std::vector<std::string>& FieldNames, std::vector<CONDITION>& Conditions) : m_FieldNames(std::move(FieldNames)), m_Conditions(std::move(Conditions)) {};
					SELECTDATA(std::vector<std::string>&& FieldNames, std::vector<CONDITION>&& Conditions) : m_FieldNames(FieldNames), m_Conditions(Conditions) {};

				};

				struct UPATEDATA {
				public:
					std::vector<ROW> m_Rows;
					std::vector<CONDITION> m_Conditions;

				public:
					UPATEDATA() {};
					UPATEDATA(const std::vector<ROW>& Rows, const std::vector<CONDITION>& Conditions) : m_Rows(Rows), m_Conditions(Conditions) {};
					UPATEDATA(const UPATEDATA& lvalue) : m_Rows(lvalue.m_Rows), m_Conditions(lvalue.m_Conditions) {};
					UPATEDATA(UPATEDATA&& lvalue) : m_Rows(std::move(lvalue.m_Rows)), m_Conditions(std::move(lvalue.m_Conditions)) {};

				};

				static void MakeConditionList(std::vector<std::string>& ConditionLists) {}

				template<typename TYPE, typename ...TYPES>
				static void MakeConditionList(std::vector<std::string>& ConditionLists, const TYPE& Type, const TYPES&... Args) {
					ConditionLists.emplace_back(std::to_string(Type));
					
					MakeConditionList(ConditionLists, Args...);
				}
			}

			static std::string InsertNewData(const std::string& TableName, const std::vector<DETAIL::INSERTDATA>& Datas) {
				std::string FieldResult;
				std::string DataResult;
				for (auto Iterator = Datas.cbegin(); Iterator != Datas.cend(); ++Iterator) {
					FieldResult.append(std::string("`" + Iterator->m_Row.m_FieldName + "`"));
					DataResult.append(std::string("'" + Iterator->m_Row.m_Value + "'"));
					if ((Iterator + 1) != Datas.cend()) {
						FieldResult.append(",");
						DataResult.append(",");
					}
				}

				return std::string("insert into `" + TableName + "` (" + FieldResult + ") values (" + DataResult + ")");
			}

			static std::string SearchData(const std::string& TableName, const DETAIL::SELECTDATA& Data) {
				std::string FieldResult;
				std::string ConditionResult;
				for (auto FieldIt = Data.m_FieldNames.cbegin(); FieldIt != Data.m_FieldNames.cend(); ++FieldIt) {
					FieldResult.append('`' + *FieldIt + '`');
					if ((FieldIt + 1) != Data.m_FieldNames.cend()) {
						FieldResult.append(", ");
					}
				}

				try {
					if (Data.m_Conditions.size() != 0) {
						ConditionResult.append("WHERE ");
						for (auto CondiIt : Data.m_Conditions) {
							ConditionResult.append(CondiIt.GetConditionResult());
						}
					}
				}
				catch (const std::exception& Exception) {
					FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
					return std::string();
				}

				return std::string("select " + FieldResult + " from " + TableName + ' ' + ConditionResult);
			}

			static std::string UpdateData(const std::string& TableName, const DETAIL::UPATEDATA& Data) {
				std::string UpdateResult;
				std::string ConditionResult;

				for (auto It = Data.m_Rows.cbegin(); It != Data.m_Rows.cend(); ++It) {
					UpdateResult.append('`' + It->m_FieldName + "` = '" + It->m_Value + "'");
					if ((It + 1) != Data.m_Rows.cend()) {
						UpdateResult.append(", ");
					}
				}

				try {
					if (Data.m_Conditions.size() != 0) {
						ConditionResult.append(" WHERE ");
						for (auto CondiIt : Data.m_Conditions) {
							ConditionResult.append(CondiIt.GetConditionResult());
						}
					}
				}
				catch (const std::exception& Exception) {
					FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
					return std::string();
				}

				return std::string("UPDATE `" + TableName + "` SET " + UpdateResult + ConditionResult);
			}

			template<typename ...TYPES>
			static DETAIL::CONDITION MakeCondition(const DETAIL::ECONDITIONTYPE& ConditionType, const std::string& FieldName, const TYPES&... Args) {
				std::vector<std::string> ConditionList;
				DETAIL::MakeConditionList(ConditionList, Args...);

				return DETAIL::CONDITION(ConditionType, FieldName, ConditionList);
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
			sql::Connection* const GetConnectionFromPoolList(const std::string& Schema) {
				FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ListLock);

				for (auto& Iterator : m_DBConnectionList) {
					if (sql::Connection* ReturnValue = nullptr; Iterator.second == EDBPOOLSTATE::EDBS_FREE) {
						Iterator.second = EDBPOOLSTATE::EDBS_USED;
						ReturnValue = Iterator.first;
						ReturnValue->setSchema(Schema);
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