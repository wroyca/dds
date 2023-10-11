// WARN: This is a prototype not mean to be high quality code.

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

#include "Zycore/Format.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), primary_model(new QFileSystemModel(this)), secondary_model(new QFileSystemModel(this))
{
  ui->setupUi(this);
  ui->treeWidgetPrimary->hide();
  ui->treeWidgetSecondary->hide();
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

  // TODO: Compare tree, change treeViewPrimary item color to red if it can't be found in treeViewSecondary.
  //
  // load_source_tree()
  //
  // primary_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  // primary_model->setRootPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/dds/"));
  //
  // secondary_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
  // secondary_model->setRootPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/dds/"));
  //
  // ui->treeViewPrimary->setModel(primary_model);
  // ui->treeViewPrimary->setRootIndex(primary_model->index(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddsx/0/C/projects_pc/cod/codsrc")));
  // ui->treeViewPrimary->header()->setStretchLastSection(true);
  // ui->treeViewPrimary->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  //
  // ui->treeViewSecondary->setModel(primary_model);
  // ui->treeViewSecondary->setRootIndex(primary_model->index(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddsx/1/c/projects_pc/cod/codsrc")));
  // ui->treeViewSecondary->header()->setStretchLastSection(true);
  // ui->treeViewSecondary->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  ui->treeViewPrimary->hide();
  ui->treeViewSecondary->hide();
  ui->treeWidgetPrimary->show();
  ui->treeWidgetSecondary->show();

  primary_file_info = QFileInfo(primary_file_name);
  secondary_file_info = QFileInfo(secondary_file_name);

  // load executables
  primary_binary = LIEF::PE::Parser::parse((primary_file_info.path() + "/" + primary_file_info.completeBaseName() + ".exe").toStdString());
  secondary_binary = LIEF::PE::Parser::parse((secondary_file_info.path() + "/" + secondary_file_info.completeBaseName() + ".exe").toStdString());
}

ZydisFormatterFunc default_print_address_absolute;
ZydisFormatterFunc default_print_address_absolute_s;

auto GetTable(IDiaSession *pSession, REFIID iid, void **ppUnk) -> HRESULT
{
  IDiaEnumTables *pEnumTables;

  if (FAILED(pSession->getEnumTables(&pEnumTables))) {
    wprintf(L"ERROR - GetTable() getEnumTables\n");

    return E_FAIL;
  }

  IDiaTable *pTable;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumTables->Next(1, &pTable, &celt)) && (celt == 1)) {
    // There's only one table that matches the given IID

    if (SUCCEEDED(pTable->QueryInterface(iid, (void **) ppUnk))) {
      pTable->Release();
      pEnumTables->Release();

      return S_OK;
    }

    pTable->Release();
  }

  pEnumTables->Release();

  return E_FAIL;
}

static auto ZydisFormatterPrintAddressAbsolute_p(const ZydisFormatter *formatter, ZydisFormatterBuffer *buffer, ZydisFormatterContext *context) -> ZyanStatus
{
  ZyanU64 address;
  ZYAN_CHECK(ZydisCalcAbsoluteAddress(context->instruction, context->operand, context->runtime_address, &address));

  IDiaSymbol *pSymbol;
  LONG lDisplacement;

  // BUG: findSymbolByRVA consider nullptr a success,
  if (SUCCEEDED(dia.session[0]->findSymbolByRVA(static_cast<DWORD>(address - 0x0400000), SymTagFunction, &pSymbol)) && pSymbol != nullptr)
  {
    BSTR name;
    pSymbol->get_name(&name);
    const auto sym_name = QString(reinterpret_cast<QChar*>(name));

    ZYAN_CHECK(ZydisFormatterBufferAppend(buffer, ZYDIS_TOKEN_SYMBOL));
    ZyanString *string;
    ZYAN_CHECK(ZydisFormatterBufferGetString(buffer, &string));

    pSymbol->Release();
    SysFreeString(name);

    return ZyanStringAppendFormat(string, "<%s>", sym_name.toStdString().c_str());
  }

  //
  // TODO: Currently only work with global type, need to figure out how to handle local types as well.
  //

  if (SUCCEEDED(dia.session[0]->findSymbolByRVA(static_cast<DWORD>(address - 0x0400000), SymTagData, &pSymbol)) && pSymbol != nullptr)
  {
    DWORD dwDataKind;

    if (pSymbol->get_dataKind(&dwDataKind) != S_OK)
      return default_print_address_absolute(formatter, buffer, context);

    BSTR name;
    pSymbol->get_name(&name);
    const auto sym_name = QString(reinterpret_cast<QChar*>(name));

    ZYAN_CHECK(ZydisFormatterBufferAppend(buffer, ZYDIS_TOKEN_USER));
    ZyanString *string;
    ZYAN_CHECK(ZydisFormatterBufferGetString(buffer, &string));

    pSymbol->Release();
    SysFreeString(name);

    return ZyanStringAppendFormat(string, "<%s>", sym_name.toStdString().c_str());
  }

  return default_print_address_absolute(formatter, buffer, context);
}

