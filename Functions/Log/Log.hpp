#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "../CriticalSection/CriticalSection.hpp"

namespace SERVER {
	namespace FUNCTIONS {
		namespace UTIL {
			static std::wstring MBToUni(const std::string& mbString) {
				std::wstring uniString;
				uniString.assign(mbString.begin(), mbString.end());

				return uniString;
			}
		}

		namespace LOG {
			static const size_t MAX_BUFFER_LENGTH = 1024;
			static const size_t MAX_DATETIME_LENGTH = 32;
			static const size_t MAX_DIRECTORY_LENGTH = 50;

			class Log {
			private:
				static CRITICALSECTION::CriticalSection m_lock;
				static TCHAR m_sCurrentFileName[MAX_PATH];
				static size_t m_iCurrentDirectoryLength;

			private:
				static bool Write(const std::wstring& logData) {
					SYSTEMTIME systemTime;
					GetLocalTime(&systemTime);

					TCHAR sCurrentDate[MAX_DATETIME_LENGTH] = { TEXT("\0") };
					TCHAR sDebugLog[MAX_BUFFER_LENGTH] = { TEXT("\0") };

					swprintf_s(sCurrentDate, MAX_DATETIME_LENGTH,
							  TEXT("%d-%d-%d %d:%d:%d"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
	  						   systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

					swprintf_s(m_sCurrentFileName + m_iCurrentDirectoryLength, MAX_PATH, TEXT("LOG_M_%d-%d-%d %d.log"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour);

					std::wfstream fileStream(m_sCurrentFileName, std::ios::app);
					if (!fileStream.is_open()) return false;

					swprintf_s(sDebugLog, MAX_BUFFER_LENGTH, TEXT("[%s] %s\n"), sCurrentDate, logData.c_str());

					fileStream << sDebugLog;
					fileStream.close();
					
					OutputDebugString(sDebugLog);
					std::wcout << sDebugLog;

					return true;
				}

			public:
				static bool WriteLog(LPCTSTR sLog, ...) {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					TCHAR sString[MAX_BUFFER_LENGTH] = { TEXT("\0") };

					va_list argcList;
					va_start(argcList, sLog);
					vswprintf_s(sString, sLog, argcList);
					va_end(argcList);

					return Write(sString);
				}

				static void SetLogFileDirectory(LPCTSTR sDirectory) {
					m_iCurrentDirectoryLength = wcslen(sDirectory);
					wcsncpy_s(m_sCurrentFileName, sDirectory, MAX_DIRECTORY_LENGTH);
				}
			};
		}
	}
}

__declspec(selectany) SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection SERVER::FUNCTIONS::LOG::Log::m_lock(0);
__declspec(selectany) TCHAR SERVER::FUNCTIONS::LOG::Log::m_sCurrentFileName[MAX_PATH];
__declspec(selectany) size_t SERVER::FUNCTIONS::LOG::Log::m_iCurrentDirectoryLength;