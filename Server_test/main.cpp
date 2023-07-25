#include <iostream>
#include <Functions/MySQL/MySQL.hpp>
#include <cppconn/prepared_statement.h>
#include <cppconn/datatype.h>
#include <MySQLDataFormatGenerator/dbtest_jon.h>

using namespace SERVER::FUNCTIONS::MYSQL;

int main() {
	{
		CMySQLPool::CSQLRealConnection pConnection = mysql.GetConnection("testdb");

		try {
			CDBTEST_JON test_jon(501, "Winter");
			/*test_jon.ExecuteQueryForInsert(pConnection, "dbtest_jon");*/

			std::vector<CDBTEST_JON> listOfOutput;
			test_jon.ExecuteQueryForSelect(pConnection, "dbtest_jon", listOfOutput, {SQL::CQueryWhereConditional("user_name", "Winter", SQL::CQueryWhereConditional::ELogicalOperator::AND),
																						  SQL::CQueryWhereConditional("id", "501") });
 			test_jon.ExecuteQueryForDelete(pConnection, "dbtest_jon", SQL::CQueryWhereConditional("id", "100"));

			for (auto It : listOfOutput) {
				std::cout << It.m_ID.m_sColumnLabel << '\t' << It.m_ID.m_rawData << '\t';
				std::cout << It.m_user_name.m_sColumnLabel << '\t' << It.m_user_name.m_rawData << std::endl;
			}
		}
		catch (sql::SQLException& exception) {
			std::cout << exception.what() << std::endl;
		}
		//state->setInt(1, 100);

	}

	return 0;
}