static auto ZydisFormatterPrintAddressAbsolute_s(const ZydisFormatter *formatter, ZydisFormatterBuffer *buffer, ZydisFormatterContext *context) -> ZyanStatus
{
  ZyanU64 address;
  ZYAN_CHECK(ZydisCalcAbsoluteAddress(context->instruction, context->operand, context->runtime_address, &address));

  IDiaSymbol *pSymbol;
  LONG lDisplacement;

  // BUG: findSymbolByRVA consider nullptr a success,
  if (SUCCEEDED(dia.session[1]->findSymbolByRVA(static_cast<DWORD>(address - 0x0400000), SymTagFunction, &pSymbol)) && pSymbol != nullptr)
  {
    BSTR name;
    pSymbol->get_name(&name);
    const auto sym_name = QString(reinterpret_cast<QChar*>(name));

    ZYAN_CHECK(ZydisFormatterBufferAppend(buffer, ZYDIS_TOKEN_SYMBOL));
    ZyanString *string;
    ZYAN_CHECK(ZydisFormatterBufferGetString(buffer, &string));

    pSymbol->Release();
    SysFreeString(name);

    return ZyanStringAppendFormat(string, "<%s>", sym_name.toStdString().c_str());
  }

  //
  // TODO: Currently only work with global type, need to figure out how to handle local types as well.
  //

  if (SUCCEEDED(dia.session[1]->findSymbolByRVA(static_cast<DWORD>(address - 0x0400000), SymTagData, &pSymbol)) && pSymbol != nullptr)
  {
    DWORD dwDataKind;

    if (pSymbol->get_dataKind(&dwDataKind) != S_OK)
      return default_print_address_absolute(formatter, buffer, context);

    BSTR name;
    pSymbol->get_name(&name);
    const auto sym_name = QString(reinterpret_cast<QChar*>(name));

    ZYAN_CHECK(ZydisFormatterBufferAppend(buffer, ZYDIS_TOKEN_USER));
    ZyanString *string;
    ZYAN_CHECK(ZydisFormatterBufferGetString(buffer, &string));

    pSymbol->Release();
    SysFreeString(name);

    return ZyanStringAppendFormat(string, "<%s>", sym_name.toStdString().c_str());
  }

  return default_print_address_absolute_s(formatter, buffer, context);
}


