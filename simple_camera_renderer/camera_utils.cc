// zheng.che copyright 2015
#pragma once

#include "camera_utils.h"

#include <comdef.h>
#include <dshow.h>

#define SAFE_RELEASE(x) { if (x) (x)->Release(); (x) = nullptr; }

namespace camera {

CameraDeviceVector EnumerateCameraDevices() {
  CameraDeviceVector cameras;
  HRESULT hr = S_OK;
  ICreateDevEnum* dev_enum = nullptr;
  IEnumMoniker* class_enum = nullptr;
  IMoniker* moniker = nullptr;

  // Create the system device enumerrator.
  hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                        IID_ICreateDevEnum, (void**)&dev_enum);
  if (FAILED(hr))
    return cameras;

  // Create an enumerator for the video capture devices.
  hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                       &class_enum,
                                       0);
  if (FAILED(hr) || !class_enum) {
    SAFE_RELEASE(dev_enum);
    return cameras;
  }

  while(class_enum->Next(1, &moniker, nullptr) == S_OK) {
    IPropertyBag* prop_bag = nullptr;
    hr = moniker->BindToStorage(nullptr,
                                nullptr,
                                IID_IPropertyBag,
                                (void**)(&prop_bag));
    if (FAILED(hr)) {
      SAFE_RELEASE(moniker);
      continue;
    }
    CameraDevice device;
    VARIANT var;
    VariantInit(&var);
    hr = prop_bag->Read(L"FriendlyName", &var, nullptr);
    if (hr == S_OK)
      device.name = (_bstr_t)var.bstrVal;
    VariantClear(&var);
    hr = prop_bag->Read(L"Description", &var, nullptr);
    if (hr == S_OK)
      device.description = (_bstr_t)var.bstrVal;
    VariantClear(&var);
    hr = prop_bag->Read(L"DevicePath", &var, nullptr);
    if (hr == S_OK)
      device.device_path = (_bstr_t)var.bstrVal;
    VariantClear(&var);
    cameras.push_back(device);
    SAFE_RELEASE(prop_bag);
    SAFE_RELEASE(moniker);
  }

  SAFE_RELEASE(dev_enum);
  SAFE_RELEASE(class_enum);
  return cameras;
}

}
