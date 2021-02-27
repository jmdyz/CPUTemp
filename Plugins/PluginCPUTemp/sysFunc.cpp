#include "sysFunc.h"

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

bool ResourceRelease(const char* filename)
{
    LPCWSTR lpName = NULL;
    HRSRC hrsrc = NULL;
    HMODULE hInstance = GetModuleHandle(L"CPUTemp.dll");
    if (filename == "WinRing0.sys") lpName = MAKEINTRESOURCE(IDR_BIN1);
    if (filename == "WinRing0.vxd") lpName = MAKEINTRESOURCE(IDR_BIN2);
    if (filename == "WinRing0x64.sys") lpName = MAKEINTRESOURCE(IDR_BIN3);
    if (lpName != NULL)
    {
        hrsrc = FindResource(hInstance, lpName, L"bin");
        if (hrsrc != NULL)
        {
            DWORD hrsrc_size = SizeofResource(hInstance, hrsrc);
            if (hrsrc_size > 0)
            {
                HGLOBAL hg = LoadResource(hInstance, hrsrc);
                if (hg != NULL)
                {
                    unsigned char* DATA = (unsigned char*)LockResource(hg);
                    WriteFromStream(DATA, filename, UINT(hrsrc_size));
                }
            }
        }
    }
    return true;
}
