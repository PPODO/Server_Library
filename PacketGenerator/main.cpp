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

struct PROTOCOL {
public:
	const std::string m_ProtocolType;
	std::string m_ProtocolName;
	std::vector<PARAMETER> m_Structures;
	std::vector<PARAMETER> m_Parameters;
	std::vector<std::string> m_Headers;
	std::vector<std::string> m_MessageTypes;

public:
	PROTOCOL(const std::string& ProtocolType, const std::string& ProtocolName) : m_ProtocolType(ProtocolType), m_ProtocolName(ProtocolName) {};

};

int main(int argc, char* argv[]) {
	std::vector<PROTOCOL> Protocols;

	if (argc > 1) {
		std::fstream ProtocolFile(argv[1]);
		if (ProtocolFile.is_open()) {
			std::string ReadedLine;
			bool bIsParameterOpen = false;
			bool bIsMessageTypeOpen = false;
			bool bIsStructureTypeOpen = false;
			bool bIsHeadersOpen = false;

			while (std::getline(ProtocolFile, ReadedLine)) {
				std::string ResultWithoutSpaces;
				for (auto& Iterator : ReadedLine) {
					// 공백 제거
					if (Iterator != '\t' && Iterator != ' ' && Iterator != '\r' && Iterator != '\n') {
						ResultWithoutSpaces.push_back(Iterator);
					}
				}

				// 프로토콜은 열거형이기에 컴마로 구분
				if (ResultWithoutSpaces.find(',') != std::string::npos) {
					// 프로토콜의 시작을 알리는 경우가 아닐 때만
					if (ResultWithoutSpaces.find("VERSION") == std::string::npos) {
						size_t StartPosition = ResultWithoutSpaces.find('_') + 1;
						Protocols.emplace_back(ResultWithoutSpaces.substr(0, ResultWithoutSpaces.length() - 1), ResultWithoutSpaces.substr(StartPosition, ResultWithoutSpaces.length() - StartPosition - 1));
					}
				}
				else {
					// 변수 정의가 시작
					if (ResultWithoutSpaces.find("/*") != std::string::npos) {
						bIsParameterOpen = true;
						continue;
					}

					// 끝
					if (ResultWithoutSpaces.find("*/") != std::string::npos) {
						bIsParameterOpen = false;
						continue;
					}

					// 시작중이라면
					if (bIsParameterOpen) {
						if (ResultWithoutSpaces.find('{') != std::string::npos) {
							bIsMessageTypeOpen = true;
							continue;
						}

						if (ResultWithoutSpaces.find('}') != std::string::npos) {
							bIsMessageTypeOpen = false;
							continue;
						}
						
						if (ResultWithoutSpaces.find('(') != std::string::npos) {
							bIsStructureTypeOpen = true;
							continue;
						}

						if (ResultWithoutSpaces.find(')') != std::string::npos) {
							bIsStructureTypeOpen = false;
							continue;
						}

						if (ResultWithoutSpaces.find('<') != std::string::npos && ResultWithoutSpaces.find('-') == std::string::npos) {
							bIsHeadersOpen = true;
							continue;
						}
						
						if (ResultWithoutSpaces.find('>') != std::string::npos && ResultWithoutSpaces.find('-') == std::string::npos) {
							bIsHeadersOpen = false;
							continue;
						}

						if (bIsMessageTypeOpen) {
							Protocols.back().m_MessageTypes.emplace_back(ResultWithoutSpaces);
						}
						else if (bIsHeadersOpen) {
							Protocols.back().m_Headers.emplace_back(ResultWithoutSpaces);
						}
						else {
							size_t CenterIndex = 0;
							if ((CenterIndex = ResultWithoutSpaces.find('-')) != std::string::npos) {
								size_t ArraySizeIndex = 0;
								uint16_t ArraySize = 0;
								if ((ArraySizeIndex = ResultWithoutSpaces.find('[')) != std::string::npos) {
									ArraySize = std::stoi(ResultWithoutSpaces.substr(ArraySizeIndex + 1, ResultWithoutSpaces.length() - ArraySizeIndex - 1));
								}
								std::string Type = ResultWithoutSpaces.substr(0, CenterIndex);
								std::string Name = ResultWithoutSpaces.substr(CenterIndex + 1, ArraySizeIndex - 5);
								if (bIsStructureTypeOpen) {
									Protocols.back().m_Structures.emplace_back(Type, Name, ArraySize);
								}
								else {
									Protocols.back().m_Parameters.emplace_back(Type, Name, ArraySize);
								}
							}
						}
					}
				}
			}
		}

		for (auto& It : Protocols) {
			std::cout << It.m_ProtocolName << '\n';
			for (auto It2 : It.m_Parameters) {
				std::cout << It2.m_Type << '\t' << It2.m_Name << '\t' << It2.m_ArraySize << std::endl;
			}
			std::cout << std::endl << std::endl;
		}

		std::string SavePath(argv[1]);
		std::string HeaderName;
		for (auto It = SavePath.crbegin(); It != SavePath.crend(); ++It) {
			if (*It != '/' && *It != '\''){
				HeaderName.insert(HeaderName.cbegin(), *It);
				SavePath.pop_back();
				continue;
			}
			break;
		}

		for (const auto& It : Protocols) {
			std::fstream PacketDefineFile(SavePath.substr(0, SavePath.find('/')) + '/' + It.m_ProtocolName + ".h", std::fstream::out);
			
			if (PacketDefineFile.is_open()) {
				PacketDefineFile << "#pragma once\n";
				PacketDefineFile << "#pragma comment(lib, \"Network.lib\")\n";
				PacketDefineFile << "#include \"" << HeaderName << "\"\n";

				for (auto It : It.m_Headers) {
					PacketDefineFile << "#include <" << It << ">\n";
				}

				PacketDefineFile << "#include <Network/Packet/BasePacket.hpp>\n\n";

				if (It.m_MessageTypes.size() != 0) {
					PacketDefineFile << "enum E" << It.m_ProtocolName << "MESSAGETYPE {\n";
					for (auto Iterator = It.m_MessageTypes.cbegin(); Iterator != It.m_MessageTypes.cend(); ++Iterator) {
						PacketDefineFile << '\t' << "EMT_" << (*Iterator);
						if ((Iterator + 1) != It.m_MessageTypes.cend()) {
							PacketDefineFile << ',';
						}
						PacketDefineFile << '\n';
					}
					PacketDefineFile << "};\n\n";
				}

				if (It.m_Structures.size() != 0) {
					PacketDefineFile << "struct " << It.m_ProtocolName << " {\n";
					PacketDefineFile << "\tfriend boost::serialization::access;\n";
					PacketDefineFile << "public:\n";

					for (const auto& ParamIt : It.m_Structures) {
						PacketDefineFile << '\t' << ParamIt.m_Type << "\tm_" << ParamIt.m_Name;
						if (ParamIt.m_ArraySize != 0) {
							PacketDefineFile << "[" << ParamIt.m_ArraySize << "]";
						}
						PacketDefineFile << ";\n";
					}

					PacketDefineFile << "\npublic:\n";
					PacketDefineFile << "\t" << It.m_ProtocolName << "() {};\n";
					PacketDefineFile << "\t" << It.m_ProtocolName << "(";

					for (auto ParamIt = It.m_Structures.cbegin(); ParamIt != It.m_Structures.cend(); ++ParamIt) {
						PacketDefineFile << "const " << ParamIt->m_Type << (ParamIt->m_ArraySize == 0 ? "& " : "* ") << ParamIt->m_Name;
						if ((ParamIt + 1) != It.m_Structures.cend()) {
							PacketDefineFile << ", ";
						}
					}
					PacketDefineFile << ") : ";

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
							PacketDefineFile << "\t\tCopyMemory(m_" << ParamIt.m_Name << ", " << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << ");\n";
						}
					}

					PacketDefineFile << "\t};\n\npublic:\n";

					PacketDefineFile << "\t const " << It.m_ProtocolName << "& operator=(const " << It.m_ProtocolName << "& rhs) {\n";

					for (const auto& ParamIt : It.m_Structures) {
						if (ParamIt.m_ArraySize == 0) {
							PacketDefineFile << "\t\tm_" << ParamIt.m_Name << "= rhs.m_" << ParamIt.m_Name << ";\n";
						}
						else {
							PacketDefineFile << "\t\tCopyMemory(m_" << ParamIt.m_Name << ", rhs.m_" << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << ");\n";
						}
					}

					PacketDefineFile << "\n\t\treturn (*this);\n";
					PacketDefineFile << "\t}\n\n";

					PacketDefineFile << "protected:\n";
					PacketDefineFile << "\ttemplate<typename Archive>\n";
					PacketDefineFile << "\tvoid serialize(Archive& ar, unsigned int Version) {\n";

					for (const auto& ParamIt : It.m_Structures) {
						PacketDefineFile << "\t\tar& m_" << ParamIt.m_Name << ";\n";
					}

					PacketDefineFile << "\t};\n\n";

					PacketDefineFile << "};\n\n";
				}

				PacketDefineFile << "struct C" << It.m_ProtocolName << " : public NETWORK::PACKET::CPacket<C" << It.m_ProtocolName << "> {\n";
				PacketDefineFile << "\tfriend boost::serialization::access;\n";
				PacketDefineFile << "public:\n";

				for (const auto& ParamIt : It.m_Parameters) {
					PacketDefineFile << '\t' << ParamIt.m_Type << "\tm_" << ParamIt.m_Name;
					if (ParamIt.m_ArraySize != 0) {
						PacketDefineFile << "[" << ParamIt.m_ArraySize << "]";
					}
					PacketDefineFile << ";\n";
				}

				PacketDefineFile << "\npublic:\n";
				PacketDefineFile << "\tC" << It.m_ProtocolName << "() : NETWORK::PACKET::CPacket<C" << It.m_ProtocolName << ">(" << It.m_ProtocolType << ") {};\n";
				PacketDefineFile << "\tC" << It.m_ProtocolName << "(const uint8_t& MessageType";

				for (auto ParamIt = It.m_Parameters.cbegin(); ParamIt != It.m_Parameters.cend(); ++ParamIt) {
					PacketDefineFile << ", const " << ParamIt->m_Type << ((ParamIt->m_ArraySize != 0) ? "* " : "& ") << ParamIt->m_Name;
				}

				PacketDefineFile << ") : NETWORK::PACKET::CPacket<C" << It.m_ProtocolName << ">(" << It.m_ProtocolType << ", MessageType)";

				bool bIsCannnotUseMemberInit = false;
				for (auto ParamIt = It.m_Parameters.cbegin(); ParamIt != It.m_Parameters.cend(); ++ParamIt) {
					if (ParamIt->m_ArraySize == 0) {
						PacketDefineFile << ", m_" << ParamIt->m_Name << '(' << ParamIt->m_Name << ")";
					}
					else {
						bIsCannnotUseMemberInit = true;
					}
				}

				if (!bIsCannnotUseMemberInit) {
					PacketDefineFile << " {}; \n\n";
				}
				else {
					PacketDefineFile << " {\n";
					for (const auto& ParamIt : It.m_Parameters) {
						if (ParamIt.m_ArraySize != 0) {
							PacketDefineFile << "\t\tCopyMemory(m_" << ParamIt.m_Name << ", " << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << " * sizeof(" << ParamIt.m_Type << "));\n";
						}
					}
					PacketDefineFile << "\t}; \n\n";
				}

				PacketDefineFile << "public:\n";
				PacketDefineFile << "\t const C" << It.m_ProtocolName << "& operator=(const C" << It.m_ProtocolName << "& rhs) {\n";
				PacketDefineFile << "\t\tm_MessageType = rhs.m_MessageType;\n";

				for (const auto& ParamIt : It.m_Parameters) {
					if (ParamIt.m_ArraySize == 0) {
						PacketDefineFile << "\t\tm_" << ParamIt.m_Name << " = rhs.m_" << ParamIt.m_Name << ";\n";
					}
					else {
						PacketDefineFile << "\t\tCopyMemory(m_" << ParamIt.m_Name << ", rhs.m_" << ParamIt.m_Name << ", " << ParamIt.m_ArraySize << ");\n";
					}
				}

				PacketDefineFile << "\n\t\treturn (*this);\n";
				PacketDefineFile << "\t}\n\n";

				PacketDefineFile << "protected:\n";
				PacketDefineFile << "\ttemplate<typename Archive>\n";
				PacketDefineFile << "\tvoid serialize(Archive& ar, unsigned int Version) {\n";
				PacketDefineFile << "\t\tNETWORK::PACKET::CPacket<C" << It.m_ProtocolName << ">::serialize(ar, Version);\n\n";

				for (const auto& ParamIt : It.m_Parameters) {
					PacketDefineFile << "\t\tar& m_" << ParamIt.m_Name << ";\n";
				}

				PacketDefineFile << "\t}";
				
				PacketDefineFile << "\n\n};";
			}
		}
	}
	return 0;
}