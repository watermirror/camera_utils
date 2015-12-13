// zheng.che copyright 2015
#include "simple_camera_renderer.h"

#include "camera_delegate.h"
#include "camera_vmr9.h"
#include "simple_camera_renderer_observer.h"

namespace camera {

SimpleCameraRenderer::SimpleCameraRenderer() {
  delegate_ = new CameraVMR9();
}

SimpleCameraRenderer::~SimpleCameraRenderer() {
  if (delegate_) {
    delete delegate_;
    delegate_ = nullptr;
  }
}

void SimpleCameraRenderer::SetObserver(SimpleCameraRendererObserver* observer) {
  observer_ = observer;
}

bool SimpleCameraRenderer::AttachToWindow(HWND host_hwnd,
                                          const std::string& device_path) {
  device_path_ = device_path;
  if (!delegate_)
    return false;
  return delegate_->Init(host_hwnd, this, this);
}

bool SimpleCameraRenderer::DetachFromWindow() {
  if (!delegate_)
    return false;
  return delegate_->Uninit();
}

void SimpleCameraRenderer::SetPreviewMirrored(bool mirrored) {
  is_preview_mirrored_ = mirrored;
  if (delegate_)
    delegate_->NotifyConfigChanged();
}

void SimpleCameraRenderer::SetIntelligentCropping(bool enabled) {
  is_intelligent_cropping_ = enabled;
  if (delegate_)
    delegate_->NotifyConfigChanged();
}

void SimpleCameraRenderer::SetPreviewPosition(int left,
                                              int top,
                                              int width,
                                              int height) {
  display_left_ = left;
  display_top_ = top;
  display_width_ = width;
  display_height_ = height;
  if (delegate_)
    delegate_->NotifyConfigChanged();
}

bool SimpleCameraRenderer::Run() {
  return delegate_ && delegate_->Run();
}

bool SimpleCameraRenderer::Pause() {
  return delegate_ && delegate_->Pause();
}

bool SimpleCameraRenderer::Stop() {
  return delegate_ && delegate_->Stop();
}

bool SimpleCameraRenderer::Snapshot() {
  return delegate_ && delegate_->Snapshot();
}

bool SimpleCameraRenderer::OnHostWindowRepaint() {
  return delegate_ &&
         static_cast<CameraVMR9*>(delegate_)->OnHostWindowRepaint();
}

bool SimpleCameraRenderer::IsPreviewMirrored() {
  return is_preview_mirrored_;
}

bool SimpleCameraRenderer::IsCroppingByDisplayRatio() {
  return is_intelligent_cropping_;
}

int SimpleCameraRenderer::DisplayLeft() {
  return display_left_;
}

int SimpleCameraRenderer::DisplayTop() {
  return display_top_;
}

int SimpleCameraRenderer::DisplayWidth() {
  return display_width_;
}

int SimpleCameraRenderer::DisplayHeight() {
  return display_height_;
}

std::string SimpleCameraRenderer::DevicePath() {
  return device_path_;
}

void SimpleCameraRenderer::OnSnapshot(const char* dib_data, int data_size) {
  if (observer_)
    observer_->OnSnapshot(dib_data, data_size);
}

void SimpleCameraRenderer::OnRun() {
  if (observer_)
    observer_->OnRun();
}

void SimpleCameraRenderer::OnPause() {
  if (observer_)
    observer_->OnPause();
}

void SimpleCameraRenderer::OnStop() {
  if (observer_)
    observer_->OnStop();
}

void SimpleCameraRenderer::OnCameraException() {
  if (observer_)
    observer_->OnCameraException();
}

}
