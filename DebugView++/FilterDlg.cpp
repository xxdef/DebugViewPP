// (C) Copyright Gert-Jan de Vos 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// See http://boosttestui.wordpress.com/ for the boosttestui home page.

#include "stdafx.h"
#include "atlstr.h"
#include "resource.h"
#include "Utilities.h"
#include "FilterDlg.h"

namespace gj {

LogFilter::LogFilter() :
	type(FilterType::Include),
	color(RGB(255, 255, 255)),
	enable(true)
{
}

LogFilter::LogFilter(const std::string& text, FilterType::type type, COLORREF color, bool enable) :
	text(text), re(text, std::regex_constants::icase), type(type), color(color), enable(enable)
{
}

BEGIN_MSG_MAP_TRY(CFilterDlg)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_DESTROY(OnDestroy)
	COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER_EX(IDOK, OnOk)
	NOTIFY_CODE_HANDLER_EX(PIN_ADDITEM, OnAddItem);
	NOTIFY_CODE_HANDLER_EX(PIN_CLICK, OnClickItem);
	REFLECT_NOTIFICATIONS()
	CHAIN_MSG_MAP(CDialogResize<CFilterDlg>)
END_MSG_MAP_CATCH(ExceptionHandler)

CFilterDlg::CFilterDlg(const std::wstring& name) :
	m_name(name)
{
}

CFilterDlg::CFilterDlg(const std::wstring& name, const std::vector<LogFilter>& filters) :
	m_name(name),
	m_filters(filters)
{
}

std::wstring CFilterDlg::GetName() const
{
	return m_name;
}

std::vector<LogFilter> CFilterDlg::GetFilters() const
{
	return m_filters;
}

void CFilterDlg::ExceptionHandler()
{
	MessageBox(WStr(GetExceptionMessage()), LoadString(IDR_APPNAME).c_str(), MB_ICONERROR | MB_OK);
}

static const COLORREF HighlightColors[] =
{
	RGB(255, 255, 255), // white
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
};

std::unique_ptr<CColorPickerListCtrl> CFilterDlg::CreateColorCtrl()
{
	auto pCtrl = make_unique<CColorPickerListCtrl>();
	pCtrl->Create(*this, rcDefault, nullptr, WS_CHILD | WS_BORDER);
	pCtrl->SetParent(nullptr);
	int index = 0;
	for (auto it = std::begin(HighlightColors); it != std::end(HighlightColors); ++it, ++index)
		pCtrl->AddColor(index, *it);
	return pCtrl;
}

void CFilterDlg::AddFilter(const LogFilter& filter)
{
	int item = m_grid.GetItemCount();
	m_grid.InsertItem(item, PropCreateSimple(L"", WStr(filter.text)));

	static const wchar_t* types[] = { L"Include", L"Exclude", L"Highlight" , nullptr };
	auto pTypeList = PropCreateList(L"", types);
	pTypeList->SetValue(CComVariant(filter.type));
	m_grid.SetSubItem(item, 1, pTypeList);

	auto pColorCombo = CreateColorCtrl();
	m_grid.SetSubItem(item, 2, PropCreateComboControl(L"", *pColorCombo, filter.color));
	m_colorCtrls.push_back(std::move(pColorCombo));

	m_grid.SetSubItem(item, 3, PropCreateCheckButton(L"", filter.enable));
	m_grid.SetSubItem(item, 4, PropCreateReadOnlyItem(L"", L"�"));
	m_grid.SelectItem(item);
}

void CFilterDlg::RemoveFilter(int index)
{
	m_grid.DeleteItem(index);
	auto it = m_colorCtrls.begin() + index;
	(*it)->DestroyWindow();
	m_colorCtrls.erase(it);
}

BOOL CFilterDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	SetDlgItemText(IDC_NAME, m_name.c_str());

	m_grid.SubclassWindow(GetDlgItem(IDC_GRID));
	m_grid.InsertColumn(0, L"Filter", LVCFMT_LEFT, 200, 0);
	m_grid.InsertColumn(1, L"Type", LVCFMT_LEFT, 60, 0);
	m_grid.InsertColumn(2, L"Highlight", LVCFMT_LEFT, 90, 0);
	m_grid.InsertColumn(3, L"", LVCFMT_LEFT, 32, 0);
	m_grid.InsertColumn(4, L"", LVCFMT_LEFT, 16, 0);
	m_grid.SetExtendedGridStyle(PGS_EX_SINGLECLICKEDIT | PGS_EX_ADDITEMATEND);

	for (auto it = m_filters.begin(); it != m_filters.end(); ++it)
		AddFilter(*it);

	CenterWindow(GetParent());
	DlgResize_Init();
	return TRUE;
}

void CFilterDlg::OnDestroy()
{
	for (auto it = m_colorCtrls.begin(); it != m_colorCtrls.end(); ++it)
		(*it)->DestroyWindow();
}

LRESULT CFilterDlg::OnAddItem(NMHDR* pnmh)
{
	AddFilter(LogFilter());
	return 0;
}

LRESULT CFilterDlg::OnClickItem(NMHDR* pnmh)
{
	auto pClick = reinterpret_cast<NMPROPERTYITEM*>(pnmh);

	int iItem;
	int iSubItem;
	if (m_grid.FindProperty(pClick->prop, iItem, iSubItem) && iSubItem == 4)
	{
		RemoveFilter(iItem);
		return TRUE;
	}

	return FALSE;
}

void CFilterDlg::OnCancel(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	EndDialog(nID);
}

std::wstring GetGridItemText(const CPropertyGridCtrl& grid, int iItem, int iSubItem)
{
	const int BufSize = 1024;
	wchar_t buf[BufSize];
	if (grid.GetItemText(iItem, iSubItem, buf, BufSize))
		return buf;
	return L"";
}

template <typename ItemType>
ItemType& GetGridItem(const CPropertyGridCtrl& grid, int iItem, int iSubItem)
{
	return dynamic_cast<ItemType&>(*grid.GetProperty(iItem, iSubItem));
}

std::wstring CFilterDlg::GetFilterText(int iItem) const
{
	return GetGridItemText(m_grid, iItem, 0);
}

FilterType::type CFilterDlg::GetFilterType(int iItem) const
{
	CComVariant val;
	GetGridItem<CPropertyListItem>(m_grid, iItem, 1).GetValue(&val);
	switch (val.lVal)
	{
	case FilterType::Include: return FilterType::Include;
	case FilterType::Exclude: return FilterType::Exclude;
	case FilterType::Highlight: return FilterType::Highlight;
	}
	throw std::runtime_error("Unknown FilterType");
}

COLORREF CFilterDlg::GetFilterColor(int iItem) const
{
	CComVariant val;
	GetGridItem<CPropertyComboItem>(m_grid, iItem, 2).GetValue(&val);
	return val.lVal;
}

bool CFilterDlg::GetFilterEnable(int iItem) const
{
	CComVariant val;
	GetGridItem<CPropertyCheckButtonItem>(m_grid, iItem, 3).GetValue(&val);
	return val.boolVal != VARIANT_FALSE;
}

void CFilterDlg::OnOk(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	m_name = gj::GetDlgItemText(*this, IDC_NAME);

	std::vector<LogFilter> filters;
	int n = m_grid.GetItemCount();
	filters.reserve(n);

	for (int i = 0; i < n; ++i)
		filters.push_back(LogFilter(Str(GetFilterText(i)), GetFilterType(i), GetFilterColor(i), GetFilterEnable(i)));

	m_filters.swap(filters);
	EndDialog(nID);
}

} // namespace gj
