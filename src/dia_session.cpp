// ReSharper disable CppClangTidyClangDiagnosticReturnType
// ReSharper disable CppNotAllPathsReturnValue

#include "dia_session.h"
#include "dia_interfaces.h"

#include <QDir>
#include <QStandardPaths>

auto dia_session::dump_source_files() -> bool
{
  // In order to find the source files, we have to look at the image's compilands/modules
  IDiaEnumSymbols *enum_compilands;

  if (FAILED(interfaces.dia_symbol->findChildren(SymTagCompiland, NULL, nsNone, &enum_compilands)))
    return false;

  IDiaSymbol *compiland;
  ULONG celt = 0;

  while (SUCCEEDED(enum_compilands->Next(1, &compiland, &celt)) && celt == 1)
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
          auto source_name = QString(reinterpret_cast<QChar*>(bstr_source_name)).remove(':').replace('\\', '/');
          auto source_directory = source_name.section("/", 0, -2);

          // Physically extracting the source tree is more convenient than adding node manually to the treeview.
          // BUG: The append() function of the QString class was overwriting the variable instead of using a temporary. The lambda is a workaround.

          auto writable_location = [](auto &a)
          {
            return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddscache/").append(a);
          };

          if (QDir dir; !dir.exists(writable_location(source_directory)))
            dir.mkpath(writable_location(source_directory));

          if (QFile file(writable_location(source_name)); !file.exists())
            file.open(QIODevice::WriteOnly);

          SysFreeString(bstr_source_name);
        }
        source_file->Release();
      }
      enum_source_files->Release();
    }
    compiland->Release();
  }
  enum_compilands->Release();
}
