#include "dds_main_window.h"

#include <QApplication>

auto main (int argc, char *argv[]) -> int
{
  QApplication application(argc, argv);
  MainWindow main_window;
  main_window.show();
  return QApplication::exec();
}
