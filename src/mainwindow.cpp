#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dia_interfaces.h"
#include "dia_load_callback.h"

#include <Zydis/Zydis.h>

#include <QFileDialog>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>

#include <cinttypes>

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

  QFileInfo info(filename);
  const auto exe_filename = info.path() + "/" + info.completeBaseName() + ".exe";

  QFile file(exe_filename);
  if (!file.open(QIODevice::ReadOnly)) return;
  QByteArray contents = file.readAll();
  ZyanU8 *data = reinterpret_cast<ZyanU8 *>(contents.data());

  //ZyanU8 data[] = {0x51, 0x8D, 0x45, 0xFF, 0x50, 0xFF, 0x75, 0x0C, 0xFF, 0x75, 0x08, 0xFF, 0x15, 0xA0, 0xA5, 0x48, 0x76, 0x85, 0xC0, 0x0F, 0x88, 0xFC, 0xDA, 0x02, 0x00};

  // Initialize decoder context
  ZydisDecoder decoder;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

  // Initialize formatter. Only required when you actually plan to do instruction
  // formatting ("disassembling"), like we do here
  ZydisFormatter formatter;
  ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

  // Loop over the instructions in our buffer.
  // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
  // visualize relative addressing
  ZyanU64 runtime_address = 0x007FFFFFFF400000;

  ZyanU64 base_address = 0x401000;
  ZyanUSize offset = 0x7A1170 - base_address;

  constexpr auto length = sizeof(data);
  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

  ui->treeWidget->setHeaderLabel("name");
  ui->treeWidget->setColumnCount(2);
  ui->treeWidget->setColumnWidth(1, 1);
  //ui->tableWidget->insertRow(currentRowCount, 0, QTableWidgetItem("Some text"));

  //TODO: break after this many iterations. just some arbitary number
  static int i = 0;
  while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, data + offset, length - offset, &instruction, operands, ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
  {
    // Print current instruction pointer.
    //printf("a %016" PRIX64 "  ", runtime_address);


    // Format & print the binary instruction structure to human readable format
    char buffer[256];
    ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address);
    puts(buffer);

    QString a = buffer;

    const auto item = new QTreeWidgetItem(); item->setText(0, a.section(' ', 0, 0));
    ui->treeWidget->addTopLevelItem(item);

    item->setText(1, a.section(' ', 1, 2));
    ui->treeWidget->addTopLevelItem(item);

    offset += instruction.length;
    runtime_address += instruction.length;

    i++;
    if (i == 30)
      break;
  }

  // skip this for now
  /*
  ui->statusbar->showMessage("Loading the debugging symbols. It may take a few minutes."); qApp->processEvents();

  dia_data_source.reset(new struct dia_data_source(filename.toStdWString().c_str()));
  dia_data_source->get_source_files();

  treeview_init();
  */
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

