#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dia_interfaces.h"
#include "dia_load_callback.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

auto MainWindow::on_actionOpen_triggered() -> void
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, tr("Open..."), "", tr("File (*.pdb)"));

  if(filename.isEmpty())
    return;

  // TODO:
  //  A) Error handling
  //  B) loadDataForExe

  assert(SUCCEEDED(interfaces.dia_data_source->loadDataFromPdb(filename.toStdWString().c_str())));
}

//dia_interfaces interfaces;