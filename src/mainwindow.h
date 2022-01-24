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
  auto on_treeview_layout_about_to_be_changed() -> void;
  auto on_treeview_layout_changed() -> void;

private slots:
  auto on_actionOpen_triggered() -> void;

private:
  Ui::MainWindow *ui;
  QFileSystemModel *file_system_model;
};

#endif // MAINWINDOW_H
