// ReSharper disable CppClangTidyClangDiagnosticReturnType
// ReSharper disable CppNotAllPathsReturnValue

#include "dia_session.h"
#include "dia_interfaces.h"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>

auto dia_session::dump_source_files() -> bool
{
  QMessageBox::information(nullptr, "DDS", "This will take a while, please wait");

  // In order to find the source files, we have to look at the image's compilands/modules
  IDiaEnumSymbols *enum_symbols;

  if (FAILED(interfaces.dia_symbol->findChildren(SymTagCompiland, NULL, nsNone, &enum_symbols)))
    return false;

  IDiaSymbol *compiland;
  ULONG celt = 0;

  while (SUCCEEDED(enum_symbols->Next(1, &compiland, &celt)) && celt == 1)
  {
    // Every compiland could contain multiple references to the source files which were used to build it
    // Retrieve all source files by compiland by passing NULL for the name of the source file
    IDiaEnumSourceFiles *enum_source_files;

    if (SUCCEEDED(interfaces.dia_session->findFile(compiland, NULL, nsNone, &enum_source_files)))
    {
      IDiaSourceFile *source_file;

      while (SUCCEEDED(enum_source_files->Next(1, &source_file, &celt)) && celt == 1)
      {
        BSTR bstr_source_name;

        if (source_file->get_fileName(&bstr_source_name) == S_OK)
        {
          // TODO: This is quite messy, hardcoded, and bad. Cleanup later.
          auto source_name = QString(reinterpret_cast<QChar*>(bstr_source_name)).remove(':').replace('\\', '/');
          auto source_directory = source_name.section("/", 0, -2);

          auto extract_source = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/cache/").append(source_name);
          auto extract_directory = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/cache/").append(source_directory);

          if (QDir dir; !dir.exists(extract_directory))
            dir.mkpath(extract_directory);

          if (QFile file(extract_source); !file.exists())
            file.open(QIODevice::WriteOnly);

          SysFreeString(bstr_source_name);
        }
        source_file->Release();
      }
      enum_source_files->Release();
    }
    compiland->Release();
  }
  enum_symbols->Release();

  QMessageBox::information(nullptr, "DDS", "Done");
}