auto MainWindow::on_pushButton_clicked() -> void
{
  const auto func_name = ui->textEdit->toPlainText();

  try
  {
    auto primary_sym_rva = symbol_rva(func_name, 0);
    auto secondary_sym_rva = symbol_rva(func_name, 1);

    auto primary_symbol = primary_binary->get_content_from_virtual_address(primary_sym_rva, symbol_length(func_name, 0), LIEF::Binary::VA_TYPES::RVA);
    auto secondary_symbol = secondary_binary->get_content_from_virtual_address(secondary_sym_rva, symbol_length(func_name, 1), LIEF::Binary::VA_TYPES::RVA);

    // Initialize decoder context
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    ZyanU64 runtime_address = primary_sym_rva + 0x0400000;
    ZyanUSize offset = 0;
    ZyanU8 *p_data = &primary_symbol[0];
    ZyanU8 *s_data = &secondary_symbol[0];
    ZyanUSize p_length = primary_symbol.size();
    ZyanUSize s_length = secondary_symbol.size();
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

    ui->treeWidgetPrimary->clear();
    ui->treeWidgetSecondary->clear();

    QVector<QString> primary, secondary;

    default_print_address_absolute = (ZydisFormatterFunc)&ZydisFormatterPrintAddressAbsolute_p;
    ZydisFormatterSetHook(&formatter, ZYDIS_FORMATTER_FUNC_PRINT_ADDRESS_ABS,
        (const void**)&default_print_address_absolute);

    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, p_data + offset, p_length - offset, &instruction, operands, ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
    {
      char buffer[256];
      ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address);
      primary.push_back(buffer);
      offset += instruction.length;
      runtime_address += instruction.length;
    }

    //
    // BUG: We recreate everything because the hook conflict otherwise.
    //

    // Initialize decoder context
    ZydisDecoder decoder_s;
    ZydisDecoderInit(&decoder_s, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter_s;
    ZydisFormatterInit(&formatter_s, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    ZyanU64 runtime_address_s = secondary_sym_rva + 0x0400000;
    ZyanUSize offset_s = 0;
    ZydisDecodedInstruction instruction_s;
    ZydisDecodedOperand operands_s[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

    default_print_address_absolute_s = (ZydisFormatterFunc)&ZydisFormatterPrintAddressAbsolute_s;
    ZydisFormatterSetHook(&formatter_s, ZYDIS_FORMATTER_FUNC_PRINT_ADDRESS_ABS,
        (const void**)&default_print_address_absolute_s);

    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder_s, s_data + offset_s, s_length - offset_s, &instruction_s, operands_s, ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
    {
      char buffer[256];
      ZydisFormatterFormatInstruction(&formatter_s, &instruction_s, operands_s, instruction_s.operand_count_visible, buffer, sizeof(buffer), runtime_address_s);
      secondary.push_back(buffer);
      offset_s += instruction_s.length;
      runtime_address_s += instruction_s.length;
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
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          item1->setText(1, b.section(' ', 0, 0));
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          item1->setText(2, b.section(' ', 1, 1000));
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          ui->treeWidgetPrimary->setAlternatingRowColors(true);

          // 2
          const auto item2 = new QTreeWidgetItem();
          item2->setText(0, QString::number(i));
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          item2->setText(1, c.section(' ', 0, 0));
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          item2->setText(2, c.section(' ', 1, 1000));
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          ui->treeWidgetSecondary->setAlternatingRowColors(true);
        }
        else
        {
          // 1

          const auto item1 = new QTreeWidgetItem();
          item1->setText(0, QString::number(i));
          item1->setForeground(0, Qt::red);
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          item1->setText(1, b.section(' ', 0, 0));
          item1->setForeground(1, Qt::red);
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          item1->setText(2, b.section(' ', 1));
          item1->setForeground(2, Qt::red);
          ui->treeWidgetPrimary->addTopLevelItem(item1);
          ui->treeWidgetPrimary->setAlternatingRowColors(true);

          // 2
          const auto item2 = new QTreeWidgetItem();
          item2->setText(0, QString::number(i));
          item2->setForeground(0, Qt::red);
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          item2->setText(1, c.section(' ', 0, 0));
          item2->setForeground(1, Qt::red);
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          item2->setText(2, c.section(' ', 1));
          item2->setForeground(2, Qt::red);
          ui->treeWidgetSecondary->addTopLevelItem(item2);
          ui->treeWidgetSecondary->setAlternatingRowColors(true);
        }

        // Look like A have more than B, oops? ;)
        if(i == primary.size() && i < secondary.size())
        {
          for(;i < secondary.size(); i++)
          {
            const auto item2 = new QTreeWidgetItem();
            item2->setText(0, QString::number(i));
            item2->setForeground(0, Qt::red);
            ui->treeWidgetSecondary->addTopLevelItem(item2);
            item2->setText(1, c.section(' ', 0, 0));
            item2->setForeground(1, Qt::red);
            ui->treeWidgetSecondary->addTopLevelItem(item2);
            item2->setText(2, c.section(' ', 1));
            item2->setForeground(2, Qt::red);
            ui->treeWidgetSecondary->addTopLevelItem(item2);
            ui->treeWidgetSecondary->setAlternatingRowColors(true);
          }
        }
      }
      else
      {
        // We've reached the end of A when B still have more.
        QString b = primary.at(i);
        const auto item1 = new QTreeWidgetItem();
        item1->setText(0, QString::number(i));
        item1->setForeground(0, Qt::red);
        ui->treeWidgetPrimary->addTopLevelItem(item1);
        item1->setText(1, b.section(' ', 0, 0));
        item1->setForeground(1, Qt::red);
        ui->treeWidgetPrimary->addTopLevelItem(item1);
        item1->setText(2, b.section(' ', 1));
        item1->setForeground(2, Qt::red);
        ui->treeWidgetPrimary->addTopLevelItem(item1);
        ui->treeWidgetPrimary->setAlternatingRowColors(true);
      }
    }
  }
  catch (...)
  {
    QMessageBox::critical(nullptr, "Error", func_name + " not found");
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}


