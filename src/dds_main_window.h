#ifndef DDS_MAINWINDOW_H
#define DDS_MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>

namespace LIEF { namespace PE { class Binary; } }

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MainWindow)

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

//
private slots:
  auto on_actionDebugging_Symbols_triggered() -> void;
  auto on_pushButton_clicked() -> void;

private:
  Ui::MainWindow *ui;
  QFileSystemModel *primary_model, *secondary_model;
  QFileInfo primary_file_info, secondary_file_info;
  std::unique_ptr<LIEF::PE::Binary> primary_binary, secondary_binary;
};

#endif // DDS_MAINWINDOW_H
