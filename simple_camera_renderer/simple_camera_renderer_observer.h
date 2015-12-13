// zheng.che copyright 2015
#pragma once

namespace camera {

class SimpleCameraRendererObserver {
 public:
  virtual void OnSnapshot(const char* dib_data, int data_size) = 0;

  virtual void OnRun() = 0;

  virtual void OnPause() = 0;

  virtual void OnStop() = 0;

  virtual void OnCameraException() = 0;
};

}
