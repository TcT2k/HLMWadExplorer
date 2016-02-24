#ifndef HexView__INCLUDED
#define HexView__INCLUDED

#include <wx/control.h>
#include <wx/stream.h>

class HexRowView;
class wxPanel;

class HexView: public wxControl
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
	wxPanel* m_header;
	HexRowView* m_rowView;
	unsigned char* m_data;
	size_t m_dataSize;
	int m_rowsize;
	
	void OnPaintHeader(wxPaintEvent& evt);

	friend class HexRowView;
};

#endif // HexView__INCLUDED
