// (C) Copyright Gert-Jan de Vos and Jan Wilmans 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// Repository at: https://github.com/djeedjay/DebugViewPP/

#include "stdafx.h"
#include "atlstr.h"
#include "dbgstream.h"
#include "resource.h"
#include "Utilities.h"
#include "FilterDlg.h"

namespace fusion {

static COLORREF HighlightColors[16] = 
{
	RGB(255, 255, 255), // white
	RGB(192, 192, 192), // light-grey
	RGB(128, 128, 128), // mid-grey
	RGB( 64,  64,  64), // dark-grey
	RGB(  0,   0,   0), // black
	RGB( 27, 161, 226), // blue
	RGB(160,  80,   0), // brown
	RGB( 51, 153,  51), // green
	RGB(162, 193,  57), // lime
	RGB(216,   0, 115), // magenta
	RGB(240, 150,   9), // mango (orange)
	RGB(230, 113, 184), // pink
	RGB(162,   0, 255), // purple
	RGB(229,  20,   0), // red
	RGB(  0, 171, 169), // teal (viridian)
	RGB(255, 255, 255), // white
};

void InitializeCustomColors()
{
	auto colors = ColorDialog::GetCustomColors();
	for (int i = 0; i < 16; ++i)
		colors[i] = HighlightColors[i];
}

bool CustomColorsInitialized = (InitializeCustomColors(), true);

BEGIN_MSG_MAP_TRY(CFilterDlg)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_DESTROY(OnDestroy)
	MSG_WM_SIZE(OnSize)
	COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER_EX(IDOK, OnOk)
	NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnTabSelChange)
	REFLECT_NOTIFICATIONS()
	CHAIN_MSG_MAP(CDialogResize<CFilterDlg>)
END_MSG_MAP_CATCH(ExceptionHandler)

CFilterDlg::CFilterDlg(const std::wstring& name) :
	m_name(name)
{
}

CFilterDlg::CFilterDlg(const std::wstring& name, const LogFilter& filters) :
	m_messagePage(filters.messageFilters),
	m_name(name),
	m_filter(filters)
{
}

std::wstring CFilterDlg::GetName() const
{
	return m_name;
}

LogFilter CFilterDlg::GetFilters() const
{
	return m_filter;
}

void CFilterDlg::ExceptionHandler()
{
	MessageBox(WStr(GetExceptionMessage()), LoadString(IDR_APPNAME).c_str(), MB_ICONERROR | MB_OK);
}

BOOL CFilterDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	SetDlgItemText(IDC_NAME, m_name.c_str());

	m_tabCtrl.Attach(GetDlgItem(IDC_TAB));
	m_tabCtrl.AddItem(L"Messages");
	m_tabCtrl.AddItem(L"Processes");
	CRect tabRect;
	m_tabCtrl.GetWindowRect(&tabRect);
	m_tabCtrl.AdjustRect(false, &tabRect);
	m_tabCtrl.ScreenToClient(&tabRect);

	CRect dlgRect;
	GetClientRect(&dlgRect);
	m_border.cx = dlgRect.Width() - tabRect.Width();
	m_border.cy = dlgRect.Height() - tabRect.Height();

	m_messagePage.Create(m_tabCtrl, tabRect);
	m_messagePage.MoveWindow(&tabRect);
	m_messagePage.ShowWindow(SW_SHOW);

	m_taskPage.Create(m_tabCtrl, tabRect);
	m_taskPage.MoveWindow(&tabRect);
	m_taskPage.ShowWindow(SW_HIDE);

	CenterWindow(GetParent());
	DlgResize_Init();

	return TRUE;
}

void CFilterDlg::OnDestroy()
{
}

void CFilterDlg::OnSize(UINT /*nType*/, CSize size)
{
	RECT rect;
	m_tabCtrl.GetWindowRect(&rect);
	m_tabCtrl.AdjustRect(false, &rect);
	m_tabCtrl.ScreenToClient(&rect);
	rect.right = rect.left + size.cx - m_border.cx;
	rect.bottom = rect.top + size.cy - m_border.cy;

	m_messagePage.MoveWindow(&rect);
	m_taskPage.MoveWindow(&rect);
	SetMsgHandled(FALSE);
}

LRESULT CFilterDlg::OnTabSelChange(NMHDR* /*pnmh*/)
{
	int tab = m_tabCtrl.GetCurSel();
	m_messagePage.ShowWindow(tab == 0 ? SW_SHOW : SW_HIDE);
	m_taskPage.ShowWindow(tab == 1 ? SW_SHOW : SW_HIDE);
	return 0;
}

void CFilterDlg::OnCancel(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	EndDialog(nID);
}

void CFilterDlg::OnOk(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	m_name = fusion::GetDlgItemText(*this, IDC_NAME);
	m_filter.messageFilters = m_messagePage.GetFilters();
	EndDialog(nID);
}

} // namespace fusion
