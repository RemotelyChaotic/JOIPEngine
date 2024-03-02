#include <Windows.h>
#include <winbase.h>

int main(int argc, char *argv[])
{
  bool bNotFound =
      INVALID_FILE_ATTRIBUTES == GetFileAttributes(L"updater\\JOIPEngineUpdater.exe");
  if(bNotFound)
  {
    STARTUPINFO info = { sizeof(info) };
    PROCESS_INFORMATION processInfo;
    if (CreateProcess(L"bin\\JOIPEngine.exe", L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
    {
        CloseHandle(processInfo.hProcess); // Cleanup since you don't need this
        CloseHandle(processInfo.hThread); // Cleanup since you don't need this
    }
  }
  else
  {
    STARTUPINFO info = { sizeof(info) };
    PROCESS_INFORMATION processInfo;
    if (CreateProcess(L"updater\\JOIPEngineUpdater.exe", L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
    {
        CloseHandle(processInfo.hProcess); // Cleanup since you don't need this
        CloseHandle(processInfo.hThread); // Cleanup since you don't need this
    }
  }
  return 0;
}
