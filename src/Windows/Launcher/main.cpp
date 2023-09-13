#include <Windows.h>

int main(int argc, char *argv[])
{
  STARTUPINFO info = { sizeof(info) };
  PROCESS_INFORMATION processInfo;
  if (CreateProcess(L"updater\\JOIPEngineUpdater.exe", L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
  {
      CloseHandle(processInfo.hProcess); // Cleanup since you don't need this
      CloseHandle(processInfo.hThread); // Cleanup since you don't need this
  }
  return 0;
}
