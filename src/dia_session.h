#ifndef DIA_SESSION_H
#define DIA_SESSION_H

#include <dia2.h>
#include <atlcomcli.h>
#include <cassert>

struct dia_session
{
  static auto dump_source_files() -> bool;
};

#endif // DIA_SESSION_H
