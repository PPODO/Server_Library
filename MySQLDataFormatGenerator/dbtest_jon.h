#pragma once
#include <Functions/MySQL/MySQL.hpp>
#include <vector>
#include <string>

static const std::string dbtest_jon_TABLE_NAME = "dbtest_jon";

struct CDBTEST_JON : public SERVER::FUNCTIONS::MYSQL::SQL::CBaseTable {
	static const size_t NUM_OF_COLUMN = 2;
public:
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t> m_ID; 
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string> m_user_name; 

public:
	CDBTEST_JON() : m_ID(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("ID", int32_t())), m_user_name(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_name", std::string())) {};
	CDBTEST_JON(int32_t ID, std::string user_name) : m_ID(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("ID", ID)), m_user_name(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_name", user_name)) {};

protected:
	virtual bool PreparedTableVariables(sql::PreparedStatement* pPreparedStatement) override {
		if (pPreparedStatement) {

			pPreparedStatement->setInt(1, m_ID.m_rawData);
			pPreparedStatement->setString(2, m_user_name.m_rawData);

			CBaseTable::PreparedTableVariables(pPreparedStatement);
			return true;
		}
		return false;
	}

public:
	bool ExecuteQueryForInsert(sql::Connection* sqlRealConnection) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(dbtest_jon_TABLE_NAME, "`ID`, `user_name`", NUM_OF_COLUMN));
		return PreparedTableVariables(pStatement);
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(dbtest_jon_TABLE_NAME, listOfField, listOfConditional)); 
		if (pStatement) {
			auto pResultSet = pStatement->executeQuery();
			listOfOutput.reserve(pResultSet->rowsCount());

			while (pResultSet->next()) {
				CDBTEST_JON rowData;
				for (const auto& sFieldName : listOfField) {
					if (rowData.m_ID.m_sColumnLabel == sFieldName) {
						rowData.m_ID.m_rawData = pResultSet->getInt(sFieldName);
						continue;
					}
					if (rowData.m_user_name.m_sColumnLabel == sFieldName) {
						rowData.m_user_name.m_rawData = pResultSet->getString(sFieldName);
						continue;
					}
				}
				listOfOutput.push_back(rowData);
			}
			return true;
		}
		return false;
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CDBTEST_JON>& listOfOutput) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CDBTEST_JON>& listOfOutput, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(dbtest_jon_TABLE_NAME, {}, listOfConditional)); 
		if (pStatement) {
			auto pResultSet = pStatement->executeQuery();
			listOfOutput.reserve(pResultSet->rowsCount());

			while (pResultSet->next()) {
				CDBTEST_JON rowData;
				rowData.m_ID.m_rawData = pResultSet->getInt("ID"); 
				rowData.m_user_name.m_rawData = pResultSet->getString("user_name"); 
				listOfOutput.push_back(rowData);
			}
			return true;
		}
		return false;
	}

	static bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection) {
		return ExecuteQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForDelete(dbtest_jon_TABLE_NAME, listOfConditional)); 
		if (pStatement && pStatement->executeQuery()) return true;
		return false;
	}

};