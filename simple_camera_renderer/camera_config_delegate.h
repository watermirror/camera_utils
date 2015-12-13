// zheng.che copyright 2015
#pragma once

#include <string>

namespace camera {

class CameraConfigDelegate {
 public:
  virtual bool IsPreviewMirrored() = 0;

  virtual bool IsCroppingByDisplayRatio() = 0;

  virtual int DisplayLeft() = 0;

  virtual int DisplayTop() = 0;

  virtual int DisplayWidth() = 0;

  virtual int DisplayHeight() = 0;

  virtual std::string DevicePath() = 0;
};

}
