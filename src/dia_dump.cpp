#include "dia_dump.h"




//#include "dia_dump.h"
//#include "dia_interfaces.h"
//
//
//
//
//
//
//struct dia_dump
//{
//  template <typename F, typename... A>
//  dia_dump(F &&f, A&&... a);
//
//  auto source_files() -> bool;
//};
//
//auto dia_dump::source_files() -> bool
//{
//  // foo;
//}
//
//void test()
//{
//  dia_dump::dia_dump(dia_dump::source_files());
//}
//
//
//
//template <typename F, typename... A>
//auto dia_dump(F &&f, A &&... a) -> void
//{
//  IDiaEnumSymbols *enum_compilands;
//
//  if (SUCCEEDED(interfaces.dia_symbol->findChildren(SymTagCompiland, NULL, nsNone, &enum_compilands)))
//  {
//    IDiaSymbol *compiland;
//    ULONG celt = 0;
//
//    while (SUCCEEDED(enum_compilands->Next(1, &compiland, &celt)) && celt == 1)
//    {
//      std::forward<F>(f)(std::forward<compiland, A>(a)...);
//    }
//  }
//}
//
//auto b() -> void
//{
//  
//}
//
//auto f() -> void
//{
//  dia_dump(b);
//}