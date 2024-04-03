#pragma once
#include <Functions/MySQL/MySQL.hpp>
#include <Functions/MemoryPool/MemoryPool.h>
#include <vector>
#include <string>

static const std::string user_info_TABLE_NAME = "user_info";

struct CUSER_INFO : public SERVER::FUNCTIONS::MYSQL::SQL::CBaseTable, public SERVER::FUNCTIONS::MEMORYMANAGER::MemoryManager<CUSER_INFO> {
	static const size_t NUM_OF_COLUMN = 4;
public:
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t> m_token; 
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string> m_user_id; 
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string> m_uesr_pw; 
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string> m_user_name; 

public:
	CUSER_INFO() : m_token(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("token", int32_t())), m_user_id(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_id", std::string())), m_uesr_pw(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("uesr_pw", std::string())), m_user_name(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_name", std::string())) {};
	CUSER_INFO(int32_t token, std::string user_id, std::string uesr_pw, std::string user_name) : m_token(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("token", token)), m_user_id(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_id", user_id)), m_uesr_pw(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("uesr_pw", uesr_pw)), m_user_name(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_name", user_name)) {};

public:
	bool ExecuteQueryForInsert(sql::PreparedStatement* pPreparedStatement) {
		try {
			if (pPreparedStatement) {
				pPreparedStatement->setInt(1, m_token.m_rawData);
				pPreparedStatement->setString(2, m_user_id.m_rawData);
				pPreparedStatement->setString(3, m_uesr_pw.m_rawData);
				pPreparedStatement->setString(4, m_user_name.m_rawData);

				pPreparedStatement->executeUpdate();
				return true;
			}
		}
		catch(const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return false;
		}
		return false;
	}

	bool ExecuteQueryForSelect(sql::PreparedStatement* pPreparedStatement, std::vector<CUSER_INFO>& listOfOutput) {
		try {
			if (pPreparedStatement) {
				auto pResultSet = pPreparedStatement->executeQuery();
				listOfOutput.reserve(pResultSet->rowsCount());

				while (pResultSet->next()) {
					CUSER_INFO rowData;

					rowData.m_token.m_rawData = pResultSet->getInt("token");
					rowData.m_user_id.m_rawData = pResultSet->getString("user_id");
					rowData.m_uesr_pw.m_rawData = pResultSet->getString("uesr_pw");
					rowData.m_user_name.m_rawData = pResultSet->getString("user_name");
					listOfOutput.push_back(rowData);
				}
				return true;
			}
		}
		catch(const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return false;
		}
		return false;
	}

	bool ExecuteQueryForDelete(sql::PreparedStatement* pPreparedStatement) {
		try {
			if (pPreparedStatement) {
				pPreparedStatement->executeQuery();
				return true;
			}
		}
		catch(const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return false;
		}
		return false;
	}


public:
	static sql::PreparedStatement* PreparedQueryForInsert(sql::Connection* sqlRealConnection) {
		try {
			return sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(user_info_TABLE_NAME, "`token`, `user_id`, `uesr_pw`, `user_name`", NUM_OF_COLUMN));
		}
		catch (const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return nullptr;
		}
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection, const std::vector<std::string>&listOfField) {
		return PreparedQueryForSelect(sqlRealConnection, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection, const std::vector<std::string>&listOfField, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return PreparedQueryForSelect(sqlRealConnection, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection, const std::vector<std::string>&listOfField, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		try {
			return sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(user_info_TABLE_NAME, listOfField, listOfConditional)); 
		}
		catch (const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return nullptr;
		}
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection) {
		return PreparedQueryForSelect(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return PreparedQueryForSelect(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static sql::PreparedStatement* PreparedQueryForSelect(sql::Connection* sqlRealConnection, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		try {
			return sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(user_info_TABLE_NAME, const std::vector<std::string>{}, listOfConditional)); 
		}
		catch (const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return nullptr;
		}
	}

	static sql::PreparedStatement* PreparedQueryForDelete(sql::Connection* sqlRealConnection) {
		return PreparedQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static sql::PreparedStatement* PreparedQueryForDelete(sql::Connection* sqlRealConnection, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return PreparedQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static sql::PreparedStatement* PreparedQueryForDelete(sql::Connection* sqlRealConnection, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		try {
			return sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForDelete(user_info_TABLE_NAME, listOfConditional)); 
		}
		catch (const sql::SQLException& exce) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(TEXT("SQL Error - %s", exce.what()));
			return nullptr;
		}
	}

};