// zheng.che copyright 2015
#include "camera_vmr9.h"

#include "camera_config_delegate.h"
#include "camera_event_delegate.h"

#define SAFE_RELEASE(x) { if (x) (x)->Release(); (x) = nullptr; }
#define WM_GRAPH_EVENT (WM_APP + 100)

namespace camera {

std::map<HWND, CameraVMR9*> CameraVMR9::event_receiver_map_;
const char kEventReceiverName[] = "camera vmr9 event receiver";

CameraVMR9::CameraVMR9() {
  
}

CameraVMR9::~CameraVMR9() {
  Uninit();
}

bool CameraVMR9::Init(HWND host_hwnd,
                      CameraEventDelegate* event_delegate,
                      CameraConfigDelegate* config_delegate) {
  // Return false if it's initialized already.
  if (host_hwnd_)
    return false;
  // These parameters cannot be null.
  if (!host_hwnd || !config_delegate)
    return false;
  host_hwnd_ = host_hwnd;
  config_delegate_ = config_delegate;
  event_delegate_ = event_delegate;

  HRESULT hr = S_OK;

  // Create the filter graph manager.
  hr = CoCreateInstance (CLSID_FilterGraph, nullptr, CLSCTX_INPROC,
                         IID_IGraphBuilder, (void**)&graph_manager_);
  if (FAILED(hr))
    return FailInit();

  // Obtain the interface for media control.
  hr = graph_manager_->QueryInterface(IID_IMediaControl,
                                      (void**)&media_control_);
  if (FAILED(hr))
    return FailInit();

  // Obtain the interface for media event.
  hr = graph_manager_->QueryInterface(IID_IMediaEventEx,
                                      (void**)&media_event_);
  if (FAILED(hr))
    return FailInit();

  // Create the event receiver window.
  event_receiver_hwnd_ = CreateEventReceiver();
  if (!event_receiver_hwnd_)
    return FailInit();

  // Set the window to retrive the graph event notify.
  hr = media_event_->SetNotifyWindow((OAHWND)event_receiver_hwnd_,
                                     WM_GRAPH_EVENT,
                                     0);
  if (FAILED(hr))
    return FailInit();

  // Get the video capture device filter.
  video_capture_ = GetVideoCaptureFilter(config_delegate->DevicePath());
  if (!video_capture_)
    return FailInit();

  // Get the VMR9 renderer filter.
  hr = CoCreateInstance(CLSID_VideoMixingRenderer9,
                        nullptr,
                        CLSCTX_INPROC,
                        IID_IBaseFilter,
                        (void**)&vmr9_);
  if (FAILED(hr))
    return FailInit();

  // Initialize the VMR9 renderer filter.
  if (!InitVMR9Filter())
    return FailInit();

  // Initialize the filters.
  if (!ConnectFilters())
    return FailInit();

  // Ignore this return value for Ignoring errors.
  NotifyConfigChanged();
  return true;
}

bool CameraVMR9::Uninit() {
  if (!host_hwnd_)
    return false;
  // Stop previewing
  if (media_control_)
    media_control_->Stop();
  if (media_event_)
    media_event_->SetNotifyWindow((OAHWND)nullptr, WM_GRAPH_EVENT, 0);
  // Release the all COM interfaces.
  SAFE_RELEASE(vmr9_);
  SAFE_RELEASE(video_capture_);
  SAFE_RELEASE(media_control_);
  SAFE_RELEASE(media_event_);
  SAFE_RELEASE(graph_manager_);
  // Remove from the event receiver map and close the receiver window.
  if (event_receiver_hwnd_) {
    event_receiver_map_.erase(event_receiver_hwnd_);
    SendMessage(event_receiver_hwnd_, WM_CLOSE, (WPARAM)0, (LPARAM)0);
    event_receiver_hwnd_ = nullptr;
  }
  // Forget the delegates and the host window.
  config_delegate_ = nullptr;
  event_delegate_ = nullptr;
  host_hwnd_ = nullptr;
  return true;
}

bool CameraVMR9::NotifyConfigChanged() {
  HRESULT hr = S_OK;
  if (!config_delegate_)
    return false;

  // Set or unset preview mirrored.
  IVMRMixerControl9* control = GetVMR9Control();
  if (!control)
    return false;
  VMR9NormalizedRect out_rect;
  out_rect.top = 0.0;
  out_rect.bottom = 1.0;
  if (config_delegate_->IsPreviewMirrored()) {
    out_rect.left = 1.0;
    out_rect.right = 0.0;
  } else {
    out_rect.left = 0.0;
    out_rect.right = 1.0;
  }
  hr = control->SetOutputRect(0, &out_rect);
  if (FAILED(hr)) {
    SAFE_RELEASE(control);
    return false;
  }
  SAFE_RELEASE(control);

  // Set the display position and source cropping.
  long origin_width = 0;
  long origin_height = 0;
  IVMRWindowlessControl9* windowless_control = GetVMR9WindowlessControl();
  if (!windowless_control)
    return false;
  hr = windowless_control->GetNativeVideoSize(
      &origin_width, &origin_height, nullptr, nullptr);
  if (FAILED(hr)) {
    SAFE_RELEASE(windowless_control);
    return false;
  }
  int out_width = config_delegate_->DisplayWidth();
  int out_height = config_delegate_->DisplayHeight();
  RECT dst = {
      config_delegate_->DisplayLeft(),
      config_delegate_->DisplayTop(),
      config_delegate_->DisplayLeft() + out_width,
      config_delegate_->DisplayTop() + out_height};
  RECT src = {0, 0, origin_width, origin_height};
  if (config_delegate_->IsCroppingByDisplayRatio()) {
    double ratio = (double)out_height / (double)out_width;
    long new_width = origin_width;
    long new_height = static_cast<long>(origin_width * ratio);
    if (new_height > origin_height) {
      new_height = origin_height;
      new_width = static_cast<long>(origin_height / ratio);
    }
    src.left = (origin_width - new_width) / 2;
    src.right = src.left + new_width;
    src.top = (origin_height - new_height) / 2;
    src.bottom = src.top + new_height;
  }
  hr = windowless_control->SetVideoPosition(&src, &dst);
  if (FAILED(hr)) {
    SAFE_RELEASE(windowless_control);
    return false;
  }
  SAFE_RELEASE(windowless_control);
  return true;
}

bool CameraVMR9::Run() {
  if (!media_control_)
    return false;
  return !(FAILED(media_control_->Run()));
}

bool CameraVMR9::Pause() {
  if (!media_control_)
    return false;
  return !(FAILED(media_control_->Pause()));
}

bool CameraVMR9::Stop() {
  if (!media_control_)
    return false;
  return !(FAILED(media_control_->Stop()));
}

bool CameraVMR9::Snapshot() {
  char* dib = nullptr;
  IVMRWindowlessControl9* windowless_control = GetVMR9WindowlessControl();
  if (!windowless_control)
    return false;
  HRESULT hr = windowless_control->GetCurrentImage((BYTE**)&dib);
  if (FAILED(hr)) {
    SAFE_RELEASE(windowless_control);
    return false;
  }

  BITMAPINFOHEADER* header = reinterpret_cast<BITMAPINFOHEADER*>(dib);
  int colors = 1 << header->biBitCount;
  if (colors > 256)
    colors = 0;
  int data_size = sizeof(BITMAPINFOHEADER) +
                  colors * sizeof(RGBQUAD) +
                  header->biSizeImage;
  if (event_delegate_)
    event_delegate_->OnSnapshot(dib, data_size);

  CoTaskMemFree(dib);
  SAFE_RELEASE(windowless_control);
  return true;
}

bool CameraVMR9::OnHostWindowRepaint() {
  if (!host_hwnd_)
    return false;
  IVMRWindowlessControl9* windowless_control = GetVMR9WindowlessControl();
  if (windowless_control) {
    HDC hdc = GetDC(host_hwnd_);
    windowless_control->RepaintVideo(host_hwnd_, hdc);
    ReleaseDC(host_hwnd_, hdc);
    SAFE_RELEASE(windowless_control);
    return true;
  }
  return false;
}

bool CameraVMR9::FailInit() {
  Uninit();
  return false;
}

HWND CameraVMR9::CreateEventReceiver() {
  static bool has_reg_class = false;
  if (!has_reg_class) {
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = CameraVMR9::EventReceiverProcess;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = kEventReceiverName;
    if (!RegisterClassA(&wc))
      return nullptr;
    has_reg_class = true;
  }

  HWND rcv = CreateWindowA(kEventReceiverName, "",
                           WS_POPUP, 0, 0, 0, 0,
                           nullptr, nullptr,
                           GetModuleHandleA(nullptr), 0);
  if (rcv)
    event_receiver_map_[rcv] = this;
  return rcv;
}

IBaseFilter* CameraVMR9::GetVideoCaptureFilter(const std::string& device_path) {
  HRESULT hr = S_OK;
  ICreateDevEnum* dev_enum = nullptr;
  IEnumMoniker* class_enum = nullptr;
  IMoniker* moniker = nullptr;
  IBaseFilter* filter = nullptr;

  // Create the system device enumerrator.
  hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                        IID_ICreateDevEnum, (void**)&dev_enum);
  if (FAILED(hr))
    return nullptr;

