///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "HLMWADFrames.h"

///////////////////////////////////////////////////////////////////////////

BaseExploreFrame::BaseExploreFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	m_menubar = new wxMenuBar( 0 );
	file = new wxMenu();
	wxMenuItem* open;
	open = new wxMenuItem( file, wxID_OPEN, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( open );
	
	wxMenuItem* extract;
	extract = new wxMenuItem( file, ID_EXTRACT, wxString( _("&Extract...") ) + wxT('\t') + wxT("Ctrl+E"), wxEmptyString, wxITEM_NORMAL );
	file->Append( extract );
	
	file->AppendSeparator();
	
	wxMenuItem* quit;
	quit = new wxMenuItem( file, wxID_EXIT, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( quit );
	
	m_menubar->Append( file, _("&File") ); 
	
	this->SetMenuBar( m_menubar );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_fileListCtrl = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE );
	m_fileListNameColumn = m_fileListCtrl->AppendTextColumn( _("Name") ); 
	m_fileListSizeColumn = m_fileListCtrl->AppendTextColumn( _("Size") ); 
	bSizer2->Add( m_fileListCtrl, 1, wxEXPAND, 5 );
	
	m_previewPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_previewBitmap = new wxStaticBitmap( m_previewPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_previewBitmap, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	m_previewPanel->SetSizer( bSizer4 );
	m_previewPanel->Layout();
	bSizer4->Fit( m_previewPanel );
	bSizer2->Add( m_previewPanel, 2, wxEXPAND | wxALL, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( open->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnOpenClicked ) );
	this->Connect( extract->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnExtractClicked ) );
	this->Connect( quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnQuitClicked ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( BaseExploreFrame::OnFileListSelectionChanged ) );
	m_fileListCtrl->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( BaseExploreFrame::OnFileListDoubleClick ), NULL, this );
}

BaseExploreFrame::~BaseExploreFrame()
{
	// Disconnect Events
	this->Disconnect( wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnOpenClicked ) );
	this->Disconnect( ID_EXTRACT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnExtractClicked ) );
	this->Disconnect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnQuitClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( BaseExploreFrame::OnFileListSelectionChanged ) );
	m_fileListCtrl->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( BaseExploreFrame::OnFileListDoubleClick ), NULL, this );
	
}
