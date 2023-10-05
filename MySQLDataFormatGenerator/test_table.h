#pragma once
#include <Functions/MySQL/MySQL.hpp>
#include <vector>
#include <string>

static const std::string test_table_TABLE_NAME = "test table";

struct CTEST_TABLE : public SERVER::FUNCTIONS::MYSQL::SQL::CBaseTable {
	static const size_t NUM_OF_COLUMN = 1;
public:
	SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t> m_id; 

public:
	CTEST_TABLE() : m_id(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("id", int32_t())) {};
	CTEST_TABLE(int32_t id) : m_id(SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<int32_t>("id", id)) {};

protected:
	virtual bool PreparedTableVariables(sql::PreparedStatement* pPreparedStatement) override {
		if (pPreparedStatement) {

			pPreparedStatement->setInt(1, m_id.m_rawData);

			CBaseTable::PreparedTableVariables(pPreparedStatement);
			return true;
		}
		return false;
	}

public:
	bool ExecuteQueryForInsert(sql::Connection* sqlRealConnection) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(test_table_TABLE_NAME, "`id`", NUM_OF_COLUMN));
		return PreparedTableVariables(pStatement);
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CTEST_TABLE>& listOfOutput, const std::vector<std::string>&listOfField, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CTEST_TABLE>& listOfOutput, const std::vector<std::string>&listOfField, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(test_table_TABLE_NAME, listOfField, listOfConditional)); 
		if (pStatement) {
			auto pResultSet = pStatement->executeQuery();
			listOfOutput.reserve(pResultSet->rowsCount());

			while (pResultSet->next()) {
				CTEST_TABLE rowData;
				for (const auto& sFieldName : listOfField) {
					if (rowData.m_id.m_sColumnLabel == sFieldName) {
						rowData.m_id.m_rawData = pResultSet->getInt(sFieldName);
						continue;
					}
				}
				listOfOutput.push_back(rowData);
			}
			return true;
		}
		return false;
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CTEST_TABLE>& listOfOutput) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CTEST_TABLE>& listOfOutput, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {
		return ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });
	}

	static bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<CTEST_TABLE>& listOfOutput, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(test_table_TABLE_NAME, {}, listOfConditional)); 
		if (pStatement) {
			auto pResultSet = pStatement->executeQuery();
			listOfOutput.reserve(pResultSet->rowsCount());

			while (pResultSet->next()) {
				CTEST_TABLE rowData;
				rowData.m_id.m_rawData = pResultSet->getInt("id"); 
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
		auto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForDelete(test_table_TABLE_NAME, listOfConditional)); 
		if (pStatement && pStatement->executeQuery()) return true;
		return false;
	}

};