#include "stdafx.h"
#include <iostream>
#include <string>
#include <tchar.h>
#include <process.h>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <vector>
#include <algorithm>
#include <conio.h>
#include <thread> 

using namespace std;

struct AboutP
{
	int id;
	SIZE_T mem;
	WCHAR name[260];
	AboutP (int id, WCHAR *name)
	{
		this->id = id;
		memcpy(&this->name, name, 260*sizeof(WCHAR));
		HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);
		PROCESS_MEMORY_COUNTERS memCounter;
		
		if (ph == NULL)
			this->mem = 0;
		else
		{
			GetProcessMemoryInfo(ph, &memCounter, sizeof(memCounter));
			this->mem = memCounter.WorkingSetSize/1024;
		}
	}
	bool operator< (const AboutP& rhs) const
	{
		return (mem > rhs.mem) || (mem == rhs.mem && id < rhs.id);
	}
};
vector<AboutP> procs;

BOOL TerminateMyProcess(DWORD dwProcessId)
{
	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL  bInheritHandle = FALSE;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	if (hProcess == NULL)
		return FALSE;

	BOOL result = TerminateProcess(hProcess, 1);
	CloseHandle(hProcess);
	return result;
}
BOOL GetProcessList()
{
	procs.clear();

	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return(FALSE);
	}
	do
	{
		AboutP t(pe32.th32ProcessID, pe32.szExeFile);
		if (t.mem > 0)
			procs.push_back(t);
	} while (Process32Next(hProcessSnap, &pe32));
	sort(procs.begin(), procs.end());
	CloseHandle(hProcessSnap);
	return(TRUE);
}
int q = -1;
void refresh_loop()
{
	while (q == -1)
	{
		GetProcessList();
		system("cls");		
		printf("Process memory monitor is ON. Press SPACE to kill any.\n\n");
		for (int i = 0; i < 20 && i < procs.size(); i++)
			printf("%d\t%d.%d\t%ls\n", i + 1, procs.at(i).mem / 1024, procs.at(i).mem % 1024, procs.at(i).name);
		Sleep(1000);
	}
	
}
int main(void)
{	
	while (true)
	{
		thread monitor(refresh_loop);
		_getch();
		q = 0;
		monitor.join();
		printf("\nSelect process by number: ");
		scanf_s("%d", &q);
		printf((q > 0 && q <= procs.size() && TerminateMyProcess(procs.at(q - 1).id)) ?
			"Success" : "Error");
		q = -1;
		Sleep(1500);
	}
	return 0;
}