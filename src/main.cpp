#include "mainwindow.h"

#include <Windows.h>

#include <QApplication>
#include <QStyleFactory>

auto main(int argc, char *argv[]) -> int
{
  QApplication a(argc, argv);
  MainWindow w;
  QPalette d;

  // Helper for COM object logs
  {
    AllocConsole();

    auto fpstdin = stdin, fpstdout = stdout, fpstderr = stderr;

    freopen_s(&fpstdin, "CONIN$", "r", stdin);
    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
    freopen_s(&fpstderr, "CONOUT$", "w", stderr);
  }

  d.setColor(QPalette::Window, QColor(53, 53, 53));
  d.setColor(QPalette::WindowText, Qt::white);
  d.setColor(QPalette::Base, QColor(25, 25, 25));
  d.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  d.setColor(QPalette::ToolTipBase, Qt::white);
  d.setColor(QPalette::ToolTipText, Qt::white);
  d.setColor(QPalette::Text, Qt::white);
  d.setColor(QPalette::Button, QColor(53, 53, 53));
  d.setColor(QPalette::ButtonText, Qt::white);
  d.setColor(QPalette::BrightText, Qt::red);
  d.setColor(QPalette::Link, QColor(42, 130, 218));
  d.setColor(QPalette::Highlight, QColor(42, 130, 218));
  d.setColor(QPalette::HighlightedText, Qt::black);

  qApp->setStyle(QStyleFactory::create("Fusion"));
  qApp->setPalette(d);
  qApp->setStyleSheet("QToolTip { color: #FFFFFF; background-color: #2A82DA; border: 1px solid white; }");

  w.show();
  return a.exec();
}
