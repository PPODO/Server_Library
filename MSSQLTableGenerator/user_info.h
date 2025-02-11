#pragma once
#include <Functions/SQL/SQL.hpp>
#include <Functions/SQL/MSSQL/MSSQL.hpp>
#include <vector>
#include <string>

using namespace SERVER::FUNCTIONS::SQL;
using namespace SERVER::FUNCTIONS::UTIL;

static const std::string user_info_TABLE_NAME = "user_info";

struct Cuser_info : public SERVER::FUNCTIONS::SQL::CBaseTable {
	static const size_t NUM_OF_COLUMN = 3;
public:
	SERVER::FUNCTIONS::SQL::CSQL_ROW<std::string> m_user_id;
	SERVER::FUNCTIONS::SQL::CSQL_ROW<SQL_DATE_STRUCT> m_last_attendance_date;
	SERVER::FUNCTIONS::SQL::CSQL_ROW<SQLSMALLINT> m_attendance_count;


public:
	Cuser_info() : m_user_id(SERVER::FUNCTIONS::SQL::CSQL_ROW<std::string>("user_id", {})), m_last_attendance_date(SERVER::FUNCTIONS::SQL::CSQL_ROW<SQL_DATE_STRUCT>("last_attendance_date", {})), m_attendance_count(SERVER::FUNCTIONS::SQL::CSQL_ROW<SQLSMALLINT>("attendance_count", {})) {};
	Cuser_info(const std::string& user_id, const SQL_DATE_STRUCT& last_attendance_date, const SQLSMALLINT& attendance_count) : m_user_id(SERVER::FUNCTIONS::SQL::CSQL_ROW<std::string>("user_id", user_id)), m_last_attendance_date(SERVER::FUNCTIONS::SQL::CSQL_ROW<SQL_DATE_STRUCT>("last_attendance_date", last_attendance_date)), m_attendance_count(SERVER::FUNCTIONS::SQL::CSQL_ROW<SQLSMALLINT>("attendance_count", attendance_count)) {};

public:
	bool ExecuteQueryForInsert(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection) {
		auto pSTMT = sqlRealConnection->AllocSTMT();
		auto sQuery = CBaseTable::MakeQueryForInsert(user_info_TABLE_NAME, "user_id, last_attendance_date, attendance_count", 3);

		SQLPrepare(*pSTMT, const_cast<SQLWCHAR*>(MBToUni(sQuery).c_str()), sQuery.length());

		SQLBindParameter(*pSTMT, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, m_user_id.m_rawData.length(), 0, (SQLCHAR*)const_cast<char*>(m_user_id.m_rawData.c_str()), 0, NULL);
		SQLBindParameter(*pSTMT, 2, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_TIMESTAMP, sizeof(SQL_DATE_STRUCT), 0, &m_last_attendance_date.m_rawData, 0, NULL);
		SQLBindParameter(*pSTMT, 3, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, sizeof(SQLSMALLINT), 0, &m_attendance_count.m_rawData, 0, NULL);

		auto ret = SQLExecute(*pSTMT);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

		GetMSSQLErrorMessage(SQL_HANDLE_STMT, *pSTMT);
		return false;
	}

	static bool ExecuteQueryForSelect(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection, std::vector<Cuser_info>&listOfOutput, const std::vector<std::string>& listOfField, const std::vector<SERVER::FUNCTIONS::SQL::CQueryWhereConditional>& listOfConditional = {}) {
		auto sQuery = CBaseTable::MakeQueryForSelect(user_info_TABLE_NAME, listOfField, listOfConditional);

		if (auto pSTMT = sqlRealConnection->ExecuteQuery(MBToUni(sQuery))) {
			while (SQLFetch(*pSTMT) == SQL_SUCCESS) {
				Cuser_info newData;
				for(const auto& sFieldName : listOfField) {
					if(newData.m_user_id.m_sColumnLabel == sFieldName) {
						SQLGetData(*pSTMT,1, SQL_C_CHAR, &newData.m_user_id.m_rawData.at(0), newData.m_user_id.m_rawData.length(), NULL);
						continue;
					}
					if(newData.m_last_attendance_date.m_sColumnLabel == sFieldName) {
						SQLGetData(*pSTMT,2, SQL_C_TYPE_DATE, &newData.m_last_attendance_date, sizeof(SQL_DATE_STRUCT), NULL);
						continue;
					}
					if(newData.m_attendance_count.m_sColumnLabel == sFieldName) {
						SQLGetData(*pSTMT,3, SQL_C_SSHORT, &newData.m_attendance_count, sizeof(SQLSMALLINT), NULL);
						continue;
					}
				};
			}
			return listOfOutput.size() > 0 ? true : false;
		}
		return false;
	}

	static bool ExecuteQueryForDelete(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection, const std::vector<SERVER::FUNCTIONS::SQL::CQueryWhereConditional>& listOfConditional = {}) {
		auto sQuery = CBaseTable::MakeQueryForDelete(user_info_TABLE_NAME, listOfConditional); 
		return sqlRealConnection->ExecuteQuery(MBToUni(sQuery)) ? true : false;
	}

};

