// zheng.che copyright 2015
#pragma once

#include <string>
#include <vector>

namespace camera {

struct CameraDevice {
  std::string name;
  std::string description;
  std::string device_path;
};

typedef std::vector<CameraDevice> CameraDeviceVector;

CameraDeviceVector EnumerateCameraDevices();

}
