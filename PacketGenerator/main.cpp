#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

struct PARAMETER {
public:
	std::string m_Type;
	std::string m_Name;
	uint16_t m_ArraySize;

public:
	PARAMETER(const std::string& Type, const std::string& Name, const uint16_t& Size) : m_Type(Type), m_Name(Name), m_ArraySize(Size) {};

};

struct STRUCTURE {
public:
	std::string m_StructureName;
	std::vector<PARAMETER> m_Structures;

};

int main() {
	std::vector<STRUCTURE> packetInformation;

	std::ifstream iFile("packet.txt");
	if (iFile.is_open()) {
		STRUCTURE newInfo;
		while (!iFile.eof()) {

			std::string sLine;
			std::getline(iFile, sLine);

			if (sLine.find("VERSION") == 0) {
				newInfo.m_StructureName = sLine.substr(sLine.find('=') + 1);
			}
			else if (sLine.find('{') == 0) {
				while (true) {
					std::string sParameterLine;
					std::getline(iFile, sParameterLine);

					if (sParameterLine.compare("}") != std::string::npos)
						break;

					auto iNameAndTypeInfoIndex = sParameterLine.find('=');
					auto iArraySizeInfoIndex = sParameterLine.find(':');

					auto sName = sParameterLine.substr(0, iNameAndTypeInfoIndex);
					std::string sType;
					size_t iArraySize = 0;
					if (iArraySizeInfoIndex == std::string::npos) {
						sType = sParameterLine.substr(iNameAndTypeInfoIndex + 1);
					}
					else {
						sType = sParameterLine.substr(iNameAndTypeInfoIndex + 1, iArraySizeInfoIndex - (iNameAndTypeInfoIndex + 1));
						iArraySize = std::stoi(sParameterLine.substr(iArraySizeInfoIndex + 1));
					}


					PARAMETER newParam(sType, sName, iArraySize);

					newInfo.m_Structures.push_back(newParam);
				}

				packetInformation.push_back(newInfo);
				std::memset(&newInfo, 0, sizeof(STRUCTURE));
			}
		}
	}

	iFile.close();

	for (const auto& It : packetInformation) {
		std::fstream PacketDefineFile(It.m_StructureName + ".h", std::fstream::out);

		if (PacketDefineFile.is_open()) {
			PacketDefineFile << "#pragma once\n";
			PacketDefineFile << "#pragma comment(lib, \"Network.lib\")\n";
			PacketDefineFile << "#include <string.h>\n";

			PacketDefineFile << "#include <Network/Packet/BasePacket.hpp>\n\n";

			PacketDefineFile << "using namespace std;\n";
			PacketDefineFile << "using namespace SERVER::NETWORK::PACKET;\n\n";

			PacketDefineFile << "struct " << It.m_StructureName << " : public Packet<";
			PacketDefineFile << It.m_StructureName << "> {\n";
			PacketDefineFile << "\tfriend boost::serialization::access;\n";
			PacketDefineFile << "public:\n";

			for (auto Structure : It.m_Structures) {
				PacketDefineFile << '\t' << Structure.m_Type << "\tm_" << Structure.m_Name;

				if (Structure.m_ArraySize != 0)
					PacketDefineFile << "[" << Structure.m_ArraySize << "]";
				
				PacketDefineFile << ";\n";
			}


			PacketDefineFile << "\npublic:\n";
			PacketDefineFile << "\t" << It.m_StructureName << "() : Packet(0, 0) {};\n";
			PacketDefineFile << "\t" << It.m_StructureName << "(uint8_t iPacketType, uint32_t iMessageType) : Packet(iPacketType, iMessageType) {};\n";
			PacketDefineFile << "\t" << It.m_StructureName << "(uint8_t iPacketType, uint32_t iMessageType";

			if (It.m_Structures.size() > 0)
				PacketDefineFile << ", ";

			for (auto ParamIt = It.m_Structures.cbegin(); ParamIt != It.m_Structures.cend(); ++ParamIt) {
				PacketDefineFile << "const " << ParamIt->m_Type << (ParamIt->m_ArraySize == 0 ? "& " : "* ") << ParamIt->m_Name;
				if ((ParamIt + 1) != It.m_Structures.cend()) {
					PacketDefineFile << ", ";
				}
			}
			PacketDefineFile << ") : Packet(iPacketType, iMessageType)";

			if (It.m_Structures.size() > 0)
				PacketDefineFile << ", ";

			for (auto ParamIt = It.m_Structures.cbegin(); ParamIt != It.m_Structures.cend(); ++ParamIt) {
				if (ParamIt->m_ArraySize == 0) {
					PacketDefineFile << "m_" << ParamIt->m_Name << "(" << ParamIt->m_Name << ")";
					if ((ParamIt + 1) != It.m_Structures.cend()) {
						PacketDefineFile << ", ";
					}
				}
			}

			PacketDefineFile << " {\n";

			for (auto ParamIt : It.m_Structures) {
				if (ParamIt.m_ArraySize != 0) {
					PacketDefineFile << "\t\tif (" << ParamIt.m_Name << ") {\n";
					PacketDefineFile << "\t\t\tCopyMemory(m_" << ParamIt.m_Name << ", " << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << " * sizeof(" << ParamIt.m_Type << "));\n";
					PacketDefineFile << "\t\t}\n\t\telse {\n";
					PacketDefineFile << "\t\t\tZeroMemory(m_" << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << " * sizeof(" << ParamIt.m_Type << "));\n";
					PacketDefineFile << "\t\t}\n";
				}
			}

			PacketDefineFile << "\t};\n\npublic:\n";

			PacketDefineFile << "\t const " << It.m_StructureName << "& operator=(const " << It.m_StructureName << "& rhs) {\n";

			for (const auto& ParamIt : It.m_Structures) {
				if (ParamIt.m_ArraySize == 0) {
					PacketDefineFile << "\t\tm_" << ParamIt.m_Name << "= rhs.m_" << ParamIt.m_Name << ";\n";
				}
				else {
					PacketDefineFile << "\t\tif (rhs.m_" << ParamIt.m_Name << ") {\n";
					PacketDefineFile << "\t\t\tCopyMemory(m_" << ParamIt.m_Name << ", rhs.m_" << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << " * sizeof(" << ParamIt.m_Type << "));\n";
					PacketDefineFile << "\t\t}\n\t\telse {\n";
					PacketDefineFile << "\t\t\tZeroMemory(m_" << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << " * sizeof(" << ParamIt.m_Type << "));\n";
					PacketDefineFile << "\t\t}\n";
				}
			}

			PacketDefineFile << "\n\t\treturn (*this);\n";
			PacketDefineFile << "\t}\n\n";

			PacketDefineFile << "protected:\n";
			PacketDefineFile << "\ttemplate<typename Archive>\n";
			PacketDefineFile << "\tvoid serialize(Archive& ar, unsigned int Version) {\n";
			PacketDefineFile << "\t\tar& m_iMessageType;\n\n";

			for (const auto& ParamIt : It.m_Structures) {
				PacketDefineFile << "\t\tar& m_" << ParamIt.m_Name << ";\n";
			}

			PacketDefineFile << "\t};\n\n";

			PacketDefineFile << "};\n\n";
		}
	}
	return 0;
}