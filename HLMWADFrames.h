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
#include <wx/srchctrl.h>
#include <wx/toolbar.h>
#include <wx/timer.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/grid.h>

///////////////////////////////////////////////////////////////////////////

#define ID_RESTORE 1000
#define ID_EXTRACT 1001
#define ID_REPLACE 1002
#define ID_PATCH_APPLY 1003
#define ID_PATCH_PREPARE 1004
#define ID_PATCH_CREATE 1005

///////////////////////////////////////////////////////////////////////////////
/// Class BaseExploreFrame
///////////////////////////////////////////////////////////////////////////////
class BaseExploreFrame : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar;
		wxMenu* m_fileMenu;
		wxMenu* resource;
		wxMenu* m_patchMenu;
		wxMenu* help;
		wxToolBar* m_toolBar;
		wxSearchCtrl* m_searchCtrl;
		wxTimer m_searchTimer;
		wxSplitterWindow* m_mainSplitter;
		wxPanel* m_listPanel;
		wxDataViewCtrl* m_fileListCtrl;
		wxPanel* m_previewPanel;
		wxSimplebook* m_previewBookCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnWindowClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnOpenClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveAsClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRestoreClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuitClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExtractClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReplaceClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAddClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDeleteClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPatchApplyClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPatchPrepareClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPatchCreateClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAboutClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSearchCtrlButton( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSearchCtrlText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSearchCtrlEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSearchTimer( wxTimerEvent& event ) { event.Skip(); }
		virtual void OnFileListSelectionChanged( wxDataViewEvent& event ) { event.Skip(); }
		virtual void OnFileListDoubleClick( wxMouseEvent& event ) { event.Skip(); }
		
	
	public:
		
		BaseExploreFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 886,484 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~BaseExploreFrame();
		
		void m_mainSplitterOnIdle( wxIdleEvent& )
		{
			m_mainSplitter->SetSashPosition( 400 );
			m_mainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( BaseExploreFrame::m_mainSplitterOnIdle ), NULL, this );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class BaseTexturePackPanel
///////////////////////////////////////////////////////////////////////////////
class BaseTexturePackPanel : public wxPanel 
{
	private:
	
	protected:
		wxSplitterWindow* m_splitter2;
		wxPanel* m_panel3;
		wxListBox* m_textureListBox;
		wxPanel* m_panel4;
		wxStaticText* m_staticText1;
		wxSpinCtrl* m_frameSpinCtrl;
		wxStaticText* m_staticText2;
		wxColourPickerCtrl* m_colourPicker;
		wxPanel* m_framePanel;
		wxStaticBitmap* m_frameBitmap;
		wxStaticText* m_staticText8;
		wxSpinCtrl* m_frameDelaySpinCtrl;
		wxButton* m_exportGIFButton;
		wxBoxSizer* m_infoSizer;
		wxStaticText* m_staticText3;
		wxSpinCtrl* m_zoomSpinCtrl;
		wxStaticText* m_staticText4;
		wxStaticText* m_sizeStaticText;
		wxStaticText* m_staticText41;
		wxStaticText* m_offsetStaticText;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTextureListBoxSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFrameSpinCtrlChanged( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnFrameSpinCtrlEnterPressed( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnColourPickerChanged( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnExportGIFClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnZoomSpinCtrlChanged( wxSpinEvent& event ) { event.Skip(); }
		
	
	public:
		
		BaseTexturePackPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~BaseTexturePackPanel();
		
		void m_splitter2OnIdle( wxIdleEvent& )
		{
			m_splitter2->SetSashPosition( 180 );
			m_splitter2->Disconnect( wxEVT_IDLE, wxIdleEventHandler( BaseTexturePackPanel::m_splitter2OnIdle ), NULL, this );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImagePanel
///////////////////////////////////////////////////////////////////////////////
class ImagePanel : public wxPanel 
{
	private:
	
	protected:
	
	public:
		wxStaticBitmap* m_previewBitmap;
		
		ImagePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~ImagePanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TextPanel
///////////////////////////////////////////////////////////////////////////////
class TextPanel : public wxPanel 
{
	private:
	
	protected:
	
	public:
		wxTextCtrl* m_textCtrl;
		
		TextPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~TextPanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class BaseStringTablePanel
///////////////////////////////////////////////////////////////////////////////
class BaseStringTablePanel : public wxPanel 
{
	private:
	
	protected:
		wxGrid* m_grid;
	
	public:
		
		BaseStringTablePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~BaseStringTablePanel();
	
};

#endif //__HLMWADFRAMES_H__
