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
	
	~HexView();
	
	bool Create(wxWindow *parent,
				wxWindowID winid = wxID_ANY,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize);
	
	void Clear();
	
	bool Load(const void* data, size_t dataSize);
	
	bool Load(wxInputStream& iStr);
	
private:
	unsigned char* m_data;
	size_t m_dataSize;
	int m_rowsize;
	int m_rowHeight;
	int m_charWidth;
	
	void CalcSize();
	
	void OnPaint(wxPaintEvent& event);
	
	void OnSetCursor(wxSetCursorEvent& event);
	
	void OnLeftDown(wxMouseEvent& event);
	
	void OnLeftUp(wxMouseEvent& event);
};

#endif // HexView__INCLUDED
