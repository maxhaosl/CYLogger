
// CYLoggerTestDlg.h: 头文件
//

#pragma once

#include <thread>
#include <atomic>

// CCYLoggerTestDlg 对话框
class CCYLoggerTestDlg : public CDialogEx
{
// 构造
public:
	CCYLoggerTestDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CYLOGGERTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();

private:
	void Entry();

private:
	std::atomic_bool m_bRunning;
	std::jthread m_thread;

public:
	afx_msg void OnClose();
};
