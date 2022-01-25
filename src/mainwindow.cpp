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
}

MainWindow::~MainWindow()
{
  delete ui;
}

auto MainWindow::on_treeview_layout_update() -> void
{
  const auto writable_location(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddscache/"));

  file_system_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  file_system_model->setRootPath(writable_location);

  ui->treeView->setModel(file_system_model);
  ui->treeView->setRootIndex(file_system_model->index(writable_location));

  for (auto i = 1; i < file_system_model->columnCount(); ++i)
    ui->treeView->hideColumn(i);

  ui->treeView->header()->setStretchLastSection(true);
  ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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

#ifdef SKIP
  if (QDir dir; !dir.exists(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddscache/")))
    dia_session::dump_source_files();
#else
    dia_session::dump_source_files();
#endif

  ui->statusbar->showMessage("Ready.");

  on_treeview_layout_update();
}

// TODO: This is clearly a quick experimental function. It need to be reorganizated. 
void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
  if (const QFileInfo file(file_system_model->filePath(index)); file.exists() && file.isFile())
  {
    ui->treeWidget->clear();

    const auto data = index.data(Qt::DisplayRole).toString();
    const auto sz_file_name = data.toStdWString();

    IDiaEnumSourceFiles *enum_source_files;

    if (FAILED(interfaces.dia_session->findFile(NULL, sz_file_name.c_str(), nsFNameExt, &enum_source_files)))
      return;

    IDiaSourceFile *source_file;
    ULONG celt = 0;

    while (SUCCEEDED(enum_source_files->Next(1, &source_file, &celt)) && celt == 1)
    {
      IDiaEnumSymbols *enum_compilands;

      if (source_file->get_compilands(&enum_compilands) == S_OK)
      {
        IDiaSymbol *compiland;
        celt = 0;

        while (SUCCEEDED(enum_compilands->Next(1, &compiland, &celt)) && celt == 1)
        {
          BSTR bstrName;

          if (compiland->get_name(&bstrName) == S_OK)
          {
            wprintf(L"Compiland = %s\n", bstrName);

            IDiaEnumSymbols *enum_children;

            if (SUCCEEDED(compiland->findChildren(SymTagNull, NULL, nsNone, &enum_children)))
            {
              IDiaSymbol *pSymbol;
              ULONG celtChildren = 0;

              while (SUCCEEDED(enum_children->Next(1, &pSymbol, &celtChildren)) && (celtChildren == 1))
              {
                DWORD dwSymTag;

                if (pSymbol->get_symTag(&dwSymTag) == S_OK)
                {
                  switch (dwSymTag)
                  {
                    case SymTagFunction:
                    {
                      BSTR bstr_name;

                      if (pSymbol->get_name(&bstr_name) == S_OK)
                      {
                        // TODO: It somehow dump symbols on headers, so we want to
                        // abuse line number information instead.
                        auto name = QString(reinterpret_cast<QChar*>(bstr_name));
                        const auto item = new QTreeWidgetItem();
                        item->setText(0, name);
                        ui->treeWidget->addTopLevelItem(item);
                        ui->treeWidget->setHeaderLabel("name");
                      }
                    }
                  }
                }
                pSymbol->Release();
              }
              enum_children->Release();
            }
            SysFreeString(bstrName);
          }
          compiland->Release();
        }
        enum_compilands->Release();
      }
      source_file->Release();
    }
    enum_source_files->Release();
  }
}


void MainWindow::on_treeView_expanded(const QModelIndex &index)
{
  ui->treeView->resizeColumnToContents(0);
}

