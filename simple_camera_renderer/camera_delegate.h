// zheng.che copyright 2015
#pragma once

#include <windows.h>

namespace camera {

class CameraEventDelegate;
class CameraConfigDelegate;

class CameraDelegate {
 public:
  virtual bool Init(HWND host_hwnd,
                    CameraEventDelegate* event_delegate,
                    CameraConfigDelegate* config_delegate) = 0;

  virtual bool Uninit() = 0;

  virtual bool NotifyConfigChanged() = 0;

  virtual bool Run() = 0;

  virtual bool Pause() = 0;

  virtual bool Stop() = 0;

  virtual bool Snapshot() = 0;
};

}