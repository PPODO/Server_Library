#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include <string>

namespace SERVER {
	namespace FUNCTIONS {
		namespace MINIDUMP {
			class Minidump {
				typedef bool (WINAPI* MINIDUMPWRITEDUMP)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, 
														 const PMINIDUMP_EXCEPTION_INFORMATION, const PMINIDUMP_USER_STREAM_INFORMATION, 
														 const PMINIDUMP_CALLBACK_INFORMATION);
			private:
				LPTOP_LEVEL_EXCEPTION_FILTER m_pPrevExceptionFilter;

			private:
				static LONG WINAPI UnHandleExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo) {
					LONG iResult = EXCEPTION_CONTINUE_SEARCH;
					HMODULE hDLLHandle = LoadLibrary(TEXT("DBGHELP.DLL"));

					if (hDLLHandle) {
						MINIDUMPWRITEDUMP dump = (MINIDUMPWRITEDUMP)GetProcAddress(hDLLHandle, "MiniDumpWriteDump");

						if (dump) {
							SYSTEMTIME systemTime;
							GetLocalTime(&systemTime);

							TCHAR sDumpFileFormat[MAX_PATH];
							swprintf_s(sDumpFileFormat, MAX_PATH,
								TEXT("%ls\\PID-%d_%d-%d-%d %d_%d_%d.dmp"), GetDumpFilePath().c_str(), GetCurrentProcessId(), systemTime.wYear, systemTime.wMonth, systemTime.wDay,
								systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

							HANDLE hFileHandler = CreateFile(sDumpFileFormat, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
							if (hFileHandler != INVALID_HANDLE_VALUE) {
								_MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;

								dumpExceptionInfo.ThreadId = GetCurrentThreadId();
								dumpExceptionInfo.ExceptionPointers = pExceptionInfo;
								dumpExceptionInfo.ClientPointers = NULL;

								if (dump(GetCurrentProcess(), GetCurrentProcessId(), hFileHandler, MiniDumpNormal, &dumpExceptionInfo, nullptr, nullptr))
									iResult = EXCEPTION_EXECUTE_HANDLER;

								CloseHandle(hFileHandler);
							}
						}
					}
					return iResult;

				}

			public:
				Minidump() {
					SetErrorMode(SEM_FAILCRITICALERRORS);

					m_pPrevExceptionFilter = SetUnhandledExceptionFilter(Minidump::UnHandleExceptionFilter);
				}

				~Minidump() {
					SetUnhandledExceptionFilter(m_pPrevExceptionFilter);
				}

				static std::wstring GetDumpFilePath() {
					std::wstring sDumpPath(MAX_PATH, L'\0');
					GetModuleFileName(NULL, &sDumpPath.at(0), MAX_PATH);

					return sDumpPath.substr(0, sDumpPath.find_last_of(L'\\'));
				}

				static std::string GetDumpFilePathA() {
					return UTIL::UniToMB(GetDumpFilePath());
				}
			};

			static Minidump g_MinidumpInst;
		}
	}
}