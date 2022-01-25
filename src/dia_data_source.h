#ifndef DIA_DATA_SOURCE_H
#define DIA_DATA_SOURCE_H

#include "dia_interfaces.h"

#include <comdef.h>

#include <QMessageBox>

struct dia_data_source : dia_interfaces
{
  dia_data_source(const wchar_t* filename) : dia_interfaces()
  {
    auto unwrap = [&](auto hr)
    {
      if (FAILED(hr))
      {
        const _com_error error(hr);
        QMessageBox::critical(nullptr, "Error", QString::fromWCharArray(error.ErrorMessage()));
      }
    };

    unwrap(data_source->loadDataFromPdb(filename));
    unwrap(data_source->openSession(&session));
    unwrap(session->get_globalScope(&symbol));
  }

  // Retrieves the source files name.
  auto get_source_files() -> void;
};


#endif // DIA_DATA_SOURCE_HE