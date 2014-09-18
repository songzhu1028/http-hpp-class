//getcmdoutput.hpp
#ifndef GET_CMD_OUTPUT_HPP
#define GET_CMD_OUTPUT_HPP

#include <windows.h>
#include <iostream>
#include <tchar.h>
TCHAR* _tcstrim(TCHAR* str);
TCHAR* _tcsrev(TCHAR* str);
TCHAR* _tcssub(TCHAR* str,int from,int end);
TCHAR* getcmdoutput(TCHAR* pcmd)
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
		CloseHandle(hWrite); //关闭管道句柄
		DWORD d = GetLastError();
		return ret;
	}
	CloseHandle(hWrite); //关闭管道句柄
	
	WaitForSingleObject(pi.hProcess, INFINITE);
	TCHAR buffer[4096] = {0};
	ret = (TCHAR*)&(TEXT("\n\0"));
	DWORD bytesRead=0;

	while (true)
	{
		if (ReadFile(hRead,buffer,4095,&bytesRead,NULL) == NULL) //读取管道
			break;
		TCHAR* temp = new TCHAR[_tcslen(ret)+bytesRead+1];
		_tcscpy(temp, ret);
		ret=_tcscat(temp, buffer);
		break;
		//UpdateWindow();
		//Sleep(100);
	}
	CloseHandle(hRead);
	return ret;	
}
TCHAR* _tcstrim(TCHAR* str){
	if (str){
		if (str[_tcslen(str) - 1] == ' ')
			return _tcstrim(_tcssub(str, 0, _tcslen(str) - 1));
		else if (str[0] == ' ')
			return _tcstrim(_tcssub(str, 1, _tcslen(str) - 1));
		else
			return str;
	}
	else
		return NULL;
}

TCHAR* _tcssub(TCHAR* str, int from, int size){
	if (str){
		if (0 <= from && size > 0 && from < _tcslen(str)){
			int total = min(from + size, _tcslen(str));
			TCHAR* ret = new TCHAR[total - from + 1];
			_tcsset(ret, '\0');
			for (int i = from; i < total; i++)
				ret[i - from] = str[i];
			ret[total - from] = '\0';
			return ret;
		}
		else return NULL;
	}
	else return NULL;
}
#endif