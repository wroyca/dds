#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
  auto on_treeview_layout_update() -> void;

private slots:
  auto on_actionOpen_triggered() -> void;
  void on_treeView_clicked(const QModelIndex &index);

  void on_treeView_expanded(const QModelIndex &index);

private:
  Ui::MainWindow *ui;
  QFileSystemModel *file_system_model;
};

#endif // MAINWINDOW_H
