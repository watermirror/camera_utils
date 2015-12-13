
// simple_camera_renderer_test_dialog.h : header file
//

#pragma once
#include "afxwin.h"

#include "../simple_camera_renderer/camera_utils.h"
#include "../simple_camera_renderer/simple_camera_renderer.h"
#include "../simple_camera_renderer/simple_camera_renderer_observer.h"

// SimpleCameraRendererTestDialog dialog
class SimpleCameraRendererTestDialog :
    public CDialogEx,
    public camera::SimpleCameraRendererObserver
{
// Construction
public:
	SimpleCameraRendererTestDialog(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIMPLE_CAMERA_RENDERER_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  CComboBox devices_combo_box_;
  CButton init_button_;
  CButton uninit_button_;
  CButton run_button_;
  CButton stop_button_;
  CButton mirrored_check_;
  CButton cropping_check_;
  CStatic preview_range_;

protected:
  // camera::SimpleCameraRendererObserver implementations.
  void OnSnapshot(const char* dib_data, int data_size) override;
  void OnRun() override {}
  void OnPause() override {}
  void OnStop() override {}
  void OnCameraException() override {};

private:
  void PrepareDevicesList();
  void UpdateRendererParameters();

  camera::SimpleCameraRenderer renderer_;
  camera::CameraDeviceVector devices_;
public:
  afx_msg void OnBnClickedButtonInit();
  afx_msg void OnBnClickedButtonRun();
  afx_msg void OnBnClickedCheckMirrored();
  afx_msg void OnBnClickedCheckCropping();
  afx_msg void OnBnClickedButtonStop();
  afx_msg void OnBnClickedButtonUninit();
  CButton snapshot_button_;
  afx_msg void OnBnClickedButtonSnapshot();
};
