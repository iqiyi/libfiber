﻿
// EchoServerDlg.h: 头文件
//

#pragma once

class CFiberServer;

// CEchoServerDlg 对话框
class CEchoServerDlg : public CDialogEx
{
// 构造
public:
	CEchoServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ECHOSERVER_DIALOG };
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

private:
	UINT m_port;
	CIPAddressCtrl m_listenAddr;
	CFiberServer* m_listenFiber;
	int m_fiberCount;
	int m_ioCount;
	int m_ioCurrent;
	CProgressCtrl m_ioProgress;

	int m_fiberDone;
	int m_ioDone;

	void InitFiber();
	void StopFiber();

public:
	void OnFiberIo();
	void OnFiberConnectDone();

public:
	afx_msg void OnBnClickedListen();
	afx_msg void OnBnClickedStopListen();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedConnect();
};
