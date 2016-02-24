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
	
	m_stream = NULL;
	m_rowsize = 16;
	m_dataSize = 0;
	
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	SetFont(wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ));
	
	m_viewDataSize = 4096;
	m_viewData = new unsigned char[m_viewDataSize];
	
	Bind(wxEVT_PAINT, &HexView::OnPaint, this);
	Bind(wxEVT_SET_CURSOR, &HexView::OnSetCursor, this);
	Bind(wxEVT_LEFT_DOWN, &HexView::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &HexView::OnLeftUp, this);
	
	return true;
}

void HexView::Load(wxInputStream* stream)
{
	m_stream = stream;
	
//	m_stream->SeekI(0, wxFromEnd);
}

void HexView::OnSetCursor(wxSetCursorEvent& event)
{
	if (event.GetX() > 40 && event.GetY() > 40)
	{
		event.SetCursor(wxCURSOR_IBEAM);
	}
	else
		event.SetCursor(wxCURSOR_ARROW);
	
//	event.Skip();
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
	wxCoord rowHeight = offsetSize.GetHeight() + 2;
	
	// Header row
	pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	
	pdc.DrawText(_("Offset"), rowPos);
	rowPos.x += offsetSize.GetWidth();
	
	for (int i = 0; i < m_rowsize; ++i)
	{
		pdc.DrawText(wxString::Format("%.2X", i), rowPos);
		
		rowPos.x += byteSize.GetWidth();
	}
	
	rowPos.y += rowHeight;
	
	int offset = 0;
	while (offset < m_viewDataSize)
	{
		rowPos.x = 2;
		int rowsize = m_rowsize;
		if (offset + rowsize > m_viewDataSize)
			rowsize = m_viewDataSize - offset;
		
		// Draw offset
		pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		pdc.DrawText(wxString::Format("%.8X", offset), rowPos);
		rowPos.x += offsetSize.GetWidth();
		
		// Draw row values
		pdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		for (int i = 0; i < m_rowsize; ++i)
		{
			pdc.DrawText(wxString::Format("%.2X", m_viewData[offset + i]), rowPos);
			
			rowPos.x += byteSize.GetWidth();
		}
		
		// Draw row characters
		wxString str = wxString::From8BitData((const char*)(&m_viewData[offset]), rowsize);
		for (auto it = str.begin(); it != str.end(); ++it)
		{
			if (*it < 32)
				*it = '.';
		}
		
		pdc.DrawText(str, rowPos);
		
		rowPos.y += rowHeight;
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
