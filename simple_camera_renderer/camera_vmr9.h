// zheng.che copyright 2015
#pragma once

#include <comdef.h>
#include <d3d9.h>
#include <dshow.h>
#include <map>
#include <vmr9.h>

#include "camera_delegate.h"

namespace camera {

class CameraVMR9 : public CameraDelegate {
 public:
  CameraVMR9();
  virtual ~CameraVMR9();

  // CameraDelegate implementations.
  bool Init(HWND host_hwnd,
            CameraEventDelegate* event_delegate,
            CameraConfigDelegate* config_delegate) override;
  bool Uninit() override;
  bool NotifyConfigChanged() override;
  bool Run() override;
  bool Pause() override;
  bool Stop() override;
  bool Snapshot() override;

  bool OnHostWindowRepaint();

 private:
  bool FailInit();
  HWND CreateEventReceiver();
  IBaseFilter* GetVideoCaptureFilter(const std::string& device_path);
  bool InitVMR9Filter();
  bool ConnectFilters();
  IVMRMixerControl9* GetVMR9Control();
  IVMRWindowlessControl9* GetVMR9WindowlessControl();
  void OnDisplayModeChanged();
  void OnGraphEvent();

  static LRESULT CALLBACK EventReceiverProcess(
      HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static CameraVMR9* FindCameraByWindow(HWND event_hwnd);

  IGraphBuilder* graph_manager_ = nullptr;
  IMediaControl* media_control_ = nullptr;
  IMediaEventEx* media_event_ = nullptr;
  IBaseFilter* video_capture_ = nullptr;
  IBaseFilter* vmr9_ = nullptr;
  HWND host_hwnd_ = nullptr;
  HWND event_receiver_hwnd_ = nullptr;
  CameraEventDelegate* event_delegate_ = nullptr;
  CameraConfigDelegate* config_delegate_ = nullptr;

  static std::map<HWND, CameraVMR9*> event_receiver_map_;
};

}