  // Create an enumerator for the video capture devices.
  hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                       &class_enum,
                                       0);
  if (FAILED(hr) || !class_enum) {
    SAFE_RELEASE(dev_enum);
    return nullptr;
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
    std::string prop_device_path;
    VARIANT var;
    VariantInit(&var);
    hr = prop_bag->Read(L"DevicePath", &var, nullptr);
    if (hr == S_OK) {
      prop_device_path = (_bstr_t)var.bstrVal;
      if (device_path == prop_device_path) {
        hr = moniker->BindToObject(nullptr,
                                   nullptr,
                                   IID_IBaseFilter,
                                   (void**)&filter);
        if (SUCCEEDED(hr)) {
          filter->AddRef();
        }
        VariantClear(&var);
        SAFE_RELEASE(prop_bag);
        SAFE_RELEASE(moniker);
        break;
      }
    }
    VariantClear(&var);
    SAFE_RELEASE(prop_bag);
    SAFE_RELEASE(moniker);
  }

  SAFE_RELEASE(dev_enum);
  SAFE_RELEASE(class_enum);
  return filter;
}

bool CameraVMR9::InitVMR9Filter() {
  if (!vmr9_)
    return false;

  HRESULT hr = S_OK;

  // Set the basic properties for the filter config.
  IVMRFilterConfig9* config = nullptr;
  hr = vmr9_->QueryInterface(IID_IVMRFilterConfig9, (void**)&config);
  if (FAILED(hr))
    return false;
  config->SetRenderingMode(VMR9Mode_Windowless);
  config->SetNumberOfStreams(1);
  DWORD prefs;
  if (SUCCEEDED(config->GetRenderingPrefs(&prefs)))
    config->SetRenderingPrefs(prefs | RenderPrefs_AllowOverlays);
  SAFE_RELEASE(config);

  // Set the video clipping window for the VMR9 renderer filter.
  IVMRWindowlessControl9* windowless_control = GetVMR9WindowlessControl();
  if (!windowless_control)
    return false;
  windowless_control->SetVideoClippingWindow(host_hwnd_);
  SAFE_RELEASE(windowless_control);

  return true;
}

