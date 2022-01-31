#include "dds_main_window.h"
#include "ui_dds_main_window.h"
#include "dia/dia_data_source.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QScrollBar>
#include <QMessageBox>

#undef DEBUG
#include <LIEF/LIEF.hpp>
#include <Zydis/Zydis.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), primary_model(new QFileSystemModel(this)), secondary_model(new QFileSystemModel(this))
{
  ui->setupUi(this);
  ui->treeWidget->hide();
  ui->treeWidget_2->hide();

  // connect(ui->treeWidget->horizontalScrollBar(), &QScrollBar::rangeChanged, [=]{ ui->treeWidget_2->horizontalScrollBar()->setValue(ui->treeWidget->horizontalScrollBar()->value()); });
}

auto MainWindow::on_actionDebugging_Symbols_triggered() -> void
{
  const auto primary_file_name = QFileDialog::getOpenFileName(nullptr, "Primary Program Database.", "", "Program Database (*.pdb)");

  if (primary_file_name.isEmpty())
    return;

  const auto secondary_file_name = QFileDialog::getOpenFileName(nullptr, "Secondary Program Database.", "", "Program Database (*.pdb)");
  
  if (secondary_file_name.isEmpty())
    return;

  load_debugging_symbols({primary_file_name, secondary_file_name});

  // TODO: Compare tree, change treeView item color to red if it can't be found in treeView_2. 
  // 
  // load_source_tree()
  // 
  // primary_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  // primary_model->setRootPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/dds/"));
  // 
  // secondary_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  // secondary_model->setRootPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/dds/"));
  // 
  // ui->treeView->setModel(primary_model);
  // ui->treeView->setRootIndex(primary_model->index(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddsx/0/C/projects_pc/cod/codsrc")));
  // ui->treeView->header()->setStretchLastSection(true);
  // ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  // 
  // ui->treeView_2->setModel(primary_model);
  // ui->treeView_2->setRootIndex(primary_model->index(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddsx/1/c/projects_pc/cod/codsrc")));
  // ui->treeView_2->header()->setStretchLastSection(true);
  // ui->treeView_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  ui->treeView->hide();
  ui->treeView_2->hide();
  ui->treeWidget->show();
  ui->treeWidget_2->show();

  primary_file_info = QFileInfo(primary_file_name);
  secondary_file_info = QFileInfo(secondary_file_name);
}

