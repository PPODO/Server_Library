#include <iostream>
#include <Functions/SQL/SQL.hpp>
#include <Functions/SQL/MSSQL/MSSQL.hpp>
#include <fstream>
#include "user_info.h"

const size_t TABLE_NAME_MAX_LENGTH = 100;
const size_t COLUMN_NAME_MAX_LENGTH = 50;
const size_t DATE_TYPE_MAX_LENGTH = 50;

using namespace SERVER::FUNCTIONS::SQL;

void stringToSQLType(const std::string& type, std::string& outputType) {
	if (type == "smallint") {
		outputType = std::string("SQLSMALLINT");
	}
	else if (type == "int") {
		outputType = std::string("SQLINTEGER");
	}
	else if (type == "date") {
		outputType = std::string("SQL_DATE_STRUCT");
	}
	else if (type.find("char")) {
		outputType = std::string("std::string");
	}
}

void stringToCType(const std::string& type, std::string& output) {
	if (type == "SQLSMALLINT") {
		output = std::string("SQL_C_SSHORT");
	}
	else if (type == "SQLINTEGER") {
		output = std::string("SQL_C_SLONG");
	}
	else if (type == "SQL_DATE_STRUCT") {
		output = std::string("SQL_C_TYPE_DATE");
	}
	else if (type == "std::string") {
		output = std::string("SQL_C_CHAR");
	}
}

void stringToCSQLType(const std::string& type, std::string& output) {
	if (type == "SQLSMALLINT") {
		output = std::string("SQL_SMALLINT");
	}
	else if (type == "SQLINTEGER") {
		output = std::string("SQL_INTEGER");
	}
	else if (type == "SQL_DATE_STRUCT") {
		output = std::string("SQL_TIMESTAMP");
	}
	else if (type == "std::string") {
		output = std::string("SQL_CHAR");
	}
}

