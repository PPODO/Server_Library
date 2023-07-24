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
		if(pPreparedStatement) {

			pPreparedStatement->setInt(1, m_ID.m_rawData);
			pPreparedStatement->setString(2, m_user_name.m_rawData);

			CBaseTable::PreparedTableVariables(pPreparedStatement);
			return true;
		}
		return false;
	}

	bool ExecuteQueryForInsert(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(sTableName, "`ID`, `user_name`"));
		return PreparedTableVariables(pStatement);
	}

public:
	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField = {}, const SERVER::FUNCTIONS::MYSQL::SQL::CColumnLabelValuePair& condition = SERVER::FUNCTIONS::MYSQL::SQL::CColumnLabelValuePair()) {
		ExecuteQueryForSelect(sqlRealConnection, sTableName, listOfOutput, listOfField, { condition }, {});	}

	bool ExecuteQueryForSelect(SERVER::FUNCTIONS::MYSQL::CMySQLPool::CSQLRealConnection& sqlRealConnection, const std::string& sTableName, std::vector<CDBTEST_JON>& listOfOutput, const std::vector<std::string>&listOfField = {}, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CColumnLabelValuePair>& listOfCondition = {}, const std::vector<std::string>& listOfConditionOperator = {}) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(sTableName, listOfField, listOfCondition, listOfConditionOperator));
		if(pStatement) {
			auto pResultSet = pStatement->executeQuery();
			listOfOutput.reserve(pResultSet->rowsCount());

			while(pResultSet->next()) {
				CDBTEST_JON rowData;
				if(listOfField.size() > 0) {
					for(const auto& sFieldName : listOfField) {
						if(rowData.m_ID.m_sColumnLabel == sFieldName) {
							rowData.m_ID.m_rawData = pResultSet->getInt(sFieldName);
							continue;
						}
						if(rowData.m_user_name.m_sColumnLabel == sFieldName) {
							rowData.m_user_name.m_rawData = pResultSet->getString(sFieldName);
							continue;
						}
					}
				}
				else {
					rowData.m_ID.m_rawData = pResultSet->getInt("ID"); 
					rowData.m_user_name.m_rawData = pResultSet->getString("user_name"); 
				}
				listOfOutput.push_back(rowData);
			}
			return true;
		}
		return false;
	}

};