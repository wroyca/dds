#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dia_data_source.h"

#include <QMainWindow>
#include <QFileSystemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow
{
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MainWindow)

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  auto treeview_init() -> void;

private slots:
  auto on_actionDebugging_Symbols_triggered() -> void;
  auto on_actionExit_triggered() -> void;
  auto on_treeView_collapsed(const QModelIndex &index) -> void;
  auto on_treeView_expanded(const QModelIndex &index) -> void;
  auto on_treeView_doubleClicked(const QModelIndex &index) -> void;

private:
  Ui::MainWindow *ui;
  QFileSystemModel *file_system_model;
  std::unique_ptr<dia_data_source> dia_data_source;
};

#endif // MAINWINDOW_H
