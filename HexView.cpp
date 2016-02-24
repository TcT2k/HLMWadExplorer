#include "HexView.h"

#include <wx/settings.h>
#include <wx/dcclient.h>

bool HexView::Create(wxWindow *parent,
			wxWindowID winid,
			const wxPoint& pos,
			const wxSize& size)
{
	if (!wxScrolledCanvas::Create(parent, winid, pos, size))
		return false;
	
	m_rowsize = 16;
	m_data = NULL;
	m_dataSize = 0;
	
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	SetFont(wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ));
	
	Bind(wxEVT_PAINT, &HexView::OnPaint, this);
	Bind(wxEVT_SET_CURSOR, &HexView::OnSetCursor, this);
	Bind(wxEVT_LEFT_DOWN, &HexView::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &HexView::OnLeftUp, this);
	
	CalcSize();
	
	return true;
}

HexView::~HexView()
{
	Clear();
}

void HexView::CalcSize()
{
	unsigned int rowCount = m_dataSize / m_rowsize;
	wxSize fntSize = GetFont().GetPixelSize();
	m_rowHeight = fntSize.GetHeight() + 2;
	m_charWidth = fntSize.GetWidth();
	
	SetScrollRate(m_charWidth * 4, m_rowHeight * 20);
	SetScrollbars(m_charWidth, m_rowHeight, m_rowsize * 3, rowCount + 2);
}

void HexView::Clear()
{
	if (m_data)
	{
		delete [] m_data;
		m_data = NULL;
		m_dataSize = 0;
		
		Refresh();
		CalcSize();
	}
}

bool HexView::Load(const void* data, size_t dataSize)
{
	Clear();
	
	m_dataSize = dataSize;
	m_data = new unsigned char[dataSize];
	memcpy(m_data, data, dataSize);
	
	CalcSize();
	
	Refresh();
	
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

void HexView::OnSetCursor(wxSetCursorEvent& event)
{
	if (event.GetX() > (m_charWidth * 8) && event.GetY() > m_rowHeight)
		event.SetCursor(wxCURSOR_IBEAM);
	else
		event.SetCursor(wxCURSOR_ARROW);
}

void HexView::OnPaint(wxPaintEvent& event)
{
	wxPaintDC pdc(this);

	pdc.SetFont(GetFont());

	wxPoint rowPos(2, 2);
	wxSize offsetSize = pdc.GetTextExtent("00000000");
	offsetSize.x += 4;
	wxSize byteSize = pdc.GetTextExtent("00");
	byteSize.x += 2;
	
	// Header row
	pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	
	pdc.DrawText(_("Offset"), rowPos);
	rowPos.x += offsetSize.GetWidth();
	
	for (int i = 0; i < m_rowsize; ++i)
	{
		pdc.DrawText(wxString::Format("%.2X", i), rowPos);
		
		rowPos.x += byteSize.GetWidth();
	}
	
	rowPos.y += m_rowHeight;
	
	wxRect visRect = GetClientSize();
	wxPoint viewStart = GetViewStart();
	
	int offset = viewStart.y * m_rowsize;
	while (offset < m_dataSize && rowPos.y <= visRect.GetBottom())
	{
		rowPos.x = 2;
		int rowsize = m_rowsize;
		if (offset + rowsize > m_dataSize)
			rowsize = m_dataSize - offset;
		
		// Draw offset
		pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		pdc.DrawText(wxString::Format("%.8X", offset), rowPos);
		rowPos.x += offsetSize.GetWidth();
		
		// Draw row values
		pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		for (int i = 0; i < m_rowsize; ++i)
		{
			if (i < rowsize)
				pdc.DrawText(wxString::Format("%.2X", m_data[offset + i]), rowPos);
			
			rowPos.x += byteSize.GetWidth();
		}
		
		rowPos.x += m_charWidth;
		
		// Draw row characters
		wxString str = wxString::From8BitData((const char*)(&m_data[offset]), rowsize);
		for (auto it = str.begin(); it != str.end(); ++it)
		{
			if (*it < 32)
				*it = '.';
		}
		
		pdc.DrawText(str, rowPos);
		
		rowPos.y += m_rowHeight;
		offset+= rowsize;
	}
}

void HexView::OnLeftDown(wxMouseEvent& event)
{
	event.Skip();
}

void HexView::OnLeftUp(wxMouseEvent& event)
{
	event.Skip();
}
