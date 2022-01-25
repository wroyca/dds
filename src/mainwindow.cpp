#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dia_interfaces.h"
#include "dia_load_callback.h"

#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow), file_system_model(new QFileSystemModel(this))
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

auto MainWindow::on_actionDebugging_Symbols_triggered() -> void
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, tr("Load Debugging Symbols..."), "", tr("Program Database (*.pdb)"));

  if (filename.isEmpty())
    return;

  ui->statusbar->showMessage("Loading the debugging symbols. It may take a few minutes."); qApp->processEvents();

  dia_data_source.reset(new struct dia_data_source(filename.toStdWString().c_str()));
  dia_data_source->get_source_files();

  treeview_init();
}

auto MainWindow::on_actionExit_triggered() -> void
{

}

auto MainWindow::treeview_init() -> void
{
  const auto writable_location(QStandardPaths::writableLocation(QStandardPaths::TempLocation).append("/dds/"));

  // TODO: We need a way to navigate backward and QDir::NoDotAndDotDot does *not* ignore subdirectories.

  file_system_model->setFilter(QDir::NoDot | QDir::AllDirs | QDir::Files);
  file_system_model->setRootPath(writable_location);

  ui->treeView->setModel(file_system_model);
  ui->treeView->setRootIndex(file_system_model->index(writable_location));

  for (auto i = 1; i < file_system_model->columnCount(); ++i)
    ui->treeView->hideColumn(i);

  ui->treeView->header()->setStretchLastSection(true);
  ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui->statusbar->clearMessage();
}

auto MainWindow::on_treeView_collapsed(const QModelIndex &index) -> void
{
  ui->treeView->resizeColumnToContents(0);
}

auto MainWindow::on_treeView_expanded(const QModelIndex &index) -> void
{
  ui->treeView->resizeColumnToContents(0);
}

auto MainWindow::on_treeView_doubleClicked(const QModelIndex &index) -> void
{
  if (const QFileInfo file(file_system_model->filePath(index)); !file.isFile())
  {
    file_system_model->setRootPath(file_system_model->filePath(index));                       // watchdog
    ui->treeView->setRootIndex(file_system_model->index(file_system_model->filePath(index))); // dir
  }
}

