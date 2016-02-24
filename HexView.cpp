#include "HexView.h"

#include <wx/settings.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <cmath>

class HexRowView : public wxScrolledCanvas
{
public:
	HexRowView(HexView *hexView,
		wxWindowID winid = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize)
	{
		Create(hexView, winid, pos, size);
		m_view = hexView;

		Bind(wxEVT_LEFT_DOWN, &HexRowView::OnLeftDown, this);
		Bind(wxEVT_LEFT_UP, &HexRowView::OnLeftUp, this);

		SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetCursor(wxCURSOR_IBEAM);

		CalcSize();
	}

	void CalcSize()
	{
		unsigned int rowCount = m_view->m_dataSize / m_view->m_rowsize;
		wxSize fntSize = m_view->GetFont().GetPixelSize();
		m_rowHeight = fntSize.GetHeight() + 2;
		m_charWidth = fntSize.GetWidth();

		SetScrollRate(m_charWidth * 4, m_rowHeight * 20);
		SetScrollbars(m_charWidth, m_rowHeight, m_view->m_rowsize * 3, rowCount + 2);

		Refresh();
	}

	void OnDraw(wxDC& dc)
	{
		dc.SetBackground(m_view->GetBackgroundColour());
		dc.Clear();

		dc.SetFont(m_view->GetFont());

		wxRect updateRect = GetUpdateClientRect();
		updateRect.SetPosition(CalcUnscrolledPosition(updateRect.GetPosition()));

		int startRow = updateRect.GetTop() / m_rowHeight;
		int endRow = std::ceil(updateRect.GetBottom() / (double)m_rowHeight);
		if (endRow > m_view->m_dataSize / m_view->m_rowsize)
			endRow = m_view->m_dataSize / m_view->m_rowsize;

		wxSize offsetSize = dc.GetTextExtent("00000000");
		offsetSize.x += 4;
		wxSize byteSize = dc.GetTextExtent("00");
		byteSize.x += 2;

		wxRect visRect = GetClientSize();
		wxPoint viewStart = GetViewStart();

		for (int row = startRow; row <= endRow; ++row)
		{
			int offset = row * m_view->m_rowsize;
			wxPoint rowPos = wxPoint(2, (row * m_rowHeight) + 2);

			int rowsize = m_view->m_rowsize;
			if (offset + rowsize > m_view->m_dataSize)
				rowsize = m_view->m_dataSize - offset;

			// Draw offset
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
			dc.DrawText(wxString::Format("%.8X", offset), rowPos);
			rowPos.x += offsetSize.GetWidth();

			// Draw row values
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
			for (int i = 0; i < m_view->m_rowsize; ++i)
			{
				if (i < rowsize)
					dc.DrawText(wxString::Format("%.2X", m_view->m_data[offset + i]), rowPos);

				rowPos.x += byteSize.GetWidth();
			}

			rowPos.x += m_charWidth;

			// Draw row characters
			wxString str((const char*)&m_view->m_data[offset], rowsize);
			for (auto it = str.begin(); it != str.end(); ++it)
			{
				if (*it < 32)
					*it = '.';
			}

			dc.DrawText(str, rowPos);

			rowPos.y += m_rowHeight;
		}
	}

	void OnLeftDown(wxMouseEvent& event)
	{
		event.Skip();
	}

	void OnLeftUp(wxMouseEvent& event)
	{
		event.Skip();
	}

private:
	int m_rowHeight;
	int m_charWidth;
	HexView* m_view;

	friend class HexView;
};

bool HexView::Create(wxWindow *parent,
			wxWindowID winid,
			const wxPoint& pos,
			const wxSize& size)
{
	if (!wxControl::Create(parent, winid, pos, size))
		return false;
	
	m_rowsize = 16;
	m_data = NULL;
	m_dataSize = 0;

	SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_rowView = new HexRowView(this);

	m_header = new wxPanel(this);
	m_header->SetMinClientSize(wxSize(0, m_rowView->m_rowHeight + 4));
	m_header->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	m_header->Bind(wxEVT_PAINT, &HexView::OnPaintHeader, this);

	sizer->Add(m_header, wxSizerFlags(0).Expand());

	sizer->Add(m_rowView, wxSizerFlags(1).Expand());
	SetSizer(sizer);

	m_rowView->CalcSize();
	
	return true;
}

HexView::~HexView()
{
	Clear();
}

void HexView::Clear()
{
	if (m_data)
	{
		delete [] m_data;
		m_data = NULL;
		m_dataSize = 0;
		
		m_rowView->CalcSize();
	}
}

bool HexView::Load(const void* data, size_t dataSize)
{
	Clear();
	
	m_dataSize = dataSize;
	m_data = new unsigned char[dataSize];
	memcpy(m_data, data, dataSize);
	
	m_rowView->CalcSize();
	
	return true;
}

bool HexView::Load(wxInputStream& iStr)
{
	wxFileOffset strSize = iStr.SeekI(0, wxFromEnd);
	iStr.SeekI(0);
	unsigned char* data = new unsigned char[strSize];
	iStr.Read(data, strSize);
	Load(data, strSize);
	delete [] data;
	
	return true;
}

void HexView::OnPaintHeader(wxPaintEvent& evt)
{
	wxPaintDC dc(m_header);

	dc.SetFont(GetFont());

	wxPoint rowPos = wxPoint(2, 2);
	wxSize offsetSize = dc.GetTextExtent("00000000");
	offsetSize.x += 4;
	wxSize byteSize = dc.GetTextExtent("00");
	byteSize.x += 2;

	// Header row
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

	dc.DrawText(_("Offset"), rowPos);
	rowPos.x += offsetSize.GetWidth();

	for (int i = 0; i < m_rowsize; ++i)
	{
		dc.DrawText(wxString::Format("%.2X", i), rowPos);

		rowPos.x += byteSize.GetWidth();
	}
}
