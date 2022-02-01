#include "dia_data_source.h"
Dia dia;

#include <QDir>
#include <QStandardPaths>
#include <QFile>

auto load_debugging_symbols(const std::vector<QString> &files) -> void
{
  const auto co_initialize = CoInitialize(nullptr);
  assert(SUCCEEDED(co_initialize));

  const auto library = LoadLibrary(TEXT(R"(C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll)"));
  assert(library);

  const auto get_class_object = reinterpret_cast<HRESULT(*)(const IID &, const IID &, LPVOID)>(GetProcAddress(library, "DllGetClassObject")); // NOLINT(clang-diagnostic-cast-function-type)
  assert(get_class_object);

  for (size_t i = 0; i < files.size(); i++)
  {
    CComPtr<IClassFactory> class_factory; // Enables a class of objects to be created.

    assert(SUCCEEDED(get_class_object(__uuidof(DiaSource), __uuidof(IClassFactory), &class_factory)));
    assert(SUCCEEDED(class_factory->CreateInstance(nullptr, __uuidof(IDiaDataSource), reinterpret_cast<void**>(&dia.data_source[i]))));
    assert(SUCCEEDED(dia.data_source[i]->loadDataFromPdb(files[i].toStdWString().c_str())));
    assert(SUCCEEDED(dia.data_source[i]->openSession(&dia.session[i])));
    assert(SUCCEEDED(dia.session[i]->get_globalScope(&dia.symbol[i])));
  }
}

auto load_source_tree() -> void
{
  for (size_t i = 0; i < dia.data_source.size(); i++)
  {
    IDiaEnumSymbols *enum_compilands;

    if (FAILED(dia.symbol[i]->findChildren(SymTagCompiland, NULL, nsNone, &enum_compilands))) return;

    IDiaSymbol *compiland;
    ULONG celt = 0;

    while (SUCCEEDED(enum_compilands->Next(1, &compiland, &celt)) && celt == 1)
    {
      IDiaEnumSourceFiles *enum_source_files;

      if (SUCCEEDED(dia.session[i]->findFile(compiland, NULL, nsNone, &enum_source_files)))
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

            auto writable_location = [&](auto &location)
            {
              return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).append("/ddsx/" + QString::number(i) + "/").append(location);
            };

            if (QDir dir; !dir.exists(writable_location(source_directory))) dir.mkpath(writable_location(source_directory));
            if (QFile file(writable_location(source_name)); !file.exists()) file.open(QIODevice::WriteOnly);

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
}

auto symbol_rva(const QString symbol, int id) -> DWORD
{
  IDiaEnumSymbols *pEnumSymbols;

  if (FAILED(dia.symbol[id]->findChildren(SymTagFunction, symbol.toStdWString().c_str(), nsRegularExpression, &pEnumSymbols))) return 0;

  IDiaSymbol *pFunction;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pFunction, &celt)) && (celt == 1))
  {
    DWORD dwSymTag;

    if ((pFunction->get_symTag(&dwSymTag) != S_OK) || (dwSymTag != SymTagFunction)) return 0;

    DWORD dwRVA;

    if (pFunction->get_relativeVirtualAddress(&dwRVA) == S_OK) return dwRVA; // OK :)

    pFunction->Release();
  }
  pEnumSymbols->Release();
  return 0;
}

auto symbol_length(const QString symbol, int id) -> DWORD
{
  IDiaEnumSymbols *pEnumSymbols;

  if (FAILED(dia.symbol[id]->findChildren(SymTagFunction, symbol.toStdWString().c_str(), nsRegularExpression, &pEnumSymbols))) return 0;

  IDiaSymbol *pFunction;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pFunction, &celt)) && (celt == 1))
  {
    DWORD dwSymTag;

    if ((pFunction->get_symTag(&dwSymTag) != S_OK) || (dwSymTag != SymTagFunction)) return 0;

    ULONGLONG ulLength;

    if (pFunction->get_length(&ulLength) == S_OK) return ulLength; // OK :)

    pFunction->Release();
  }
  pEnumSymbols->Release();
  return 0;
}
