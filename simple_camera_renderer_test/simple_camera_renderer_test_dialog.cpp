
// simple_camera_renderer_test_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "simple_camera_renderer_test.h"
#include "simple_camera_renderer_test_dialog.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

bool StringToWString(const std::string &str, std::wstring &wstr) {
  int length = (int)str.length();
  wstr.resize(length + 1, L'\0');
  int result = MultiByteToWideChar(
               CP_ACP, 0, str.c_str(), length, (wchar_t*)wstr.c_str(), length);
  if (result == 0){
    return false;
  }
  return true;
}

}


// SimpleCameraRendererTestDialog dialog



SimpleCameraRendererTestDialog::SimpleCameraRendererTestDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(SimpleCameraRendererTestDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void SimpleCameraRendererTestDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_DEVICES, devices_combo_box_);
  DDX_Control(pDX, IDC_BUTTON_INIT, init_button_);
  DDX_Control(pDX, IDC_BUTTON_UNINIT, uninit_button_);
  DDX_Control(pDX, IDC_BUTTON_RUN, run_button_);
  DDX_Control(pDX, IDC_BUTTON_STOP, stop_button_);
  DDX_Control(pDX, IDC_CHECK_MIRRORED, mirrored_check_);
  DDX_Control(pDX, IDC_CHECK_CROPPING, cropping_check_);
  DDX_Control(pDX, IDC_STATIC_PREVIEW, preview_range_);
  DDX_Control(pDX, IDC_BUTTON_SNAPSHOT, snapshot_button_);
}

BEGIN_MESSAGE_MAP(SimpleCameraRendererTestDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BUTTON_INIT, &SimpleCameraRendererTestDialog::OnBnClickedButtonInit)
  ON_BN_CLICKED(IDC_BUTTON_RUN, &SimpleCameraRendererTestDialog::OnBnClickedButtonRun)
  ON_BN_CLICKED(IDC_CHECK_MIRRORED, &SimpleCameraRendererTestDialog::OnBnClickedCheckMirrored)
  ON_BN_CLICKED(IDC_CHECK_CROPPING, &SimpleCameraRendererTestDialog::OnBnClickedCheckCropping)
  ON_BN_CLICKED(IDC_BUTTON_STOP, &SimpleCameraRendererTestDialog::OnBnClickedButtonStop)
  ON_BN_CLICKED(IDC_BUTTON_UNINIT, &SimpleCameraRendererTestDialog::OnBnClickedButtonUninit)
  ON_BN_CLICKED(IDC_BUTTON_SNAPSHOT, &SimpleCameraRendererTestDialog::OnBnClickedButtonSnapshot)
END_MESSAGE_MAP()


// SimpleCameraRendererTestDialog message handlers

BOOL SimpleCameraRendererTestDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
  preview_range_.ShowWindow(SW_HIDE);
  PrepareDevicesList();
  UpdateRendererParameters();
  renderer_.SetObserver(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void SimpleCameraRendererTestDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
    renderer_.OnHostWindowRepaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR SimpleCameraRendererTestDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void SimpleCameraRendererTestDialog::OnSnapshot(
    const char* dib_data, int data_size) {
  if (!dib_data || data_size < sizeof(BITMAPINFOHEADER))
    return;

  const BITMAPINFOHEADER* header =
      reinterpret_cast<const BITMAPINFOHEADER*>(dib_data);
  long long colors = (long long)1 << header->biBitCount;
  if (colors > 256)
    colors = 0;

  HDC hdc = ::GetDC(nullptr);
  HBITMAP hbmp = CreateDIBitmap(
      hdc,
      header,
      CBM_INIT,
      dib_data + header->biSize + colors * sizeof(RGBQUAD),
      (BITMAPINFO*)header,
      colors ? DIB_PAL_COLORS : DIB_RGB_COLORS);
  ::ReleaseDC(nullptr, hdc);
  if (!hbmp)
    return;
  CImage image;
  image.Attach(hbmp);
  image.Save(L"snapshot.bmp");
  ShellExecute(nullptr, L"open", L"snapshot.bmp", nullptr, nullptr, SW_NORMAL);
}

void SimpleCameraRendererTestDialog::PrepareDevicesList()
{
  devices_ = camera::EnumerateCameraDevices();
  for (auto device : devices_) {
    std::wstring name;
    StringToWString(device.name, name);
    devices_combo_box_.AddString(name.c_str());
  }
}

void SimpleCameraRendererTestDialog::UpdateRendererParameters() {
  CRect rect;
  preview_range_.GetWindowRect(&rect);
  ScreenToClient(&rect);
  renderer_.SetPreviewPosition(
      rect.left, rect.top, rect.Width(), rect.Height());
  renderer_.SetPreviewMirrored(mirrored_check_.GetCheck() ? true : false);
  renderer_.SetIntelligentCropping(cropping_check_.GetCheck() ? true : false);
}

void SimpleCameraRendererTestDialog::OnBnClickedButtonInit()
{
  int index = devices_combo_box_.GetCurSel();
  if (index < 0 || index >= (int)devices_.size())
    return;
  renderer_.AttachToWindow(m_hWnd, devices_[index].device_path);
}


void SimpleCameraRendererTestDialog::OnBnClickedButtonRun()
{
  renderer_.Run();
}


void SimpleCameraRendererTestDialog::OnBnClickedCheckMirrored()
{
  UpdateRendererParameters();
}


void SimpleCameraRendererTestDialog::OnBnClickedCheckCropping()
{
  UpdateRendererParameters();
}


void SimpleCameraRendererTestDialog::OnBnClickedButtonStop()
{
  renderer_.Stop();
  Invalidate();
}


void SimpleCameraRendererTestDialog::OnBnClickedButtonUninit()
{
  renderer_.DetachFromWindow();
  Invalidate();
}


void SimpleCameraRendererTestDialog::OnBnClickedButtonSnapshot()
{
  renderer_.Snapshot();
}
