#pragma comment(lib, "mysqlcppconn.lib")
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/datatype.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::string GetRawTypeByColumnType(int iCoulmnType) {
	std::string sReturnString;
	switch (iCoulmnType) {
	case sql::DataType::VARCHAR:
		return sReturnString = std::string("std::string");
		break;
	case sql::DataType::INTEGER:
		return sReturnString = std::string("int32_t");
		break;
	case sql::DataType::BIGINT:
		return sReturnString = std::string("int64_t");
		break;
	case sql::DataType::REAL:
		return sReturnString = std::string("float");
		break;
	default:
		return sReturnString;
	}
}

std::string GetSetterNameByColumnType(int iCoulmnType) {
	std::string sReturnString;
	switch (iCoulmnType) {
	case sql::DataType::VARCHAR:
		return sReturnString = std::string("setString");
		break;
	case sql::DataType::INTEGER:
		return sReturnString = std::string("setInt");
		break;
	case sql::DataType::BIGINT:
		return sReturnString = std::string("setInt64");
		break;
	case sql::DataType::REAL:
		return sReturnString = std::string("setDouble");
		break;
	default:
		return sReturnString;
	}
}

std::string GetGetterNameByColumnType(int iCoulmnType) {
	std::string sReturnString;
	switch (iCoulmnType) {
	case sql::DataType::VARCHAR:
		return sReturnString = std::string("getString");
		break;
	case sql::DataType::INTEGER:
		return sReturnString = std::string("getInt");
		break;
	case sql::DataType::BIGINT:
		return sReturnString = std::string("getInt64");
		break;
	case sql::DataType::REAL:
		return sReturnString = std::string("getDouble");
		break;
	default:
		return sReturnString;
	}
}