auto MainWindow::on_pushButton_clicked() -> void
{
  auto primary_binary = LIEF::PE::Parser::parse((primary_file_info.path() + "/" + primary_file_info.completeBaseName() + ".exe").toStdString());
  auto secondary_binary = LIEF::PE::Parser::parse((secondary_file_info.path() + "/" + secondary_file_info.completeBaseName() + ".exe").toStdString());

  // TODO: don't throw if F is not found? // R_InitDynamicVertexBufferState

  try
  {
    //your code here

    auto primary_symbol = primary_binary->get_content_from_virtual_address(symbol_rva(ui->textEdit->toPlainText(), 0), symbol_length(ui->textEdit->toPlainText(), 0), LIEF::Binary::VA_TYPES::RVA);
    auto secondary_symbol = secondary_binary->get_content_from_virtual_address(symbol_rva(ui->textEdit->toPlainText(), 1), symbol_length(ui->textEdit->toPlainText(), 1), LIEF::Binary::VA_TYPES::RVA);

    // Initialize decoder context
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
    // visualize relative addressing
    ZyanU64 runtime_address = 0x007FFFFFFF400000;
    ZyanUSize offset = 0;
    ZyanU8 *p_data = &primary_symbol[0];
    ZyanU8 *s_data = &secondary_symbol[0];
    ZyanUSize p_length = primary_symbol.size();
    ZyanUSize s_length = secondary_symbol.size();
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

    ui->treeWidget->clear();
    ui->treeWidget_2->clear();

    QVector<QString> primary, secondary;

    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, p_data + offset, p_length - offset, &instruction, operands, ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
    {
      // Format & print the binary instruction structure to human readable format
      char buffer[256];
      ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address);
      puts(buffer);

      primary.push_back(buffer);

      /*QString a = buffer;

      const auto item = new QTreeWidgetItem();
      item->setText(0, a.section(' ', 0, 0));
      ui->treeWidget->addTopLevelItem(item);

      item->setText(1, a.section(' ', 1, 2));
      ui->treeWidget->addTopLevelItem(item);
      ui->treeWidget->setAlternatingRowColors(true);*/

      offset += instruction.length;
      runtime_address += instruction.length;
    }

    offset = 0;
    runtime_address = 0x007FFFFFFF400000;

    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, s_data + offset, s_length - offset, &instruction, operands, ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
    {
      // Format & print the binary instruction structure to human readable format
      char buffer[256];
      ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address);
      puts(buffer);
      secondary.push_back(buffer);
      offset += instruction.length;
      runtime_address += instruction.length;
    }

    // TODO: Cleanup
    for (int i = 0; i < primary.size(); i++)
    {
      if(i < secondary.size())
      {
        QString b = primary.at(i);
        QString c = secondary.at(i);

        if (b == c)
        {
          QString b = primary.at(i);
          QString c = secondary.at(i);

          // 1
          const auto item1 = new QTreeWidgetItem();
          item1->setText(0, QString::number(i));
          ui->treeWidget->addTopLevelItem(item1);
          item1->setText(1, b.section(' ', 0, 0));
          ui->treeWidget->addTopLevelItem(item1);
          item1->setText(2, b.section(' ', 1, 1000));
          ui->treeWidget->addTopLevelItem(item1);
          ui->treeWidget->setAlternatingRowColors(true);

          // 2
          const auto item2 = new QTreeWidgetItem();
          item2->setText(0, QString::number(i));
          ui->treeWidget_2->addTopLevelItem(item2);
          item2->setText(1, c.section(' ', 0, 0));
          ui->treeWidget_2->addTopLevelItem(item2);
          item2->setText(2, c.section(' ', 1, 1000));
          ui->treeWidget_2->addTopLevelItem(item2);
          ui->treeWidget_2->setAlternatingRowColors(true);
        }
        else
        {
          // 1

          const auto item1 = new QTreeWidgetItem();
          item1->setText(0, QString::number(i));
          item1->setForeground(0, Qt::red);
          ui->treeWidget->addTopLevelItem(item1);
          item1->setText(1, b.section(' ', 0, 0));
          item1->setForeground(1, Qt::red);
          ui->treeWidget->addTopLevelItem(item1);
          item1->setText(2, b.section(' ', 1));
          item1->setForeground(2, Qt::red);
          ui->treeWidget->addTopLevelItem(item1);
          ui->treeWidget->setAlternatingRowColors(true);

          // 2
          const auto item2 = new QTreeWidgetItem();
          item2->setText(0, QString::number(i));
          item2->setForeground(0, Qt::red);
          ui->treeWidget_2->addTopLevelItem(item2);
          item2->setText(1, c.section(' ', 0, 0));
          item2->setForeground(1, Qt::red);
          ui->treeWidget_2->addTopLevelItem(item2);
          item2->setText(2, c.section(' ', 1));
          item2->setForeground(2, Qt::red);
          ui->treeWidget_2->addTopLevelItem(item2);
          ui->treeWidget_2->setAlternatingRowColors(true);
        }

        // Look like bo-reversed have more than codmpserver, oops? ;)
        if(i == primary.size() && i < secondary.size())
        {
          for(;i < secondary.size(); i++)
          {
            const auto item2 = new QTreeWidgetItem();
            item2->setText(0, QString::number(i));
            item2->setForeground(0, Qt::red);
            ui->treeWidget_2->addTopLevelItem(item2);
            item2->setText(1, c.section(' ', 0, 0));
            item2->setForeground(1, Qt::red);
            ui->treeWidget_2->addTopLevelItem(item2);
            item2->setText(2, c.section(' ', 1));
            item2->setForeground(2, Qt::red);
            ui->treeWidget_2->addTopLevelItem(item2);
            ui->treeWidget_2->setAlternatingRowColors(true);
          }
        }
      }
      else
      {
        // We've reached end of bo-reversed when codmpserver still have more.
        QString b = primary.at(i);
        const auto item1 = new QTreeWidgetItem();
        item1->setText(0, QString::number(i));
        item1->setForeground(0, Qt::red);
        ui->treeWidget->addTopLevelItem(item1);
        item1->setText(1, b.section(' ', 0, 0));
        item1->setForeground(1, Qt::red);
        ui->treeWidget->addTopLevelItem(item1);
        item1->setText(2, b.section(' ', 1));
        item1->setForeground(2, Qt::red);
        ui->treeWidget->addTopLevelItem(item1);
        ui->treeWidget->setAlternatingRowColors(true);
      }
    }
  }
  catch (...)
  {
    QMessageBox::critical(nullptr, "Error", ui->textEdit->toPlainText() + " not found");
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}


