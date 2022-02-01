#ifndef DIA_DATA_SOURCE_H
#define DIA_DATA_SOURCE_H

#include <dia2.h>
#include <atlcomcli.h>
#include <vector>

#include <QString>

struct Dia
{
  // Initiates access to a source of debugging symbols.
  CComPtr<IDiaDataSource> primary_data_source, secondary_data_source;

  // Provides a query context for debug symbols.
  CComPtr<IDiaSession> primary_session, secondary_session;

  // Describes the properties of a symbol instance.
  CComPtr<IDiaSymbol> primary_symbol, secondary_symbol;

  std::vector<CComPtr<IDiaDataSource>> data_source{primary_data_source, secondary_data_source};
  std::vector<CComPtr<IDiaSession>> session{primary_session, secondary_session};
  std::vector<CComPtr<IDiaSymbol>> symbol{primary_symbol, secondary_symbol};
};

// More convenient than passing an object reference through a deep hierarchy.
extern Dia dia;

auto load_debugging_symbols(const std::vector<QString> &files) -> void;
auto load_source_tree() -> void;

auto symbol_rva(QString symbol, int id) -> DWORD;
auto symbol_length(QString symbol, int id) -> DWORD;

#endif // DIA_DATA_SOURCE_H
