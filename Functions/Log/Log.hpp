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

			class Log {
			private:
				static CRITICALSECTION::CriticalSection m_lock;

			private:
				static bool Write(const std::wstring& logData) {
					SYSTEMTIME systemTime;
					GetLocalTime(&systemTime);

					TCHAR sCurrentDate[MAX_DATETIME_LENGTH] = { TEXT("\0") };
					TCHAR sCurrentFileName[MAX_PATH] = { TEXT("\0") };
					TCHAR sDebugLog[MAX_BUFFER_LENGTH] = { TEXT("\0") };

					swprintf_s(sCurrentDate, MAX_DATETIME_LENGTH,
							  TEXT("%d-%d-%d %d:%d:%d"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
	  						   systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
					swprintf_s(sCurrentFileName, MAX_PATH, TEXT("LOG_M_%d-%d-%d %d.log"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour);

					std::wfstream fileStream(sCurrentFileName, std::ios::app);
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

					return Write(sLog);
				}
			};
		}
	}
}