int main() {
	std::string sServerName;
	std::string sDBName;
	std::string sUserName;
	std::string sPassword;

	std::cout << "Server Name : ";
	std::cin >> sServerName;
	std::cout << "DB Name : ";
	std::cin >> sDBName;
	std::cout << "User Name : ";
	std::cin >> sUserName;
	std::cout << "Password : ";
	std::cin >> sPassword;

	MSSQL::CMSSQLPool pool(sServerName, sDBName, sUserName, sPassword, 2);

	if (auto pConnection = pool.GetConnection()) {
		system("cls");

		if (auto pSTMT = pConnection->ExecuteQuery(L"select TABLE_NAME from information_schema.tables")) {
			std::cout << "-Table List-\n";

			std::vector<SQLCHAR*> m_tableNameList;
			SQLSMALLINT iColIndex = 0;
			while (SQLFetch(*pSTMT) == SQL_SUCCESS) {
				SQLCHAR* sTableName = new SQLCHAR[TABLE_NAME_MAX_LENGTH];
				SQLGetData(*pSTMT, 1, SQL_C_CHAR, sTableName, TABLE_NAME_MAX_LENGTH, NULL);

				std::cout << ++iColIndex << '\t' << sTableName << std::endl;
				m_tableNameList.push_back(sTableName);
			}

			int iTableSelect;
			std::cin >> iTableSelect;

			if ((iTableSelect - 1) < m_tableNameList.size()) {
				std::string sTableName = reinterpret_cast<char*>(m_tableNameList.at((iTableSelect - 1)));
				std::ofstream fileStream(sTableName + ".h");

				fileStream << "#pragma once\n";
				fileStream << "#include <Functions/SQL/SQL.hpp>\n";
				fileStream << "#include <Functions/SQL/MSSQL/MSSQL.hpp>\n";
				fileStream << "#include <vector>\n";
				fileStream << "#include <string>\n\n";

				fileStream << "using namespace SERVER::FUNCTIONS::SQL;\n";
				fileStream << "using namespace SERVER::FUNCTIONS::UTIL;\n\n";

				fileStream << "static const std::string " << sTableName << "_TABLE_NAME = \"" << sTableName << "\";\n\n";

				fileStream << "struct C" << sTableName << " : public SERVER::FUNCTIONS::SQL::CBaseTable {\n";

				SQLINTEGER iNumOfColumn = 0;
				if (auto pSTMT = pConnection->ExecuteQuery(L"SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '" + SERVER::FUNCTIONS::UTIL::MBToUni(sTableName) + std::wstring(L"'"))) {
					while (SQLFetch(*pSTMT) == SQL_SUCCESS)
						SQLGetData(*pSTMT, 1, SQL_C_SLONG, &iNumOfColumn, sizeof(iNumOfColumn), NULL);
				}
				fileStream << "\tstatic const size_t NUM_OF_COLUMN = " << iNumOfColumn << ";\n";

				fileStream << "public:\n";

				std::vector<std::string> columnLists;
				std::vector<std::string> typeLists;

				if (auto pSTMT = pConnection->ExecuteQuery(L"SELECT COLUMN_NAME, DATA_TYPE FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '" + SERVER::FUNCTIONS::UTIL::MBToUni(sTableName) + std::wstring(L"'"))) {
					while (SQLFetch(*pSTMT) == SQL_SUCCESS) {
						SQLCHAR sColumnName[COLUMN_NAME_MAX_LENGTH] = { "\0" };
						SQLCHAR sDataType[DATE_TYPE_MAX_LENGTH] = { "\0" };

						SQLGetData(*pSTMT, 1, SQL_C_CHAR, sColumnName, COLUMN_NAME_MAX_LENGTH, NULL);
						SQLGetData(*pSTMT, 2, SQL_C_CHAR, sDataType, DATE_TYPE_MAX_LENGTH, NULL);

						std::string sOutputType;
						stringToSQLType(reinterpret_cast<char*>(sDataType), sOutputType);
						fileStream << "\tSERVER::FUNCTIONS::SQL::CSQL_ROW<" << sOutputType << "> m_" << sColumnName << ";\n";

						columnLists.emplace_back(reinterpret_cast<char*>(sColumnName));
						typeLists.push_back(sOutputType);
					}
					fileStream << "\n\npublic:\n";
				}


				fileStream << "\tC" << sTableName << "() : ";

				for (size_t i = 0; i < columnLists.size(); i++) {
					fileStream << "m_" << columnLists.at(i) << "(SERVER::FUNCTIONS::SQL::CSQL_ROW<" << typeLists.at(i) << ">(\"" << columnLists.at(i) << "\", {}))";
					if (columnLists.size() > 1 && (i + 1) < columnLists.size())
						fileStream << ", ";
				}
				fileStream << " {};\n";


				fileStream << "\tC" << sTableName << "(";

				for (size_t i = 0; i < columnLists.size(); i++) {
					fileStream << "const " << typeLists.at(i) << "& " << columnLists.at(i);

					if (columnLists.size() > 1 && (i + 1) < columnLists.size())
						fileStream << ", ";
				}

				fileStream << ") : ";

				for (size_t i = 0; i < columnLists.size(); i++) {
					fileStream << "m_" << columnLists.at(i) << "(SERVER::FUNCTIONS::SQL::CSQL_ROW<" << typeLists.at(i) << ">(\"" << columnLists.at(i) << "\", " <<  columnLists.at(i) << "))";
					if (columnLists.size() > 1 && (i + 1) < columnLists.size())
						fileStream << ", ";
				}
				fileStream << " {};\n\n";


				fileStream << "public:\n";
				fileStream << "\tbool ExecuteQueryForInsert(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection) {\n";
				fileStream << "\t\tauto pSTMT = sqlRealConnection->AllocSTMT();\n";
				fileStream << "\t\tauto sQuery = CBaseTable::MakeQueryForInsert(" << sTableName << "_TABLE_NAME, \"";

				for (size_t i = 0; i < columnLists.size(); i++) {
					fileStream << columnLists.at(i);

					if (columnLists.size() > 1 && (i + 1) < columnLists.size())
						fileStream << ", ";
				}
				fileStream << "\", " << iNumOfColumn << ");\n\n";

				fileStream << "\t\tSQLPrepare(*pSTMT, const_cast<SQLWCHAR*>(MBToUni(sQuery).c_str()), sQuery.length());\n\n";

				for (size_t i = 0; i < columnLists.size(); i++) {
					fileStream << "\t\tSQLBindParameter(*pSTMT, " << (i + 1) << ", SQL_PARAM_INPUT, ";
					
					std::string sCTypeOutput, sCSQLTypeOutput;
					stringToCType(typeLists.at(i), sCTypeOutput);
					stringToCSQLType(typeLists.at(i), sCSQLTypeOutput);

					fileStream << sCTypeOutput << ", " << sCSQLTypeOutput << ", ";

					if (typeLists.at(i).compare("std::string") == 0) {
						fileStream << "m_" << columnLists.at(i) << ".m_rawData.length(), 0, ";
						fileStream << "(SQLCHAR*)const_cast<char*>(m_" << columnLists.at(i) << ".m_rawData.c_str())";
					}
					else {
						fileStream << "sizeof(" << typeLists.at(i) << "), 0, &m_" << columnLists.at(i) << ".m_rawData";

					}
					fileStream << ", 0, NULL);\n";
				}
				fileStream << "\n";


				fileStream << "\t\tauto ret = SQLExecute(*pSTMT);\n" << "\t\tif (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)\n" << "\t\treturn true;\n\n";

				fileStream << "\t\tGetMSSQLErrorMessage(SQL_HANDLE_STMT, *pSTMT);\n";
				fileStream << "\t\treturn false;\n";
				fileStream << "\t}\n\n";


				fileStream << "\tstatic bool ExecuteQueryForSelect(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection, std::vector<C" << sTableName <<">&listOfOutput, const std::vector<std::string>& listOfField, const std::vector<SERVER::FUNCTIONS::SQL::CQueryWhereConditional>& listOfConditional = {}) {\n";
				fileStream << "\t\tauto sQuery = CBaseTable::MakeQueryForSelect(" << sTableName << "_TABLE_NAME, listOfField, listOfConditional);\n\n";
				fileStream << "\t\tif (auto pSTMT = sqlRealConnection->ExecuteQuery(MBToUni(sQuery))) {\n";
				fileStream << "\t\t\twhile (SQLFetch(*pSTMT) == SQL_SUCCESS) {\n";
				fileStream << "\t\t\t\tC" << sTableName << " newData;\n";
				fileStream << "\t\t\t\tfor(const auto& sFieldName : listOfField) {\n";

				for (size_t i = 0; i < columnLists.size(); i++) {
					std::string sCTypeOutput, sCSQLTypeOutput;
					stringToCType(typeLists.at(i), sCTypeOutput);
					stringToCSQLType(typeLists.at(i), sCSQLTypeOutput);

					fileStream << "\t\t\t\t\tif(newData.m_" << columnLists.at(i) << ".m_sColumnLabel == sFieldName) {\n";
					fileStream << "\t\t\t\t\t\tSQLGetData(*pSTMT," << (i + 1) << ", " << sCTypeOutput << ", ";

					if (typeLists.at(i).compare("std::string") == 0) {
						fileStream << "&newData.m_" << columnLists.at(i) << ".m_rawData.at(0), newData.m_" << columnLists.at(i) << ".m_rawData.length(), NULL);\n";
					}
					else {
						fileStream << "&newData.m_" << columnLists.at(i) << ", sizeof(" << typeLists.at(i) << "), NULL);\n";
					}

					fileStream << "\t\t\t\t\t\tcontinue;\n";
					fileStream << "\t\t\t\t\t}\n";
				}

				fileStream << "\t\t\t\t};\n";
				fileStream << "\t\t\t}\n";
				fileStream << "\t\t\treturn listOfOutput.size() > 0 ? true : false;\n\t\t}\n";
				fileStream << "\t\treturn false;\n";
				fileStream << "\t}\n\n";


				fileStream << "\tstatic bool ExecuteQueryForDelete(const MSSQL::CMSSQLPool::CSQLRealConnection& sqlRealConnection, const std::vector<SERVER::FUNCTIONS::SQL::CQueryWhereConditional>& listOfConditional = {}) {\n";
				fileStream << "\t\tauto sQuery = CBaseTable::MakeQueryForDelete(" << sTableName << "_TABLE_NAME, listOfConditional); \n";
				fileStream << "\t\treturn sqlRealConnection->ExecuteQuery(MBToUni(sQuery)) ? true : false;\n";
				fileStream << "\t}\n\n";


				fileStream << "};\n\n";
			}


			for (auto& iterator : m_tableNameList)
				delete iterator;
			m_tableNameList.clear();
		}
	}
	return 0;
}