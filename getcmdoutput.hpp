//getcmdoutput.hpp
#ifndef GET_CMD_OUTPUT_HPP
#define GET_CMD_OUTPUT_HPP

#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <tstring.hpp>

//function: run the command, and return the output.
//parameter: phandle: a pile handle which you can read the output from it.
//			ppi: pointer to the sub process,
//			type: if true,return the output string, else return '\n' if success place phandle & ppi.
//return value: the first character is '\n', it success;
//			if '1', pipe create fail;  else '2' , create process fail.
TCHAR* getcmdoutput(TCHAR* pcmd, HANDLE* phandle = NULL, PROCESS_INFORMATION* ppi=NULL,bool type = true)
{
	TCHAR* ret = NULL;
	ret = new TCHAR[2];
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead,hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL; //使用系统默认的安全描述符
	sa.bInheritHandle = TRUE; //创建的进程继承句柄

	if (!CreatePipe(&hRead,&hWrite,&sa,0)) //创建匿名管道
	{
		ret = (TCHAR*)&(TEXT("1\0"));
		return ret;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.hStdError = hWrite;
	si.hStdOutput = hWrite; 
	si.wShowWindow = SW_HIDE; 
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	LPTSTR szCmdline = _tcsdup(_tcstrim(pcmd));
	if (!CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		ret = (TCHAR*)&(TEXT("2\0"));
		return ret;
	}
	
	if (!type){
		phandle = &hRead;
		ppi = &pi;
		CloseHandle(hWrite); //关闭管道句柄
		return ret;	
	}
	else{
		WaitForSingleObject(pi.hProcess, INFINITE);	// Wait until child process exits.
		CloseHandle(pi.hProcess);	// Close process and thread handles. 
		CloseHandle(pi.hThread);
		CloseHandle(hWrite); //关闭管道句柄
		char buffer[4096] = { 0 };
		ret = (TCHAR*)&(TEXT("\0\0"));
		DWORD bytesRead = 0;
		while (true)
		{
			if (ReadFile(hRead, buffer, 4095, &bytesRead, NULL) == FALSE) {
				if (ERROR_BROKEN_PIPE == GetLastError())
					break;
				else
					continue;
			}
			if (0 == bytesRead){//the other end of the pipe called the WriteFile function with nNumberOfBytesToWrite set to zero.
				continue;
			}
			else{
				TCHAR* temp = new TCHAR[_tcslen(ret) + bytesRead + 1];
				_tcscpy(temp, ret);
				ret = _tcscat(temp, ctot(buffer));
				Sleep(100);
			}
		}		
		CloseHandle(hRead);
		return ret;
	}
}

#endif