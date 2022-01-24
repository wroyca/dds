#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dia_interfaces.h"
#include "dia_load_callback.h"
#include "dia_session.h"

#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow), file_system_model(new QFileSystemModel(this))
{
  ui->setupUi(this);
  ui->treeView->hide();
}

MainWindow::~MainWindow()
{
  delete ui;
}

auto MainWindow::on_actionOpen_triggered() -> void
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, tr("Open..."), "", tr("File (*.pdb)"));

  if (filename.isEmpty())
    return;

  // TODO:
  //  A) Error handling
  //  B) loadDataForExe

  assert(SUCCEEDED(interfaces.dia_data_source->loadDataFromPdb(filename.toStdWString().c_str())));
  assert(SUCCEEDED(interfaces.dia_data_source->openSession(&interfaces.dia_session))); // Move elsewhere?
  interfaces.dia_session->get_globalScope(&interfaces.dia_symbol);
  dia_session::dump_source_files();

  const auto root_path(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/cache/"));

  file_system_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  file_system_model->setRootPath(root_path);

  ui->treeView->setModel(file_system_model);
  ui->treeView->setRootIndex(file_system_model->index(root_path));

  // first column is the name
  for (auto i = 1; i < file_system_model->columnCount(); ++i)
    ui->treeView->hideColumn(i);

  ui->treeView->show();
}

//dia_interfaces interfaces;