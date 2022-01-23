#ifndef DIA_INTERFACES_H
#define DIA_INTERFACES_H

#include <dia2.h>
#include <atlcomcli.h>
#include <cassert>

struct dia_interfaces
{
  // Initializes the COM library interfaces on the current thread.
  dia_interfaces();

  // Initiates access to a source of debugging symbols.
  CComPtr<IDiaDataSource> dia_data_source;

  // Provides a query context for debug symbols.
  CComPtr<IDiaSession> dia_session;

  // Describes the properties of a symbol instance.
  CComPtr<IDiaSymbol> dia_symbol;
};

inline dia_interfaces::dia_interfaces()
{
  const auto co_initialize = CoInitialize(nullptr);
  assert(SUCCEEDED(co_initialize));

  // TODO:
  //  A ) Use CMakeLists to define the path during the first build.
  //  B ) Allow the user to define it manually in the settings. 

  const auto library = LoadLibrary(TEXT(R"(C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll)"));
  assert(library);

  const auto get_class_object = reinterpret_cast<HRESULT(*)(const IID &, const IID &, LPVOID)>(GetProcAddress(library, "DllGetClassObject")); // NOLINT(clang-diagnostic-cast-function-type)
  assert(get_class_object);

  // Enables a class of objects to be created.
  CComPtr<IClassFactory> class_factory;

  const auto object = get_class_object(__uuidof(DiaSource), __uuidof(IClassFactory), &class_factory);
  assert(SUCCEEDED(object));

  const auto instance = class_factory->CreateInstance(nullptr, __uuidof(IDiaDataSource), reinterpret_cast<void**>(&dia_data_source));
  assert(SUCCEEDED(instance));
}

// More convenient than passing an object reference through a deep hierarchy.
extern dia_interfaces interfaces;

#endif // DIA_INTERFACES_H
