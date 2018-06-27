// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "GaSchedule.h"
#include "ChildView.h"
#include "Configuration.h"
#include "Room.h"
#include "Schedule.h"
#include "MainFrm.h"
#include <iostream>
#include <fstream>
#include <string>
//#pragma comment(h,"Configuration.h")
using namespace std;
#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChildView::CChildView() : _schedule(NULL),
_running(false)
{
	Algorithm::GetInstance().GetObserver()->SetWindow(this);
}

CChildView::~CChildView()
{
	if (_schedule)
		delete _schedule;
}

void CChildView::SetSchedule(const Schedule* schedule)
{
	CSingleLock lock(&_sect, TRUE);

	if (_schedule)
		delete _schedule;

	_schedule = schedule->MakeCopy(false);

	lock.Unlock();

	Invalidate();
}

void CChildView::SetNewState(AlgorithmState state)
{
	_running = state == AS_RUNNING;
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_START, &CChildView::OnFileStart)
	ON_COMMAND(ID_FILE_STOP, &CChildView::OnFileStop)
	//ON_COMMAND(ID_FILE_OPEN_CONFIGURATION, &CChildView::OnFileOpenConfiguration)
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_CLOSE()
	//ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_CONFIGURATION, &CChildView::OnUpdateFileOpenConfiguration)
	ON_UPDATE_COMMAND_UI(ID_FILE_START, &CChildView::OnUpdateFileStart)
	ON_UPDATE_COMMAND_UI(ID_FILE_STOP, &CChildView::OnUpdateFileStop)
	//ON_COMMAND(ID_FILE_EXCEL, &CChildView::OnFileExcel)
END_MESSAGE_MAP()

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, //윈도우 프로시져 바인딩
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

	return TRUE;
}

const int ROOM_CELL_WIDTH = 85;
const int ROOM_CELL_HEIGHT = 30;

const int ROOM_MARGIN_WIDTH = 50;
const int ROOM_MARGIN_HEIGHT = 50;

const int ROOM_COLUMN_NUMBER = DAYS_NUM + 1;
const int ROOM_ROW_NUMBER = DAY_HOURS + 1;

const int ROOM_TABLE_WIDTH = ROOM_CELL_WIDTH * ROOM_COLUMN_NUMBER + ROOM_MARGIN_WIDTH;
const int ROOM_TABLE_HEIGHT = ROOM_CELL_HEIGHT * ROOM_ROW_NUMBER + ROOM_MARGIN_HEIGHT;

