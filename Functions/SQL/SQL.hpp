#pragma once
#include <string>
#include <vector>

namespace SERVER {
	namespace FUNCTIONS {
		namespace SQL {
			struct CQueryWhereConditional {
				enum class ELogicalOperator { NONE, AND, OR, XOR, NOT };

				std::string m_sColumnLabel;
				std::string m_sData;

				const ELogicalOperator m_logicalOperator;

			public:
				CQueryWhereConditional() : m_sColumnLabel(), m_sData(), m_logicalOperator(ELogicalOperator::NONE) {};
				CQueryWhereConditional(const std::string& sColumnLabel, const std::string& sData, const ELogicalOperator logicalOperator = ELogicalOperator::NONE) : m_sColumnLabel(sColumnLabel), m_sData(sData), m_logicalOperator(logicalOperator) {};

			public:
				std::string LogicalOperatorToString() const {
					switch (m_logicalOperator) {
					case ELogicalOperator::NONE:
						return "";
					case ELogicalOperator::AND:
						return "AND";
					case ELogicalOperator::OR:
						return "OR";
					case ELogicalOperator::XOR:
						return "XOR";
					case ELogicalOperator::NOT:
						return "NOT";
					}
				}

			};

			template<typename T>
			struct CSQL_ROW {
				T m_rawData;
				std::string m_sColumnLabel;

			public:
				CSQL_ROW(const std::string& sColumnLabel, const T& rawData) : m_sColumnLabel(sColumnLabel), m_rawData(rawData) {};

			};

			struct CBaseTable {
			protected:
				static std::string MakeQueryForInsert(const std::string& sTableName, const std::string& sColumnLabels, const size_t iNumOFColumn) {
					std::string sQuery;
					sQuery.append("INSERT INTO " + sTableName + " ");
					sQuery.append("(" + sColumnLabels + ")");
					sQuery.append(" VALUES (");
					for (size_t i = 0; i < iNumOFColumn; i++) {
						sQuery.append("?");
						if ((i + 1) < iNumOFColumn)
							sQuery.append(",");
					}
					sQuery.append(")");
					return sQuery;
				}

				static std::string MakeQueryForSelect(const std::string& sTableName, const std::vector<std::string>& listOfField = {}, const std::vector<CQueryWhereConditional>& listOfConditional = {}) {
					std::string sQuery;
					size_t iConditionalListSize = listOfConditional.size();

					sQuery.append("SELECT ");
					if (listOfField.size() <= 0) sQuery.append("*");
					else {
						for (auto iterator = listOfField.begin(); iterator != listOfField.end(); ++iterator) {
							sQuery.append(*iterator);
							if ((iterator + 1) != listOfField.end()) sQuery.append(", ");
						}
					}

					sQuery.append(" FROM " + sTableName);

					if (iConditionalListSize > 0) {
						sQuery.append(" WHERE ");

						for (size_t i = 0; i < iConditionalListSize; i++) {
							sQuery.append("" + listOfConditional[i].m_sColumnLabel + " = '" + listOfConditional[i].m_sData + "'");
							if ((i + 1) != iConditionalListSize) sQuery.append(listOfConditional[i].LogicalOperatorToString());
						}
					}
					return sQuery;
				}

				static std::string MakeQueryForDelete(const std::string& sTableName, const std::vector<CQueryWhereConditional>& listOfConditional = {}) {
					std::string sQuery;
					size_t iConditionalListSize = listOfConditional.size();

					sQuery.append("DELETE FROM " + sTableName);

					if (iConditionalListSize > 0) {
						sQuery.append(" WHERE ");

						for (size_t i = 0; i < iConditionalListSize; i++) {
							sQuery.append("" + listOfConditional[i].m_sColumnLabel + " = \"" + listOfConditional[i].m_sData + "\"");
							if ((i + 1) != iConditionalListSize) sQuery.append(listOfConditional[i].LogicalOperatorToString());
						}
					}
					return sQuery;
				}
			};


		}
	}
}