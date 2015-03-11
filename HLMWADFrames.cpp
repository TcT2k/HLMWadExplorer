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
	
	wxMenuItem* save;
	save = new wxMenuItem( file, wxID_SAVE, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( save );
	
	wxMenuItem* saveas;
	saveas = new wxMenuItem( file, wxID_SAVEAS, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( saveas );
	
	file->AppendSeparator();
	
	wxMenuItem* quit;
	quit = new wxMenuItem( file, wxID_EXIT, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( quit );
	
	m_menubar->Append( file, _("&File") ); 
	
	resource = new wxMenu();
	wxMenuItem* extract;
	extract = new wxMenuItem( resource, ID_EXTRACT, wxString( _("&Extract...") ) + wxT('\t') + wxT("Ctrl+E"), wxEmptyString, wxITEM_NORMAL );
	resource->Append( extract );
	extract->Enable( false );
	
	wxMenuItem* replace;
	replace = new wxMenuItem( resource, ID_REPLACE, wxString( _("&Replace...") ) + wxT('\t') + wxT("Ctrl+R"), wxEmptyString, wxITEM_NORMAL );
	resource->Append( replace );
	replace->Enable( false );
	
	resource->AppendSeparator();
	
	wxMenuItem* add;
	add = new wxMenuItem( resource, wxID_ADD, wxString( _("&Add...") ) + wxT('\t') + wxT("Ctrl+A"), wxEmptyString, wxITEM_NORMAL );
	resource->Append( add );
	add->Enable( false );
	
	wxMenuItem* deleteMenuItem;
	deleteMenuItem = new wxMenuItem( resource, wxID_DELETE, wxString( _("&Delete") ) + wxT('\t') + wxT("Del"), wxEmptyString, wxITEM_NORMAL );
	resource->Append( deleteMenuItem );
	deleteMenuItem->Enable( false );
	
	m_menubar->Append( resource, _("&Resource") ); 
	
	help = new wxMenu();
	wxMenuItem* about;
	about = new wxMenuItem( help, wxID_ABOUT, wxString( wxEmptyString ) , wxEmptyString, wxITEM_NORMAL );
	help->Append( about );
	
	m_menubar->Append( help, _("&Help") ); 
	
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
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( BaseExploreFrame::OnWindowClose ) );
	this->Connect( open->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnOpenClicked ) );
	this->Connect( save->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnSaveClicked ) );
	this->Connect( saveas->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnSaveAsClicked ) );
	this->Connect( quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnQuitClicked ) );
	this->Connect( extract->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnExtractClicked ) );
	this->Connect( replace->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnReplaceClicked ) );
	this->Connect( add->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnAddClicked ) );
	this->Connect( deleteMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnDeleteClicked ) );
	this->Connect( about->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnAboutClicked ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( BaseExploreFrame::OnFileListSelectionChanged ) );
	m_fileListCtrl->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( BaseExploreFrame::OnFileListDoubleClick ), NULL, this );
}

BaseExploreFrame::~BaseExploreFrame()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( BaseExploreFrame::OnWindowClose ) );
	this->Disconnect( wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnOpenClicked ) );
	this->Disconnect( wxID_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnSaveClicked ) );
	this->Disconnect( wxID_SAVEAS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnSaveAsClicked ) );
	this->Disconnect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnQuitClicked ) );
	this->Disconnect( ID_EXTRACT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnExtractClicked ) );
	this->Disconnect( ID_REPLACE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnReplaceClicked ) );
	this->Disconnect( wxID_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnAddClicked ) );
	this->Disconnect( wxID_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnDeleteClicked ) );
	this->Disconnect( wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BaseExploreFrame::OnAboutClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( BaseExploreFrame::OnFileListSelectionChanged ) );
	m_fileListCtrl->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( BaseExploreFrame::OnFileListDoubleClick ), NULL, this );
	
}
