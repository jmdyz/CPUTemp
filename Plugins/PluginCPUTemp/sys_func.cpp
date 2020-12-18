#include "sys_func.h"

void WriteFromStream(unsigned char* buf, const char* filename, unsigned int size)
{
    FILE* f;
    fopen_s(&f, filename, "wb+");
    if (f)
    {
        fwrite(buf, 1, size, f);
        fclose(f);
    }
}

bool resource_release(const char* filename)
{
    LPCWSTR lpName = NULL;
    HRSRC hrsrc = NULL;
    HMODULE hInstance = GetModuleHandle(L"CPUTemp.dll");
    if (filename == "WinRing0.sys") lpName = MAKEINTRESOURCE(IDR_BIN1);
    if (filename == "WinRing0.vxd") lpName = MAKEINTRESOURCE(IDR_BIN2);
    if (filename == "WinRing0x64.sys") lpName = MAKEINTRESOURCE(IDR_BIN3);
    if (lpName != NULL) hrsrc = FindResource(hInstance, lpName, L"bin");
    if (hrsrc == NULL)
    {
        return false;
    }
    DWORD hrsrc_size = SizeofResource(hInstance, hrsrc);
    if (hrsrc_size <= 0)
    {
        return false;
    }
    HGLOBAL hg = LoadResource(hInstance, hrsrc);
    if (hg == NULL)
    {
        return false;
    }
    unsigned char* DATA = (unsigned char*)LockResource(hg);
    WriteFromStream(DATA, filename, UINT(hrsrc_size));
}
