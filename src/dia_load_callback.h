#ifndef DIA_LOAD_CALLBACK_HPP
#define DIA_LOAD_CALLBACK_HPP

#include <dia2.h>

class dia_load_callback final : public IDiaLoadCallback2
{
  int reference_count;

public:
  dia_load_callback() : reference_count(0) { AddRef(); }

  auto STDMETHODCALLTYPE AddRef() -> ULONG override
  {
    reference_count++;
    return reference_count;
  }

  auto STDMETHODCALLTYPE Release() -> ULONG override
  {
    if (--reference_count != 0)
      return reference_count;

    delete this;
    return 0;
  }

  auto STDMETHODCALLTYPE QueryInterface(REFIID rid, void **ppUnk) -> HRESULT override
  {
    if (ppUnk == nullptr)
      return E_INVALIDARG;
    if (rid == __uuidof( IDiaLoadCallback2))
      *ppUnk = static_cast<IDiaLoadCallback2*>(this);
    else if (rid == __uuidof( IDiaLoadCallback))
      *ppUnk = static_cast<IDiaLoadCallback*>(this);
    else if (rid == __uuidof( IUnknown))
      *ppUnk = static_cast<IUnknown*>(this);
    else
      *ppUnk = nullptr;

    if (*ppUnk != nullptr)
    {
      AddRef();
      return S_OK;
    }

    return E_NOINTERFACE;
  }

  auto STDMETHODCALLTYPE NotifyDebugDir(BOOL fExecutable, DWORD cbData, BYTE data[]) -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE NotifyOpenDBG(LPCOLESTR dbgPath, HRESULT resultCode) -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE NotifyOpenPDB(LPCOLESTR pdbPath, HRESULT resultCode) -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictRegistryAccess() -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictSymbolServerAccess() -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictOriginalPathAccess() -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictReferencePathAccess() -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictDBGAccess() -> HRESULT override
  {
    return S_OK;
  }

  auto STDMETHODCALLTYPE RestrictSystemRootAccess() -> HRESULT override
  {
    return S_OK;
  }
};

#endif // DIA_LOAD_CALLBACK_HPP
