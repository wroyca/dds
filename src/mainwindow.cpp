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

auto MainWindow::on_treeview_layout_about_to_be_changed() -> void
{
  ui->treeView->reset();
}

auto MainWindow::on_treeview_layout_changed() -> void
{
  const auto writable_location(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddscache/"));

  file_system_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  file_system_model->setRootPath(writable_location);

  ui->treeView->setModel(file_system_model);
  ui->treeView->setRootIndex(file_system_model->index(writable_location));

  for (auto i = 1; i < file_system_model->columnCount(); ++i)
    ui->treeView->hideColumn(i);

  ui->treeView->show();
}

auto MainWindow::on_actionOpen_triggered() -> void
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, tr("Open..."), "", tr("File (*.pdb)"));

  if (filename.isEmpty())
    return;

  ui->statusbar->showMessage("Loading the debugging symbols. It may take a few minutes."); qApp->processEvents();

  assert(SUCCEEDED(interfaces.dia_data_source->loadDataFromPdb(filename.toStdWString().c_str())));
  assert(SUCCEEDED(interfaces.dia_data_source->openSession(&interfaces.dia_session)));
  assert(SUCCEEDED(interfaces.dia_session->get_globalScope(&interfaces.dia_symbol)));

  dia_session::dump_source_files();

  ui->statusbar->showMessage("Ready.");

  on_treeview_layout_changed();
}

//dia_interfaces interfaces;