bool CameraVMR9::ConnectFilters() {
  HRESULT hr = S_OK;

  // Create the capture graph builder to help building filter graph.
  ICaptureGraphBuilder2* graph_builder = nullptr;
  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2 , nullptr, CLSCTX_INPROC,
                        IID_ICaptureGraphBuilder2, (void **)&graph_builder);
  if (FAILED(hr))
    return false;

  if (!graph_builder || !graph_manager_)
    return false;

  // Inintialize the graph builder and add filters into the graph manager.
  HRESULT hr_init_builder =
          graph_builder->SetFiltergraph(graph_manager_);
  HRESULT hr_add_video_capture =
          graph_manager_->AddFilter(video_capture_, L"Video Capture");
  HRESULT hr_add_vmr9 =
          graph_manager_->AddFilter(vmr9_, L"VMR9 Renderer");
  if (FAILED(hr_init_builder) ||
      FAILED(hr_add_video_capture) ||
      FAILED(hr_add_vmr9)) {
    SAFE_RELEASE(graph_builder);
    return false;
  }

  // Connect the filters via the pins by graph builder intelligently.
  hr = graph_builder->RenderStream(&PIN_CATEGORY_PREVIEW,
                                   &MEDIATYPE_Video,
                                   video_capture_,
                                   nullptr,
                                   vmr9_);
  if (FAILED(hr)) {
    SAFE_RELEASE(graph_builder);
    return false;
  }

  SAFE_RELEASE(graph_builder);
  return true;
}

