#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    auto on_actionOpen_triggered() -> void;

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
