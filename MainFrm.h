// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ChildView.h"

class CMainFrame : public CFrameWnd
{

public:
	CMainFrame();
protected:
	DECLARE_DYNAMIC(CMainFrame)

	// Attributes

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CButton Button_excel;
	CButton Button_txt;
	CButton Button_start;
	CWnd m_rect;
	CWnd m_rect2;
protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CChildView    m_wndView;
	CStatic m;
	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateButton(CCmdUI *pCmdUI);
	afx_msg void OnExcelButton();
	afx_msg void OnTxtButton();
	afx_msg void OnStartButton();


	//afx_msg BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
	//afx_msg void OnExcelbutton();
};