IVMRMixerControl9* CameraVMR9::GetVMR9Control() {
  if (!vmr9_)
    return nullptr;
  IVMRMixerControl9* control = nullptr;
  HRESULT hr = vmr9_->QueryInterface(IID_IVMRMixerControl9, (void**)&control);
  if (FAILED(hr))
    return nullptr;
  return control;
}

IVMRWindowlessControl9* CameraVMR9::GetVMR9WindowlessControl() {
  if (!vmr9_)
    return nullptr;
  IVMRWindowlessControl9* windowless_control = nullptr;
  HRESULT hr = vmr9_->QueryInterface(IID_IVMRWindowlessControl9,
                                     (void**)&windowless_control);
  if (FAILED(hr))
    return nullptr;
  return windowless_control;
}

void CameraVMR9::OnDisplayModeChanged() {
  IVMRWindowlessControl9* vmr9_control = nullptr;
  HRESULT hr = vmr9_->QueryInterface(IID_IVMRWindowlessControl9,
                                     (void**)&vmr9_control);
  if (FAILED(hr))
    return;
  vmr9_control->DisplayModeChanged();
  SAFE_RELEASE(vmr9_control);
}

void CameraVMR9::OnGraphEvent() {
  LONG evCode;
  LONG_PTR evParam1, evParam2;
  HRESULT hr = S_OK;

  if (!media_event_)
    return;

  while(SUCCEEDED(media_event_->GetEvent(&evCode, &evParam1, &evParam2, 0))) {
    if (evCode == EC_VMR_RENDERDEVICE_SET) {
      NotifyConfigChanged();
    }
    media_event_->FreeEventParams(evCode, evParam1, evParam2);
  }
}

// static
LRESULT CALLBACK CameraVMR9::EventReceiverProcess(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch(message) {
    case WM_DISPLAYCHANGE: {
      CameraVMR9* camera = FindCameraByWindow(hwnd);
      if (camera)
        camera->OnDisplayModeChanged();
      break;
    }
    case WM_GRAPH_EVENT: {
      CameraVMR9* camera = FindCameraByWindow(hwnd);
      if (camera)
        camera->OnGraphEvent();
      break;
    }
  }
  return DefWindowProc (hwnd , message, wParam, lParam);
}

// static
CameraVMR9* CameraVMR9::FindCameraByWindow(HWND event_hwnd) {
  auto iter = event_receiver_map_.find(event_hwnd);
  if (iter == event_receiver_map_.end())
    return nullptr;
  return iter->second;
}

}
