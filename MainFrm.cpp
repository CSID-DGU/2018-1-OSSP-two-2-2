// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GaSchedule.h"
#include "MainFrm.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_BN_CLICKED(ID_BUTTON_EXCEL, &CMainFrame::OnExcelButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_EXCEL, &CMainFrame::OnUpdateButton)
	ON_BN_CLICKED(ID_BUTTON_TXT, &CMainFrame::OnTxtButton)
	ON_BN_CLICKED(ID_BUTTON_START, &CMainFrame::OnStartButton)
	ON_COMMAND(ID_FILE_START, &CChildView::OnFileStart)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	m_wndView.ShowWindow(SW_HIDE); //버튼을 만들기 위해 생성한 m_wndView(자식뷰) 비활성화

	Button_excel.Create("CONVERT DATA", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(400, 200, 600, 250), this, ID_BUTTON_EXCEL);
	Button_excel.ShowWindow(SW_SHOW); //엑셀 버튼
	Button_txt.Create("LOAD DATA", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, CRect(400, 270, 600, 320), this, ID_BUTTON_TXT);
	Button_txt.ShowWindow(SW_SHOW); //텍스트 버튼
	Button_start.Create("START", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, CRect(400, 340, 600, 390), this, ID_BUTTON_START);
	Button_start.ShowWindow(SW_SHOW); //시간표 시작 버튼

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateButton(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
} //버튼 활성화 함수

class professor {
	string name;
	int id;
public:
	int getProfID() { return id; }
	string getProfName() { return name; }
	void setProfName(string s) { name = s; }
	void setProfID(int n) { id = n; }
};
class course {
	string name;
	int id;
	string lab;
	string div;
	string fix;
public:
	int getCourseID() { return id; }
	string getCourseName() { return name; }
	string getLab() { return lab; }
	string getDiv() { return div; }
	string getFix() { return fix; }
	void setCourseID(int n) { id = n; }
	void setCourseName(string s) { name = s; }
	void setLab(string s) { lab = s; }
	void setDiv(string s) { div = s; }
	void setFix(string s) { fix = s; }
};
class c_class {
	professor prof;
	string duration;
	course cour;
public:
	string getProfName() { return prof.getProfName(); }
	int getProfID() { return prof.getProfID(); }
	string getDuration() { return duration; }
	string getCourseName() { return cour.getCourseName(); }
	int getCourseID() { return cour.getCourseID(); }
	string getLab() { return cour.getLab(); }
	string getDiv() { return cour.getDiv(); }
	string getFix() { return cour.getFix(); }