int main() {
	std::string sHostName, sUserName, sPassword;


	sql::Driver* pDriver = get_driver_instance();
	sql::Connection* pConnector = nullptr;
	try {
		pConnector = pDriver->connect("tcp://localhost:3306", "root", "a2233212.");
	}
	catch (const sql::SQLException& exception) {
		std::cout << "SQL Error - Cannot connect to MySQL! : " << exception.what() << '\n';
 		return -1;
	}

	// show all schmaes
	{
		system("cls");

		auto pDBMetaData = pConnector->getMetaData();
		auto pSchemasResult = pDBMetaData->getSchemas();

		std::cout << "--Schema Names--" << std::endl;
		while (pSchemasResult && pSchemasResult->next())
			std::cout << pSchemasResult->getString(1) << std::endl;

		std::string sSchemaName;
		std::cout << "\nInput Schema : ";
		std::cin >> sSchemaName;

		pConnector->setSchema(sSchemaName);
	}

	auto pStatement = pConnector->createStatement();
	std::string sTableName, sOriginalTableName;
	// show all tables;
	{
		system("cls");

		auto pTableResult = pStatement->executeQuery("show tables");

		std::cout << "--Table Names--" << std::endl;
		while (pTableResult && pTableResult->next())
			std::cout << pTableResult->getString(1) << std::endl;

		std::cin.get();
		std::cout << "\nInput Table : ";
		std::getline(std::cin, sTableName);
		sOriginalTableName = sTableName;
	}
	
	auto pTableResult = pStatement->executeQuery("select* from `" + sTableName + '`');
	auto pTableMetaData = pTableResult->getMetaData();
	
	// remove sapce from table name
	{
		auto findPos = sTableName.find(' ');
		while (findPos != std::string::npos) {
			sTableName.replace(findPos, 1, "_");
			findPos = sTableName.find(' ');
		}
	}

	std::string sAbsuoluteFileLocation;
	// set file location
	{
		system("cls");
		std::cout << "File Location(Absuolute) : ";
		std::cin >> sAbsuoluteFileLocation;
		system("cls");
	}
	
	const std::string CACHED_TABLE_NAME(sTableName + std::string("_TABLE_NAME"));
	std::ofstream fileStream(sAbsuoluteFileLocation + sTableName + ".h", std::ios_base::trunc);
	if (fileStream.is_open()) {
		boost::to_upper(sTableName);
		fileStream << "#pragma once\n";
		fileStream << "#include <Functions/MySQL/MySQL.hpp>\n";
		fileStream << "#include <vector>\n";
		fileStream << "#include <string>\n\n";
		fileStream << "static const std::string " << CACHED_TABLE_NAME << " = \"" << sOriginalTableName << "\";\n\n";
		fileStream << "struct C" << sTableName << " : public SERVER::FUNCTIONS::MYSQL::SQL::CBaseTable {\n";
		fileStream << "\tstatic const size_t NUM_OF_COLUMN = " << pTableMetaData->getColumnCount() << ";\n";
		fileStream << "public:\n";

		std::vector<std::string> typeLists;
		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			sql::SQLString sColumnTypeName = pTableMetaData->getColumnTypeName(i);

			std::cout << "Label : " << sColumnLabel << ", Type Name : " << sColumnTypeName << std::endl;

			std::string sType = "SERVER::FUNCTIONS::MYSQL::SQL::CSQL_ROW<" + GetRawTypeByColumnType(pTableMetaData->getColumnType(i)) + '>';
			typeLists.push_back(sType);
			fileStream << "\t" << sType << " m_" << sColumnLabel << "; \n";
		}

		fileStream << "\npublic:\n";
		fileStream << "\tC" << sTableName << "() : ";
		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			sql::SQLString sColumnTypeName = pTableMetaData->getColumnTypeName(i);

			fileStream << "m_" << sColumnLabel << '(';
			fileStream << typeLists[i - 1] << "(\"" << sColumnLabel << "\", " << GetRawTypeByColumnType(pTableMetaData->getColumnType(i)) << "()))";

			if (i < pTableMetaData->getColumnCount())
				fileStream << ", ";
		}
		fileStream << " {};\n";

		fileStream << "\tC" << sTableName << '(';

		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			sql::SQLString sColumnTypeName = pTableMetaData->getColumnTypeName(i);

			fileStream << GetRawTypeByColumnType(pTableMetaData->getColumnType(i)) << ' ' << sColumnLabel;

			if (i < pTableMetaData->getColumnCount())
				fileStream << ", ";
		}
		fileStream << ") : ";

		size_t iAllLabelNameLength = 0;
		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			sql::SQLString sColumnTypeName = pTableMetaData->getColumnTypeName(i);

			fileStream << "m_" << sColumnLabel << '(';
			fileStream << typeLists[i - 1] << "(\"" << sColumnLabel << "\", " << sColumnLabel << ")" << ')';

			iAllLabelNameLength += sColumnLabel.length();

			if (i < pTableMetaData->getColumnCount())
				fileStream << ", ";
		}

		fileStream << " {};\n\n";
		fileStream << "protected:\n";

		fileStream << "\tvirtual bool PreparedTableVariables(sql::PreparedStatement* pPreparedStatement) override {\n";
		fileStream << "\t\tif (pPreparedStatement) {\n\n";

		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			fileStream << "\t\t\tpPreparedStatement->" << GetSetterNameByColumnType(pTableMetaData->getColumnType(i)) << '(' << i << ", " << "m_" << sColumnLabel.c_str() << ".m_rawData" << ");\n";
		}
		fileStream << "\n\t\t\tCBaseTable::PreparedTableVariables(pPreparedStatement);\n\t\t\treturn true;\n";
		fileStream << "\t\t}\n\t\treturn false;\n";
		fileStream << "\t}\n\n";


		fileStream << "public:\n";

		fileStream << "\tbool ExecuteQueryForInsert(sql::Connection* sqlRealConnection) {\n";
		fileStream << "\t\tauto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForInsert(" << CACHED_TABLE_NAME << ", \"";

		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			fileStream << "`" << sColumnLabel.c_str() << "`";

			if (i < pTableMetaData->getColumnCount())
				fileStream << ", ";
		}
		fileStream << "\", NUM_OF_COLUMN));\n";
		fileStream << "\t\treturn PreparedTableVariables(pStatement);\n";
		fileStream << "\t}\n\n";


		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput, const std::vector<std::string>&listOfField) {\n";
		fileStream << "\t\treturn ExecuteQueryForSelect(sqlRealConnection, listOfOutput, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});\n";
		fileStream << "\t}\n\n";

		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput, const std::vector<std::string>&listOfField, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {\n";
		fileStream << "\t\treturn ExecuteQueryForSelect(sqlRealConnection, listOfOutput, listOfField, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });\n";
		fileStream << "\t}\n\n";


		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput, const std::vector<std::string>&listOfField, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {\n";
		fileStream << "\t\tauto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(" << CACHED_TABLE_NAME << ", listOfField, listOfConditional)); \n";
		fileStream << "\t\tif (pStatement) {\n";
		fileStream << "\t\t\tauto pResultSet = pStatement->executeQuery();\n";
		fileStream << "\t\t\tlistOfOutput.reserve(pResultSet->rowsCount());\n\n";

		fileStream << "\t\t\twhile (pResultSet->next()) {\n";
		fileStream << "\t\t\t\tC" << sTableName << " rowData;\n";

		fileStream << "\t\t\t\tfor (const auto& sFieldName : listOfField) {\n";
		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			fileStream << "\t\t\t\t\tif (rowData.m_" << sColumnLabel << ".m_sColumnLabel == sFieldName) {\n";
			fileStream << "\t\t\t\t\t\trowData.m_" << sColumnLabel << ".m_rawData = pResultSet->" << GetGetterNameByColumnType(pTableMetaData->getColumnType(i)) << "(sFieldName);\n";
			fileStream << "\t\t\t\t\t\tcontinue;\n\t\t\t\t\t}\n";
		}
		fileStream << "\t\t\t\t}\n";

		fileStream << "\t\t\t\tlistOfOutput.push_back(rowData);\n";

		fileStream << "\t\t\t}\n";
		fileStream << "\t\t\treturn true;\n\t\t}\n";
		fileStream << "\t\treturn false;\n\t}\n\n";


		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput) {\n";
		fileStream << "\t\treturn ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});\n";
		fileStream << "\t}\n\n";

		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {\n";
		fileStream << "\t\treturn ExecuteQueryForSelect(sqlRealConnection, listOfOutput, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });\n";
		fileStream << "\t}\n\n";

		fileStream << "\tstatic bool ExecuteQueryForSelect(sql::Connection* sqlRealConnection, std::vector<" << 'C' << sTableName << ">& listOfOutput, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {\n";
		fileStream << "\t\tauto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForSelect(" << CACHED_TABLE_NAME << ", {}, listOfConditional)); \n";
		fileStream << "\t\tif (pStatement) {\n";
		fileStream << "\t\t\tauto pResultSet = pStatement->executeQuery();\n";
		fileStream << "\t\t\tlistOfOutput.reserve(pResultSet->rowsCount());\n\n";

		fileStream << "\t\t\twhile (pResultSet->next()) {\n";
		fileStream << "\t\t\t\tC" << sTableName << " rowData;\n";

		for (size_t i = 1; i <= pTableMetaData->getColumnCount(); i++) {
			sql::SQLString sColumnLabel = pTableMetaData->getColumnLabel(i);
			fileStream << "\t\t\t\trowData.m_" << sColumnLabel << ".m_rawData = pResultSet->" << GetGetterNameByColumnType(pTableMetaData->getColumnType(i)) << "(\"" << sColumnLabel << "\"); \n";
		}

		fileStream << "\t\t\t\tlistOfOutput.push_back(rowData);\n";

		fileStream << "\t\t\t}\n";
		fileStream << "\t\t\treturn true;\n\t\t}\n";
		fileStream << "\t\treturn false;\n\t}\n\n";


		fileStream << "\tstatic bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection) {\n";
		fileStream << "\t\treturn ExecuteQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{});\n";
		fileStream << "\t}\n\n";

		fileStream << "\tstatic bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection, const SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional& conditional) {\n";
		fileStream << "\t\treturn ExecuteQueryForDelete(sqlRealConnection, std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>{ conditional });\n";
		fileStream << "\t}\n\n";

		fileStream << "\tstatic bool ExecuteQueryForDelete(sql::Connection* sqlRealConnection, const std::vector<SERVER::FUNCTIONS::MYSQL::SQL::CQueryWhereConditional>& listOfConditional) {\n";
		fileStream << "\t\tauto pStatement = sqlRealConnection->prepareStatement(CBaseTable::MakeQueryForDelete(" << CACHED_TABLE_NAME << ", listOfConditional)); \n";
		fileStream << "\t\tif (pStatement && pStatement->executeQuery()) return true;\n";
		fileStream << "\t\treturn false;\n";
		fileStream << "\t}\n\n";

		fileStream << "};";

		fileStream.close();
	}
	
	delete pConnector;
	return 0;
}