#pragma once
#include <iostream>
#include <fstream>
#include <Functions/Functions/CriticalSection/CriticalSection.h>

namespace FUNCTIONS {
	namespace LOG {
		static const size_t MAX_BUFFER_LENGTH = 1024;
		static const size_t MAX_DATETIME_LENGTH = 32;

		class CLog {
		private:
			inline static CRITICALSECTION::DETAIL::CCriticalSection m_Lock;

		private:
			static bool Write(const CHAR* LogData) {
				SYSTEMTIME SysTime;

				GetLocalTime(&SysTime);

				CHAR CurrentDate[MAX_DATETIME_LENGTH] = { "\0" };
				CHAR CurrentFileName[MAX_PATH] = { "\0" };
				CHAR DebugLog[MAX_BUFFER_LENGTH] = { "\0" };

				sprintf_s(CurrentDate, MAX_DATETIME_LENGTH, "%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
				sprintf_s(CurrentFileName, MAX_PATH, "LOG_M_%d-%d-%d %d.log", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

				std::fstream FileStream(CurrentFileName, std::ios::app);
				if (!FileStream.is_open()) {
					return false;
				}

				FileStream << '[' << CurrentDate << "] " << LogData << '\n';
				sprintf_s(DebugLog, MAX_BUFFER_LENGTH, "[%s] %s\n", CurrentDate, LogData);

				FileStream.close();

				OutputDebugStringA(DebugLog);
				std::cout << DebugLog;
				return true;
			}

			static bool Write(const TCHAR* LogData) {
				SYSTEMTIME SysTime;

				GetLocalTime(&SysTime);

				TCHAR CurrentDate[MAX_DATETIME_LENGTH] = { L"\0" };
				TCHAR CurrentFileName[MAX_PATH] = { L"\0" };
				TCHAR DebugLog[MAX_BUFFER_LENGTH] = { L"\0" };

				swprintf_s(CurrentDate, MAX_DATETIME_LENGTH, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
				swprintf_s(CurrentFileName, MAX_PATH, L"LOG_U_%d-%d-%d %d.log", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

				std::wfstream FileStream(CurrentFileName, std::ios::app);
				if (!FileStream.is_open()) {
					return false;
				}

				FileStream << L'[' << CurrentDate << L"] " << LogData << '\n';
				swprintf_s(DebugLog, MAX_BUFFER_LENGTH, L"[%s] %s\n", CurrentDate, LogData);

				FileStream.close();

				OutputDebugString(DebugLog);
				std::wcout << DebugLog;
				return true;
			}

		public:
			static bool WriteLog(LPCTSTR Log, ...) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_Lock);

				TCHAR Result[MAX_BUFFER_LENGTH] = { L"\0" };

				va_list ArgcList;
				va_start(ArgcList, Log);
				vswprintf_s(Result, Log, ArgcList);
				va_end(ArgcList);

				return Write(Result);
			}

			static bool WriteLog(const char* const Log, ...) {
				CRITICALSECTION::CCriticalSectionGuard Lock(&m_Lock);

				CHAR Result[MAX_BUFFER_LENGTH] = { "\0" };

				va_list ArgcList;
				va_start(ArgcList, Log);
				vsprintf_s(Result, Log, ArgcList);
				va_end(ArgcList);

				return Write(Result);
			}

		};

	}
}