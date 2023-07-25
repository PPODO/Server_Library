#pragma once
#include <Functions/MySQL/MySQL.hpp>
#include <vector>
#include <string>

struct CDBTEST_JON : public SERVER::FUNCTIONS::MYSQL::SQL::CBaseTable {
public:
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t> m_ID; 
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string> m_user_name; 

public:
	CDBTEST_JON() : m_ID(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("ID", int32_t())), m_user_name(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<std::string>("user_name", std::string())){};
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
	bool ExecuteQueryForInsert(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(sTableName, "`ID`, `user_name`"));
		return PreparedTableVariables(pStatement);
	}

	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField = {}, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional = SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional()) {
		return ExecuteQueryForSelect(sqlRealConnection, sTableName, listOfOutput, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField = {}, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional = {}) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(sTableName, listOfField, listOfConditional));
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

	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional = SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional()) {
		return ExecuteQueryForSelect(sqlRealConnection, sTableName, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional = {}) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(sTableName, {}, listOfConditional));
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

	bool ExecuteQueryForDelete(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForDelete(sqlRealConnection, sTableName, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	bool ExecuteQueryForDelete(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional = {}) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForDelete(sTableName, listOfConditional));
		if (pStatement && pStatement->executeQuery()) return true;
		return false;
	}

};