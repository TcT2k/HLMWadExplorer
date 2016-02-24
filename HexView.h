#ifndef HexView__INCLUDED
#define HexView__INCLUDED

#include <wx/scrolwin.h>
#include <wx/stream.h>

class HexView: public wxScrolledCanvas
{
public:
	
	HexView(wxWindow *parent,
			wxWindowID winid = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize)
	{
		Create(parent, winid, pos, size);
	}
	
	bool Create(wxWindow *parent,
				wxWindowID winid = wxID_ANY,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize);
	
	void Load(wxInputStream* stream);
	
private:
	wxInputStream* m_stream;
	int m_rowsize;
	wxFileOffset m_dataSize;
	unsigned char* m_viewData;
	int m_viewDataSize;
	
	void OnPaint(wxPaintEvent& event);
	
	void OnSetCursor(wxSetCursorEvent& event);
	
	void OnLeftDown(wxMouseEvent& event);
	
	void OnLeftUp(wxMouseEvent& event);
};

#endif // HexView__INCLUDED
