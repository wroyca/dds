#ifndef DIA_INTERFACES_H
#define DIA_INTERFACES_H

#include <dia2.h>
#include <atlcomcli.h>

struct Dia
{
  // Initiates access to a source of debugging symbols.
  CComPtr<IDiaDataSource> primary_data_source, secondary_data_source;

  // Provides a query context for debug symbols.
  CComPtr<IDiaSession> primary_session, secondary_session;

  // Describes the properties of a symbol instance.
  CComPtr<IDiaSymbol> primary_symbol, secondary_symbol;
};

extern Dia dia;

#endif // DIA_INTERFACES_H
