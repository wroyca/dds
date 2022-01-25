#include "mainwindow.h"

#include <Windows.h>

#include <QApplication>

auto main(int argc, char *argv[]) -> int
{
  QApplication a(argc, argv);
  MainWindow w;

  // Helper for COM object logs
  {
    AllocConsole();

    auto fpstdin = stdin, fpstdout = stdout, fpstderr = stderr;

    freopen_s(&fpstdin, "CONIN$", "r", stdin);
    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
    freopen_s(&fpstderr, "CONOUT$", "w", stderr);
  }

  w.show();
  return a.exec();
}