	void setProfName(string s) { prof.setProfName(s); }
	void setProfID(int n) { prof.setProfID(n); }
	void setDuration(string s) { duration = s; }
	void setCourseID(int n) { cour.setCourseID(n); }
	void setCourseName(string s) { cour.setCourseName(s); }
	void setLab(string s) { cour.setLab(s); }
	void setDiv(string s) { cour.setDiv(s); }
	void setFix(string s) { cour.setFix(s); }
};
class room {
	string name;
	string lab;
	int size;
public:
	string getName() { return name; }
	string getLab() { return lab; }
	int getSize() { return size; }
	void setName(string s) { name = s; }
	void setLab(string s) { lab = s; }
	void setSize(int s) { size = s; }
};
void CMainFrame::OnExcelButton()
{
	m_rect.Create(TEXT("STATIC"), TEXT("\n CONVERT DATA\n  step 1. 수업정보.csv 파일 선택 \n  step 2. 강의실정보.csv 파일 선택"), WS_VISIBLE | WS_CHILD, CRect(700, 200, 950, 300), this, 1111);
	CFileDialog ex(TRUE, NULL, NULL, 0,
		_T("Class Schedule Excel Files (*.csv)|*.csv|All Files (*.*)|*.*||"));
	c_class c[100]; int c_size = 0;
	if (ex.DoModal() == IDOK)
	{
		CString path = ex.GetFolderPath().GetBuffer();
		CString op("\\");
		CString name = ex.GetFileName().GetBuffer();
		CString tmp = path + op + name;

		string prof, duration, course, lab, index, x, div, fix;
		string room[200];
		ifstream f(tmp);
		getline(f, index, '\n');
		while (f.good()) {
			getline(f, course, ','); c[c_size].setCourseName(course);
			getline(f, duration, ','); c[c_size].setDuration(duration);
			getline(f, lab, ','); c[c_size].setLab(lab);
			getline(f, prof, ','); c[c_size].setProfName(prof);
			getline(f, div, ','); c[c_size].setDiv(div);
			getline(f, fix, '\n'); c[c_size].setFix(fix);
			c_size++;
		}
		c_size--;
		f.close();
	}

	CFileDialog rr(TRUE, NULL, NULL, 0,
		_T("Class Schedule Excel Files (*.csv)|*.csv|All Files (*.*)|*.*||"));
	room r[100]; int r_size = 0;
	if (rr.DoModal() == IDOK)
	{
		CString path = rr.GetFolderPath().GetBuffer();
		CString op("\\");
		CString name = rr.GetFileName().GetBuffer();
		CString tmp = path + op + name;

		string rname, lab, size, index, x;
		ifstream f2(tmp);
		getline(f2, index, '\n');
		while (f2.good()) {
			getline(f2, rname, ','); r[r_size].setName(rname);
			getline(f2, lab, ',');
			if (lab == "O") r[r_size].setLab("true");
			else r[r_size].setLab("false");
			getline(f2, size, '\n'); r[r_size].setSize(stoi(size));
			r_size++;
		}
		r_size--;
		f2.close();
	}
	int cn = 0; string overlap = c[0].getProfName();
	ofstream outFile("output.txt");
	outFile << endl;
	for (int k = 0; k < c_size; k++) {
		int p;
		for (p = 0; p < k; p++) {
			if (c[k].getProfName() == c[p].getProfName()) break;
		}
		if (k == p) {
			cn++;
			outFile << "#prof" << endl;
			outFile << "\tid = " << cn << endl; c[k].setProfID(cn);
			outFile << "\tname = " << c[k].getProfName() << endl;
			outFile << "#end" << endl << endl;

		}
	}
	for (int k = 1; k < c_size; k++) {
		for (int q = 0; q < k; q++) {
			if (c[q].getProfName() == c[k].getProfName()) {
				int pid = c[q].getProfID();
				c[k].setProfID(pid);
			}
		}
	}
	int cn2 = 0;
	for (int k = 0; k < c_size; k++) {
		int p;
		for (p = 0; p < k; p++) {
			if (c[k].getCourseName() == c[p].getCourseName()) break;
		}
		if (k == p) {
			cn2++;
			outFile << "#course" << endl;
			outFile << "\tid = " << cn2 << endl; c[k].setCourseID(cn2);
			outFile << "\tname = " << c[k].getCourseName() << endl;
			outFile << "#end" << endl << endl;
		}
	}
	for (int k = 1; k < c_size; k++) {
		for (int q = 0; q < k; q++) {
			if (c[q].getCourseName() == c[k].getCourseName()) {
				int pid = c[q].getCourseID();
				c[k].setCourseID(pid);
			}
		}
	}
	for (int k = 0; k < r_size; k++) {
		outFile << "#room" << endl;
		outFile << "\tname = " << r[k].getName() << endl;
		outFile << "\tlab = " << r[k].getLab() << endl;
		outFile << "\tsize = " << r[k].getSize() << endl;
		outFile << "#end" << endl << endl;
	}
	for (int k = 0; k < c_size; k++) {
		outFile << "#class" << endl;
		outFile << "\tprofessor = " << c[k].getProfID() << endl;
		outFile << "\tcourse = " << c[k].getCourseID() << endl;
		outFile << "\tduration = " << c[k].getDuration() << endl;
		if (c[k].getLab() != "0") outFile << "\tlab = true" << endl;
		if (c[k].getDiv() != "0") outFile << "\tdivision = " << c[k].getDiv() << endl;
		if (c[k].getFix() != "/") outFile << "\tfixed = " << c[k].getFix() << endl;
		outFile << "#end" << endl << endl;
	}

	outFile.close();
}