void CChildView::OnPaint()
{
	CPaintDC wndDC(this);

	CRect clientRect;
	GetClientRect(clientRect);

	CDC dc;
	CBitmap bmp; 
	dc.CreateCompatibleDC(&wndDC); //메모리 장치 컨텍스트 디스플레이 표면을 나타내는 메모리 블록
	bmp.CreateCompatibleBitmap(&wndDC, clientRect.Width(), clientRect.Height());
	dc.SelectObject(&bmp);

	int sx = -GetScrollPos(SB_HORZ);
	int sy = -GetScrollPos(SB_VERT);

	CBrush bgBrush(RGB(255, 255, 255));
	dc.FillRect(clientRect, &bgBrush);

	dc.SetBkColor(RGB(255, 255, 255));
	dc.SetBkMode(TRANSPARENT);

	CFont tableHeadersFont;
	tableHeadersFont.CreateFont(16, 0, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("Arial"));

	CFont tableTextFont;
	tableTextFont.CreateFont(14, 0, 0, 0, 100, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("Arial"));

	CFont roomDescFont;
	roomDescFont.CreateFont(12, 0, 0, 0, 100, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("Arial"));

	CFont criteriaFont;
	criteriaFont.CreateFont(14, 0, 0, 0, 900, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("Arial"));

	CBrush classBrush(RGB(255, 228, 181));//시간표(과목 색)
	CBrush classBrush_a(RGB(255, 216, 216));//신연순 교수님
	CBrush classBrush_b(RGB(250, 224, 212));//문봉교 교수님
	CBrush classBrush_c(RGB(255, 224, 140));//최성연 교수님
	CBrush classBrush_d(RGB(228, 247, 186));//주태우 교수님
	CBrush classBrush_e(RGB(206, 251, 201));//주해종 교수님
	CBrush classBrush_f(RGB(206, 251, 201));//이명숙 교수님
	CBrush classBrush_g(RGB(212, 244, 250));//최은만 교수님
	CBrush classBrush_h(RGB(218, 217, 255));//오세만 교수님
	CBrush classBrush_i(RGB(255, 217, 250));//주종화 교수님
	CBrush classBrush_j(RGB(255, 217, 236));//윤성림 교수님
	CBrush classBrush_k(RGB(234, 234, 234));//김경자 교수님
	CBrush classBrush_l(RGB(255, 167, 167));//엄진영 교수님
	CBrush classBrush_m(RGB(250, 237, 125));//장은실 교수님
	CBrush classBrush_n(RGB(181, 178, 255));//송양의 교수님
	CBrush classBrush_o(RGB(241,95,95));//김시명 교수님
	CBrush classBrush_p(RGB(242,150,97));//이호정 교수님
	CBrush classBrush_q(RGB(242,203,97));//홍성욱 교수님
	CBrush classBrush_r(RGB(229,216,92));//홍정모 교수님
	CBrush classBrush_s(RGB(188,229,92));//김가영 교수님
	CBrush classBrush_t(RGB(134,229,127));//장태무 교수님
	CBrush classBrush_u(RGB(92,209,229));//정준호 교수님
	CBrush classBrush_v(RGB(103,153,255));//정진우 교수님
	CBrush classBrush_w(RGB(107,102,255));//손윤식 교수님
	CBrush classBrush_x(RGB(165,102,255));//이강우 교수님
	CBrush classBrush_y(RGB(243,97,220));//박미화 교수님
	CBrush classBrush_z(RGB(243,97,166));//김동호 교수님
	CBrush classBrush_az(RGB(140,140,140));//이용규 교수님
	
	CPen classPen(PS_SOLID, 1, RGB(224, 224, 224));//시간표 틀 
	CBrush overlapBrush(HS_BDIAGONAL, RGB(255, 0, 0));
	CPen overlapPen(PS_NULL, 1, RGB(255, 0, 0));
	CBrush weekBrush(RGB(224, 224, 224));//요일 시간(색)
	CPen weekPen(PS_SOLID, 1, RGB(224, 224, 224));//요일 시간 틀

	int nr = Configuration::GetInstance().GetNumberOfRooms();

	dc.SelectObject(&weekBrush);
	dc.SelectObject(&weekPen);

	for (int k = 0; k < nr; k++)
	{
		for (int i = 0; i < ROOM_COLUMN_NUMBER; i++)
		{
			for (int j = 0; j < ROOM_ROW_NUMBER; j++)
			{
				int l = k % 2;
				int m = k / 2;

				CRect rect(
					sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + i * ROOM_CELL_WIDTH - 1,
					sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + j * ROOM_CELL_HEIGHT - 1,
					sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + (i + 1) * ROOM_CELL_WIDTH,
					sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + (j + 1) * ROOM_CELL_HEIGHT);

				if (i == 0 || j == 0)
					dc.Rectangle(rect);

				if (i == 0 && j == 0)
				{
					dc.SelectObject(&roomDescFont);

					rect.bottom -= rect.Height() / 2;
					dc.Rectangle(rect);

					CString str;
					str.Format(_T("Room: %s"), Configuration::GetInstance().GetRoomById(k)->GetName().c_str());
					dc.DrawText(str, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					rect.MoveToY(rect.bottom - 1);

					rect.right -= rect.Width() / 2 + 7;
					dc.Rectangle(rect);

					str.Format(_T("Lab: %c"), Configuration::GetInstance().GetRoomById(k)->IsLab() ? 'Y' : 'N');
					dc.DrawText(str, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					rect.MoveToX(rect.right - 1);
					rect.right += 14;

					str.Format(_T("Seats: %d"), Configuration::GetInstance().GetRoomById(k)->GetNumberOfSeats());
					dc.DrawText(str, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}

				dc.SelectObject(&tableHeadersFont);

				if (i == 0 && j > 0)
				{
					CString str, str2;
					str.Format(_T("%0.0f"), floor(9 + 0.5*j - 0.5));
					if (j % 2 == 0) {
						str2.Format(_T(":30"));
						
					}
					else {
						str2.Format(_T(":00"));
					}
					str = str + str2;


					dc.DrawText(str, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}

				if (j == 0 && i > 0)
				{
					
					if(i-1==0)
					dc.DrawText(_T("MON"), 3, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					else if (i - 1 == 1)
						dc.DrawText(_T("THU"), 3, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					else if (i - 1 == 2)
						dc.DrawText(_T("WED"), 3, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					else if (i - 1 == 3)
						dc.DrawText(_T("THR"), 3, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					else if (i - 1 == 4)
						dc.DrawText(_T("FRI"), 3, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
			}
		}
	}

	//dc.SelectObject(&classBrush);
	dc.SelectObject(&classPen);

	CSingleLock lock(&_sect, TRUE);

	if (_schedule)
	{
		CString fit;
		fit.Format(_T("적합도: %f, 세대 수: %d"), _schedule->GetFitness(),
			Algorithm::GetInstance().GetCurrentGeneration());

		dc.SelectObject(&tableHeadersFont);

		dc.TextOutA(sx + 10, sy + 10, fit);

		const hash_map<CourseClass*, int>& classes = _schedule->GetClasses();
		int ci = 0;
		for (hash_map<CourseClass*, int>::const_iterator it = classes.begin(); it != classes.end(); ++it, ci += 5)
		{
			CourseClass* c = (*it).first;
			int p = (*it).second;
			int pid = c->GetProfessor().GetId();

			if (pid == 1)
				dc.SelectObject(&classBrush_a);
			else if (pid == 2)
				dc.SelectObject(&classBrush_b);
			else if (pid == 3)
				dc.SelectObject(&classBrush_c);
			else if (pid == 4)
				dc.SelectObject(&classBrush_d);
			else if (pid == 5)
				dc.SelectObject(&classBrush_e);
			else if (pid == 6)
				dc.SelectObject(&classBrush_f);
			else if (pid == 7)
				dc.SelectObject(&classBrush_g);
			else if (pid == 8)
				dc.SelectObject(&classBrush_h);
			else if (pid == 9)
				dc.SelectObject(&classBrush_i);
			else if (pid == 10)
				dc.SelectObject(&classBrush_j);
			else if (pid == 11)
				dc.SelectObject(&classBrush_k);
			else if (pid == 12)
				dc.SelectObject(&classBrush_l);
			else if (pid == 13)
				dc.SelectObject(&classBrush_m);
			else if (pid == 14)
				dc.SelectObject(&classBrush_n);
			else if (pid == 15)
				dc.SelectObject(&classBrush_o);
			else if (pid == 16)
				dc.SelectObject(&classBrush_p);
			else if (pid == 17)
				dc.SelectObject(&classBrush_q);
			else if (pid == 18)
				dc.SelectObject(&classBrush_r);
			else if (pid == 19)
				dc.SelectObject(&classBrush_s);
			else if (pid == 20)
				dc.SelectObject(&classBrush_t);
			else if (pid == 21)
				dc.SelectObject(&classBrush_u);
			else if (pid == 22)
				dc.SelectObject(&classBrush_v);
			else if (pid == 23)
				dc.SelectObject(&classBrush_w);
			else if (pid == 24)
				dc.SelectObject(&classBrush_x);
			else if (pid == 25)
				dc.SelectObject(&classBrush_y);
			else if (pid == 26)
				dc.SelectObject(&classBrush_z);
			else if (pid == 27)
				dc.SelectObject(&classBrush_az);
			else
				dc.SelectObject(&classBrush);

			int t = p % (nr * DAY_HOURS);
			int d = p / (nr * DAY_HOURS) + 1;
			int r = t / DAY_HOURS;
			t = t % DAY_HOURS + 1;

			int l = r % 2;
			int m = r / 2;

			CRect rect(
				sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + d * ROOM_CELL_WIDTH - 1,
				sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + t * ROOM_CELL_HEIGHT - 1,
				sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + (d + 1) * ROOM_CELL_WIDTH,
				sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + (t + c->GetDuration()) * ROOM_CELL_HEIGHT);

			dc.Rectangle(rect);

			CString str;
			str.Format(_T("%s-%d\n%s\n/"), c->GetCourse().GetName().c_str(), c->GetDivision(), c->GetProfessor().GetName().c_str());

			
			for (list<StudentsGroup*>::const_iterator it = c->GetGroups().begin();
				it != c->GetGroups().end(); ++it)
			{
				str += (*it)->GetName().c_str();
				str += "/";
			}
			
			if (c->IsLabRequired())
				str += "\nLab";

			rect.top += 5;
			rect.bottom -= 5;
			rect.left += 5;
			rect.right -= 5;

			dc.SelectObject(&tableTextFont);

			dc.DrawText(str, rect, DT_CENTER | DT_WORDBREAK);

			dc.SelectObject(&criteriaFont);

			

			dc.SetTextColor(RGB(0, 0, 0));
		}

		dc.SelectObject(&overlapPen);
		dc.SelectObject(&overlapBrush);

		int i = 0;
		for (vector<list<CourseClass*>>::const_iterator it = _schedule->GetSlots().begin();
			it != _schedule->GetSlots().end(); ++it, ++i)
		{
			if ((*it).size() > 1)
			{
				int t = i % (nr * DAY_HOURS);
				int d = i / (nr * DAY_HOURS) + 1;
				int r = t / DAY_HOURS;
				t = t % DAY_HOURS + 1;

				int l = r % 2;
				int m = r / 2;

				CRect rect(
					sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + d * ROOM_CELL_WIDTH - 1,
					sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + t * ROOM_CELL_HEIGHT - 1,
					sx + ROOM_TABLE_WIDTH * l + ROOM_MARGIN_WIDTH + (d + 1) * ROOM_CELL_WIDTH,
					sy + ROOM_TABLE_HEIGHT * m + ROOM_MARGIN_HEIGHT + (t + 1) * ROOM_CELL_HEIGHT);

				dc.Rectangle(rect);
			}
		}
	}

	lock.Unlock();

	wndDC.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &dc, 0, 0, SRCCOPY);
}

DWORD WINAPI StartAlg(LPVOID param)
{
	Algorithm::GetInstance().Start();
	return 0;
}

void CChildView::OnFileStart()
{
	DWORD tid;
	HANDLE thread = CreateThread(NULL, 0, StartAlg, NULL, 0, &tid);
	CloseHandle(thread);
}

void CChildView::OnFileStop()
{
	Algorithm::GetInstance().Stop();
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return false;
}

void CChildView::ComputeScrollBars()
{
	int nr = Configuration::GetInstance().GetNumberOfRooms();

	int w = ROOM_TABLE_WIDTH;
	if (nr == 0)
		w = 0;
	else if (nr > 1)
		w *= 2;

	int h = ((nr / 2) + nr % 2) * ROOM_TABLE_HEIGHT;

	w += ROOM_MARGIN_WIDTH;
	h += ROOM_MARGIN_HEIGHT;

	CRect cr;
	GetClientRect(cr);

	SCROLLINFO hsi;
	hsi.cbSize = sizeof(hsi);
	hsi.fMask = SIF_RANGE | SIF_PAGE;
	hsi.nMin = 0;
	hsi.nMax = w;
	hsi.nPage = cr.Width();

	SetScrollInfo(SB_HORZ, &hsi, TRUE);

	hsi.nMax = h;
	hsi.nPage = cr.Height();

	SetScrollInfo(SB_VERT, &hsi, TRUE);
}

void CChildView::Scroll(int scrollBar, int nSBCode, int nPos)
{
	int minpos, maxpos, curpos;

	GetScrollRange(SB_HORZ, &minpos, &maxpos);
	maxpos = GetScrollLimit(scrollBar);
	curpos = GetScrollPos(scrollBar);

	switch (nSBCode)
	{
	case SB_LEFT:
		curpos = minpos;
		break;

	case SB_RIGHT:
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:
		break;

	case SB_LINELEFT:
		if (curpos > minpos)
			//curpos--;
			curpos -= (ROOM_TABLE_HEIGHT);
		break;

	case SB_LINERIGHT:
		if (curpos < maxpos)
			//curpos++;
			curpos += (ROOM_TABLE_HEIGHT);
		break;

	case SB_PAGELEFT:
	{
		SCROLLINFO   info;
		GetScrollInfo(scrollBar, &info, SIF_ALL);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:
	{
		SCROLLINFO info;
		GetScrollInfo(scrollBar, &info, SIF_ALL);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION:
		curpos = nPos;
		break;

	case SB_THUMBTRACK:
		curpos = nPos;
		break;
	}

	SetScrollPos(scrollBar, curpos);
}

void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	Scroll(SB_HORZ, nSBCode, nPos);
	Invalidate();

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	Scroll(SB_VERT, nSBCode, nPos);
	Invalidate();

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	ComputeScrollBars();
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) - zDelta);
	Invalidate();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnClose()
{
	Algorithm::GetInstance().Stop();
	Algorithm::GetInstance().GetObserver()->WaitEvent();
	Algorithm::FreeInstance();

	CWnd::OnClose();
}

void CChildView::OnUpdateFileStart(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!_running && !Configuration::GetInstance().IsEmpty());
}

void CChildView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(_running);
}

void CMainFrame::OnTxtButton()
{
	m_rect2.Create(TEXT("STATIC"), TEXT("\n  LOAD DATA\n : output.txt 파일 선택"), WS_VISIBLE | WS_CHILD, CRect(700, 270, 950, 330), this, 1111);
	CFileDialog dlg(TRUE, NULL, NULL, 0,
		_T("Class Schedule Config Files (*.txt)|*.txt|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString name = dlg.GetFileName().GetBuffer();
		char *temp = new char[name.GetLength()];
		strcpy_s(temp, name.GetLength() + 1, CT2A(name));
		Configuration::GetInstance().ParseFile(temp);

		int nr = Configuration::GetInstance().GetNumberOfRooms();

		int w = ROOM_TABLE_WIDTH;
		if (nr == 0)
			w = 0;
		else if (nr > 1)
			w *= 2;

		int h = ((nr / 2) + nr % 2) * ROOM_TABLE_HEIGHT;

		w += ROOM_MARGIN_WIDTH;
		h += ROOM_MARGIN_HEIGHT;

		CRect cr;
		GetClientRect(cr);

		SCROLLINFO hsi;
		hsi.cbSize = sizeof(hsi);
		hsi.fMask = SIF_RANGE | SIF_PAGE;
		hsi.nMin = 0;
		hsi.nMax = w;
		hsi.nPage = cr.Width();

		//SetScrollInfo(SB_HORZ, &hsi, TRUE);

		hsi.nMax = h;
		hsi.nPage = cr.Height();

		//SetScrollInfo(SB_VERT, &hsi, TRUE);
		Invalidate();
	}
}

void CMainFrame::OnStartButton()
{
	m_wndView.ShowWindow(SW_SHOW); //비활성화 시켜놓은 m_windView(자식뷰) 활성화
	Button_excel.ShowWindow(SW_HIDE);
	Button_txt.ShowWindow(SW_HIDE);
	Button_start.ShowWindow(SW_HIDE); //스타트 버튼 클릭 시 버튼 비활성화
	m_rect.ShowWindow(SW_HIDE);
	m_rect2.ShowWindow(SW_HIDE); //설명 비활성화

	DWORD tid;
	HANDLE thread = CreateThread(NULL, 0, StartAlg, NULL, 0, &tid);
	CloseHandle(thread);
}
void CChildView::OnScrollLeft()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CScrollBar* p = this->GetScrollBarCtrl(20);
	this->OnVScroll(100, 300, p);

}


void CChildView::OnScrollRight()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}


