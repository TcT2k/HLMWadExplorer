///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __HLMWADFRAMES_H__
#define __HLMWADFRAMES_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/statbmp.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

#define ID_EXTRACT 1000

///////////////////////////////////////////////////////////////////////////////
/// Class BaseExploreFrame
///////////////////////////////////////////////////////////////////////////////
class BaseExploreFrame : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar;
		wxMenu* file;
		wxMenu* help;
		wxDataViewListCtrl* m_fileListCtrl;
		wxDataViewColumn* m_fileListNameColumn;
		wxDataViewColumn* m_fileListSizeColumn;
		wxPanel* m_previewPanel;
		wxStaticBitmap* m_previewBitmap;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOpenClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveAsClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExtractClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuitClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAboutClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileListSelectionChanged( wxDataViewEvent& event ) { event.Skip(); }
		virtual void OnFileListDoubleClick( wxMouseEvent& event ) { event.Skip(); }
		
	
	public:
		
		BaseExploreFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 886,484 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~BaseExploreFrame();
	
};

#endif //__HLMWADFRAMES_H__
