// zheng.che copyright 2015
#pragma once

#include <windows.h>

#include "camera_config_delegate.h"
#include "camera_event_delegate.h"

namespace camera {

class CameraDelegate;
class SimpleCameraRendererObserver;

class SimpleCameraRenderer :
    public CameraConfigDelegate,
    public CameraEventDelegate {
 public:
  SimpleCameraRenderer();
  virtual ~SimpleCameraRenderer();

  void SetObserver(SimpleCameraRendererObserver* observer);
  bool AttachToWindow(HWND host_hwnd, const std::string& device_path);
  bool DetachFromWindow();
  void SetPreviewMirrored(bool mirrored);
  void SetIntelligentCropping(bool enabled);
  void SetPreviewPosition(int left, int top, int width, int height);
  bool Run();
  bool Pause();
  bool Stop();
  bool Snapshot();
  bool OnHostWindowRepaint();

 protected:
  // CameraConfigDelegate implementations.
  bool IsPreviewMirrored() override;
  bool IsCroppingByDisplayRatio() override;
  int DisplayLeft() override;
  int DisplayTop() override;
  int DisplayWidth() override;
  int DisplayHeight() override;
  std::string DevicePath() override;

  // CameraEventDelegate implementations.
  void OnSnapshot(const char* dib_data, int data_size) override;
  void OnRun() override;
  void OnPause() override;
  void OnStop() override;
  void OnCameraException() override;

 private:
  CameraDelegate* delegate_ = nullptr;
  bool is_preview_mirrored_ = false;
  bool is_intelligent_cropping_ = false;
  int display_left_ = 0;
  int display_top_ = 0;
  int display_width_ = 0;
  int display_height_ = 0;
  std::string device_path_;
  SimpleCameraRendererObserver* observer_ = nullptr;
};

}
