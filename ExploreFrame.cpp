/*
** HLMWadExplorer
**
** Copyright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include "ExploreFrame.h"

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/mstream.h>
#include <wx/busyinfo.h>
#include <wx/aboutdlg.h>
#include <wx/textdlg.h>
#include <wx/config.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#if defined(__WXMSW__)
#include <wx/msw/registry.h>
#endif

#include "TexturePack.h"
#include "TexturePackPanel.h"
#include "StringTablePanel.h"
#include "MergeDialog.h"
#include "VersionInfo.h"

class FileDataModel : public wxDataViewModel
{
public:
	FileDataModel(WADArchive* archive) :
		m_archive(archive)
	{

	}

	virtual unsigned int GetColumnCount() const
	{
		return 2;
	}

	// return type as reported by wxVariant
	virtual wxString GetColumnType(unsigned int col) const
	{
		return wxString();
	}

	void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const
	{
		WADDirEntry* dir = static_cast<WADDirEntry*>(item.GetID());
		switch (col)
		{
			case 0:
				variant = dir->GetName();
				break;
			case 1:
				if (dir->GetEntry())
					variant = wxFileName::GetHumanReadableSize(wxULongLong(dir->GetEntry()->GetSize()));
				else
					variant = wxString();
				break;
		}

	}

	bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
	{
		return false;
	}

	bool IsEnabled(const wxDataViewItem &item, unsigned int col) const
	{
		return true;
	}

	wxDataViewItem GetParent(const wxDataViewItem &item) const
	{
		// the invisible root node has no parent
		if (!item.IsOk())
			return wxDataViewItem(0);

		WADDirEntry* dir = static_cast<WADDirEntry*>(item.GetID());

		if (dir == m_archive->GetRootDir())
			return wxDataViewItem(0);

		return wxDataViewItem((void*)dir->GetParent());
	}

	bool IsContainer(const wxDataViewItem &item) const
	{
		// the invisble root node can have children
		if (!item.IsOk())
			return true;

		WADDirEntry* dir = static_cast<WADDirEntry*>(item.GetID());
		return !dir->GetEntry();
	}

	unsigned int GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const
	{
		WADDirEntry* dir = static_cast<WADDirEntry*>(parent.GetID());
		if (!dir)
			dir = m_archive->GetRootDir();

		for (size_t i = 0; i < dir->GetChildCount(); ++i)
		{
			WADDirEntry* childDir = dir->GetChild(i);
			array.Add(wxDataViewItem(childDir));
		}

		return dir->GetChildCount();
	}

	bool GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const
	{
		WADDirEntry* dir = static_cast<WADDirEntry*>(item.GetID());
		if (dir->GetEntry() && dir->GetEntry()->GetStatus() != WADArchiveEntry::Entry_Original)
			attr.SetBold(true);

		return true;
	}


private:
	WADArchive* m_archive;
};


ExploreFrame::ExploreFrame( wxWindow* parent ):
	BaseExploreFrame( parent ),
	m_archive(NULL)
{
	UpdateTitle();
#if defined(__WXMSW__)
	SetIcon(wxIcon("APPICON"));
#endif

	m_menubar->Enable(ID_EXTRACT, false);
	m_toolBar->EnableTool(ID_EXTRACT, false);
	m_menubar->Enable(wxID_SAVE, false);
	m_toolBar->EnableTool(wxID_SAVE, false);
	m_menubar->Enable(wxID_SAVEAS, false);
	m_ignoreSearch = true;

	m_toolBar->InsertStretchableSpace(5);
	m_toolBar->Realize();

	m_fileListCtrl->AppendTextColumn(_("Name"), 0, wxDATAVIEW_CELL_INERT, 250);
	m_fileListCtrl->AppendTextColumn(_("Size"), 1, wxDATAVIEW_CELL_INERT, -1, wxALIGN_RIGHT);

	m_fileHistory.UseMenu(m_fileMenu);
	{
		wxConfigPathChanger pathChanger(wxConfigBase::Get(), "/FileHistory/PatchFiles/");
		m_fileHistory.Load(*wxConfigBase::Get());
	}

	m_previewBookCtrl->AddPage(new wxPanel(m_previewBookCtrl), "General", true);
	m_previewBookCtrl->AddPage(new ImagePanel(m_previewBookCtrl), "Image", false);
	m_previewBookCtrl->AddPage(new TexturePackPanel(m_previewBookCtrl), "Texture", false);
	m_previewBookCtrl->AddPage(new TextPanel(m_previewBookCtrl), "Text", false);
	m_previewBookCtrl->AddPage(new StringTablePanel(m_previewBookCtrl), "String Table", false);

	Bind(wxEVT_MENU, &ExploreFrame::OnRecentFileClicked, this, wxID_FILE1, wxID_FILE9);

	wxPersistenceManager::Get().RegisterAndRestore(this);
}

ExploreFrame::~ExploreFrame()
{
}

void ExploreFrame::SelectItem(WADDirEntry* entry)
{
	if (!entry)
		entry = m_archive->GetRootDir()->GetChild(0);

	wxDataViewItem item(entry);
	//m_fileListCtrl->UnselectAll();
	//m_fileListCtrl->Select(item);
	//m_fileListCtrl->EnsureVisible(item);
	wxDataViewEvent evt;
	OnFileListSelectionChanged(evt);
}

void ExploreFrame::OnAboutClicked( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutInfo;
	aboutInfo.SetName(wxTheApp->GetAppDisplayName());
	aboutInfo.SetDescription(_("HLM Wad Extractor"));
	aboutInfo.SetCopyright("(C) 2015-2016");
	aboutInfo.SetWebSite("https://github.com/TcT2k/HLMWadExplorer");
	aboutInfo.AddDeveloper("Tobias Taschner");
	aboutInfo.AddArtist("LightVelox");
	aboutInfo.AddArtist("Everaldo Coelho");
	aboutInfo.SetVersion(APP_VERSION);
	wxAboutBox(aboutInfo);
}

void ExploreFrame::OnSearchCtrlButton(wxCommandEvent& event)
{
	OnSearchCtrlEnter(event);
}

void ExploreFrame::OnSearchCtrlText(wxCommandEvent& event)
{
	if (m_searchTimer.IsRunning())
		m_searchTimer.Stop();

	if (!m_ignoreSearch)
		m_searchTimer.StartOnce(500);
}

void ExploreFrame::OnSearchCtrlEnter(wxCommandEvent& event)
{
	if (m_searchTimer.IsRunning())
		m_searchTimer.Stop();

	ApplyFilter(m_searchCtrl->GetValue());
}

void ExploreFrame::OnSearchTimer(wxTimerEvent& event)
{
	ApplyFilter(m_searchCtrl->GetValue());
}

void ExploreFrame::ApplyFilter(const wxString& filter)
{
	if (!m_archive || m_ignoreSearch)
		return;

	wxLogDebug("Applying filter: %s", filter);

	if (!m_archive->ApplyFilter(filter))
		wxLogError(_("No items found"));

	m_fileListCtrl->GetModel()->Cleared();
	if (!m_archive->GetRootDir()->empty())
		SelectItem(NULL);
}

void ExploreFrame::OnWindowClose( wxCloseEvent& event )
{
	if (m_archive && m_archive->IsModified())
	{
		wxMessageDialog msgDlg(this, _("Do you wan't to save unsaved changes?"), _("Warning"), wxICON_WARNING | wxYES_NO | wxCANCEL);
		msgDlg.SetYesNoLabels(_("Save"), _("Don't Save"));

		switch (msgDlg.ShowModal())
		{
			case wxID_YES:
			{
				wxCommandEvent evt(wxID_SAVE);
				OnSaveClicked(evt);
				event.Skip();
				break;
			}
			case wxID_NO:
				event.Skip();
				break;
			default:
				event.Veto();
				break;
		}
	} else
		event.Skip();
}

void ExploreFrame::OnRecentFileClicked(wxCommandEvent& event)
{
	size_t historyIndex = event.GetId() - m_fileHistory.GetBaseId();
	wxString filename = m_fileHistory.GetHistoryFile(historyIndex);
	wxFileName testFN(filename);
	if (!testFN.Exists())
	{
		wxLogError(_("File not found: %s"), filename);
		m_fileHistory.RemoveFileFromHistory(historyIndex);
	} else
		OpenFile(filename);
}

wxString ExploreFrame::GetGameBasePath() const
{
	wxString hlmPath;
#if defined(__WXOSX__)
	hlmPath = wxFileName::GetHomeDir() + "/Library/Application Support/Steam/steamapps/common/Hotline Miami 2/HotlineMiami2.app/Contents/Resources/";
#elif defined(__WXMSW__)
	wxRegKey regKey(wxRegKey::HKCU, "Software\\Valve\\Steam");
	if (regKey.Open(wxRegKey::Read))
	{
		regKey.QueryValue("SteamPath", hlmPath);

		hlmPath += "\\SteamApps\\Common\\Hotline Miami 2";
}
#endif
	return hlmPath;
}

wxString ExploreFrame::GetModPath() const
{
	wxFileName modPathFN;
	
#if defined(__WXOSX__)
	modPathFN.Assign(wxFileName::GetHomeDir() + "/Library/Application Support/HotlineMiami2/Mods", "");
#elif defined(__WXMSW__)
	modPathFN.Assign(wxStandardPaths::Get().GetDocumentsDir(), "");
	modPathFN.AppendDir("My Games");
	modPathFN.AppendDir("HotlineMiami2");
	modPathFN.AppendDir("Mods");
#else
	modPathFN.Assign(wxFileName::GetHomeDir() + "/HotlineMiami2/Mods", "");
#endif
	
	modPathFN.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	
	return modPathFN.GetFullPath();
}

void ExploreFrame::OnOpenClicked( wxCommandEvent& event )
{
	wxFileDialog fileDlg(this, _("Select patch WAD file"), GetGameBasePath(), wxString(), "*.patchwad", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);

	if (fileDlg.ShowModal() == wxID_OK)
	{
		OpenFile(fileDlg.GetPath());
	}
}

bool ExploreFrame::OpenBaseFile(bool forceSelection)
{
	bool doSelect = forceSelection;

	wxString prevBasePath = wxConfigBase::Get()->Read("BaseFile");

	wxFileName baseFileName;
	if (prevBasePath.empty())
		baseFileName.Assign(GetGameBasePath(), "hlm2_data_desktop.wad");
	else
		baseFileName.Assign(prevBasePath);

	if (!baseFileName.Exists())
	{
		wxMessageDialog msgDlg(NULL, _("Could not find base game WAD. Please select main WAD to continue."),
			_("Warning"), wxICON_WARNING | wxOK | wxCANCEL | wxCENTER);
		msgDlg.SetOKLabel(_("Select"));
		if (msgDlg.ShowModal() != wxID_OK)
			return false;

		doSelect = true;
	}

	if (doSelect)
	{
		wxFileDialog fileDlg(this, _("Select base game WAD file"), "", baseFileName.GetFullPath(), "*.wad", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
		if (fileDlg.ShowModal() != wxID_OK)
			return false;

		baseFileName.Assign(fileDlg.GetPath());
	}

	m_ignoreSearch = true;
	m_searchCtrl->SetValue("");
	m_searchCtrl->Enable();
	m_ignoreSearch = false;

	m_archive = new WADArchive(baseFileName.GetFullPath());
	wxObjectDataPtr<FileDataModel> model(new FileDataModel(m_archive.get()));
	m_fileListCtrl->UnselectAll();
	m_fileListCtrl->AssociateModel(model.get());
	m_fileListCtrl->Refresh();
	m_menubar->Enable(ID_EXTRACT, true);
	m_toolBar->EnableTool(ID_EXTRACT, true);
	m_menubar->Enable(wxID_ADD, !m_archive->GetReadOnly());
	if (!m_archive->GetRootDir()->empty())
	{
		SelectItem(NULL);
		m_fileListCtrl->SetFocus();
	}

	wxConfigBase::Get()->Write("BaseFile", baseFileName.GetFullPath());

	return true;
}

void ExploreFrame::OpenFile(const wxString& filename)
{
	m_patchFileName.clear();

	wxFileName patchFN(filename);
	if (!patchFN.Exists())
	{
		wxLogError(_("File not found: %s"), patchFN.GetFullPath());
		UpdateTitle();
		return;
	}

	if (!m_archive->LoadPatch(filename))
		return;

	m_patchFileName = filename;

	UpdateTitle();

	m_fileHistory.AddFileToHistory(filename);

	wxConfigPathChanger pathChanger(wxConfigBase::Get(), "/FileHistory/PatchFiles/");
	m_fileHistory.Save(*wxConfigBase::Get());
}

void ExploreFrame::OnSaveClicked( wxCommandEvent& event )
{
	if (m_patchFileName.empty())
	{
		OnSaveAsClicked(event);
		return;
	}
	else
		CreatePatch();
}

void ExploreFrame::OnSaveAsClicked( wxCommandEvent& event )
{
	wxString savePath;
	if (m_patchFileName.empty())
		savePath = GetModPath();

	wxFileDialog fileDlg(this, _("Select destination patch filename"), savePath, m_patchFileName, "*.patchwad", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		m_patchFileName = fileDlg.GetPath();
		if (CreatePatch())
		{
			m_fileHistory.AddFileToHistory(m_patchFileName);

			wxConfigPathChanger pathChanger(wxConfigBase::Get(), "/FileHistory/PatchFiles/");
			m_fileHistory.Save(*wxConfigBase::Get());
		}
	}
}

void ExploreFrame::OnMergeClicked( wxCommandEvent& event )
{
	MergeDialog dlg(this);
	dlg.SetBaseWAD(m_archive->GetFileName());
	if (dlg.ShowModal() == wxID_OK)
	{
		WADArchive patchArchive(dlg.GetPatchWAD());

		wxMessageDialog msgDlg(this, wxString::Format(_("Applying patch\n\nThis patch contains %d new/modified files.\nAre you sure you want to apply this patch?"),
													  patchArchive.GetEntryCount()), _("Warning"),
							   wxICON_WARNING | wxOK | wxCANCEL | wxCANCEL_DEFAULT);
		msgDlg.SetOKLabel(_("Patch"));

		if (msgDlg.ShowModal() == wxID_OK)
		{
			wxBusyCursor busyCursor;
			wxBusyInfo busyInfo(_("Merging patch..."));

			WADArchive baseArchive(dlg.GetBaseWAD());
			baseArchive.LoadPatch(dlg.GetPatchWAD());
			// Save archive
			if (baseArchive.Write())
			{
				wxLogInfo(_("Patch merged"));
			}
		}
	}
}

void ExploreFrame::OnSwitchBaseWadClicked(wxCommandEvent& event)
{
	OpenBaseFile(true);
}

bool ExploreFrame::CreatePatch()
{
	bool patchCreated;
	{
		wxBusyInfo busyInfo(_("Writing file..."));
		wxBusyCursor busyCursor;
		patchCreated = m_archive->CreatePatch(m_patchFileName);
	}

	if (patchCreated)
	{
		UpdateTitle();
		wxMessageDialog msgDlg(this, _("The patch WAD has been saved\n\nDo you want to open the folder it was saved to?"), _("Information"), wxICON_INFORMATION | wxOK | wxCANCEL);
		msgDlg.SetOKCancelLabels(_("Open Folder"), wxID_CLOSE);
		if (msgDlg.ShowModal() == wxID_OK)
		{
			wxFileName patchFN(m_patchFileName);
			patchFN.SetFullName("");
			wxLaunchDefaultApplication(patchFN.GetFullPath());
		}
	}

	return patchCreated;
}

const WADArchiveEntry* ExploreFrame::GetSelectedEntry() const
{
	WADDirEntry* dir = static_cast<WADDirEntry*>(m_fileListCtrl->GetSelection().GetID());
	if (dir)
		return dir->GetEntry();
	else
		return NULL;
}

void ExploreFrame::ExtractPak(const WADArchiveEntry* entry)
{
	wxString extractFolder = wxStandardPaths::Get().GetDocumentsDir();
	wxDirDialog dirDlg(this, _("Select folder to PAK file contents to"), extractFolder, wxDD_DEFAULT_STYLE);
	if (dirDlg.ShowModal() == wxID_OK)
	{
		wxMemoryOutputStream oStr;
		m_archive->Extract(*entry, oStr);
		wxMemoryInputStream iStr(oStr);
		wxUint32 fileCount;
		iStr.Read(&fileCount, sizeof(fileCount));
		wxLogDebug("%d files in PAK", fileCount);

		wxFileName pakFN(entry->GetFileName(), wxPATH_UNIX);

		// Create base extraction path
		wxFileName extFolder(extractFolder, "");
		extFolder.AppendDir(pakFN.GetName() + "_expak");
		extFolder.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

		wxVector<wxUint32> offsets;
		offsets.reserve(fileCount);
		for (int i = 0; i < fileCount; ++i)
		{
			wxUint32 offset;
			iStr.Read(&offset, sizeof(offset));
			wxLogDebug("%d: %d", i, offset);
			offsets.push_back(offset);
		}
		offsets.push_back(entry->GetSize());

		char* rawData = new char[entry->GetSize()];
		oStr.CopyTo(rawData, entry->GetSize());

		// Extract files
		for (int i = 0; i < fileCount; ++i)
		{
			wxString name = wxString::Format("%d", i);
			wxFileName dataFN(extFolder.GetFullPath(), name);

			wxTempFileOutputStream fileStr(dataFN.GetFullPath());
			fileStr.Write(&rawData[offsets[i]],
				offsets[i + 1] - offsets[i]);

			fileStr.Commit();
		}

		delete[] rawData;

		wxMessageDialog msgDlg(this, _("The PAK contents have been extracted\n\nDo you want to open the folder it was extracted to?"), _("Information"), wxICON_INFORMATION | wxOK | wxCANCEL);
		msgDlg.SetOKCancelLabels(_("Open Folder"), wxID_CLOSE);
		if (msgDlg.ShowModal() == wxID_OK)
			wxLaunchDefaultApplication(extFolder.GetFullPath());
	}
}

void ExploreFrame::OnExtractClicked( wxCommandEvent& event )
{
	if (m_fileListCtrl->GetSelectedItemsCount() == 0)
	{
		wxLogError(_("Select file(s) to extract"));
		return;
	}

	wxString extractFolder = wxStandardPaths::Get().GetDocumentsDir();
	if (m_fileListCtrl->GetSelectedItemsCount() > 1)
	{
		wxDirDialog dirDlg(this, _("Select folder to extract files to"), extractFolder, wxDD_DEFAULT_STYLE);
		if (dirDlg.ShowModal() == wxID_OK)
		{
			wxBusyInfo busyInfo(_("Extracting data..."));
			wxBusyCursor busyCursor;

			wxDataViewItemArray selectedItems;
			m_fileListCtrl->GetSelections(selectedItems);

			for (auto it = selectedItems.begin(); it != selectedItems.end(); ++it)
			{
				WADDirEntry* dir = static_cast<WADDirEntry*>(it->GetID());
				const WADArchiveEntry* entry = dir->GetEntry();
				if (!entry)
					continue;

				wxFileName targetFileName(entry->GetFileName(), wxPATH_UNIX);
				targetFileName.Normalize(wxPATH_NORM_ALL, dirDlg.GetPath());
				wxLogDebug("Extracting to: %s", targetFileName.GetFullPath());

				if (!targetFileName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
					break;
				m_archive->Extract(*entry, targetFileName.GetFullPath());
			}
		}
		wxLogInfo(_("Files extracted to %s"), dirDlg.GetPath());
	} else {
		const WADArchiveEntry* entry = GetSelectedEntry();
		wxASSERT(entry);

		wxFileName fn(entry->GetFileName(), wxPATH_UNIX);
		if (fn.GetExt().IsSameAs("pak", false) && !wxGetKeyState(WXK_SHIFT))
		{
			// Extract pak to single files
			ExtractPak(entry);

			return;
		}

		wxString fileName = fn.GetFullName();
		wxFileDialog fileDlg(this, _("Select target for file extraction"), extractFolder, fileName, 
			wxFileSelectorDefaultWildcardStr, wxFD_OVERWRITE_PROMPT | wxFD_SAVE);
		if (fileDlg.ShowModal() == wxID_OK)
		{
			wxBusyInfo busyInfo(_("Extracting data..."));
			wxBusyCursor busyCursor;

			m_archive->Extract(*entry, fileDlg.GetPath());
		}

		wxLogInfo(_("File extracted to %s"), fileDlg.GetPath());
	}
}

void ExploreFrame::OnReplaceClicked( wxCommandEvent& event )
{
	const WADArchiveEntry* entry = GetSelectedEntry();
	wxASSERT(entry != NULL);
	wxFileName entryFN(entry->GetFileName());
	
	wxFileDialog fileDlg(this, wxString::Format(_("Select file to replace %s"), entry->GetFileName()), wxString(),
						 entryFN.GetFullName(), "*." + entryFN.GetExt(), wxFD_DEFAULT_STYLE  | wxFD_FILE_MUST_EXIST);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		if (entryFN.GetExt().IsSameAs("png", false))
		{
			wxString imgErrors;

			// Do some sanity checks on the replacement image
			wxImage replaceImage(fileDlg.GetPath());
			if (replaceImage.IsOk())
			{
				if (replaceImage.HasAlpha())
					imgErrors += _("- Image may not have an alpha mask\n");

				wxMemoryOutputStream oStr;
				m_archive->Extract(*entry, oStr);
				wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
				wxMemoryInputStream iStr(buffer->GetBufferStart(), buffer->GetBufferSize());
				wxImage orgImage(iStr);
				if (orgImage.GetSize() != replaceImage.GetSize())
					imgErrors += wxString::Format(_("- Image has to be %dx%d in size\n"), orgImage.GetSize().x, orgImage.GetSize().y);

				if (orgImage.HasMask() != replaceImage.HasMask())
					imgErrors += _("- Image must have transparency\n");
			}
			else
				imgErrors = _("Image could not be loaded");

			if (!imgErrors.empty())
			{
				wxMessageDialog msgDlg(this, wxString::Format(
					_("Image Errors\n\nThe replacement image contains erros which might crash the game\n\n%s\n\nDo you want't to replace anyway?"), imgErrors),
					_("Warning"), wxICON_WARNING | wxYES_NO | wxNO_DEFAULT);
				msgDlg.SetYesNoLabels(_("Replace"), _("Don't Replace"));
				if (msgDlg.ShowModal() != wxID_YES)
					return;
			}
		}

		WADDirEntry* dir = static_cast<WADDirEntry*>(m_fileListCtrl->GetSelection().GetID());
		m_archive->Replace(dir, fileDlg.GetPath());
		m_fileListCtrl->GetModel()->ItemChanged(wxDataViewItem(dir));
		wxDataViewEvent evt(wxEVT_DATAVIEW_SELECTION_CHANGED);
		OnFileListSelectionChanged(evt);

		UpdateTitle();
	}
}

void ExploreFrame::OnAddClicked( wxCommandEvent& event )
{
	wxFileDialog fileDlg(this, _("Select file to add"), wxString(), wxString(), "*.*", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		wxFileName newFN(fileDlg.GetPath());
		wxString newEntryName = newFN.GetFullName();
		
		wxTextEntryDialog dlg(this, _("Enter filename in WAD"), _("Add resource"), newEntryName);
		if (dlg.ShowModal() == wxID_OK)
		{
			newEntryName = dlg.GetValue();
			WADArchiveEntry entry(newEntryName, fileDlg.GetPath());
			entry.SetStatus(WADArchiveEntry::Entry_Added);
			m_archive->Add(entry);
			// TODO: replace
			//static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->RowAppended();

			UpdateTitle();
		}
	}
}

void ExploreFrame::OnDeleteClicked( wxCommandEvent& event )
{
	const WADArchiveEntry* entry = GetSelectedEntry();
	wxASSERT(entry != NULL);
	if (wxMessageBox(wxString::Format(_("Are you sure you want to delete %s?"), entry->GetFileName()),
									  _("Warning"), wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxYES)
	{
		size_t index = (size_t) m_fileListCtrl->GetSelection().GetID() - 1;
		m_archive->Remove(index);
		// TODO:: replace
		//static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->RowDeleted(index);
		wxDataViewEvent evt(wxEVT_DATAVIEW_SELECTION_CHANGED);
		OnFileListSelectionChanged(evt);

		UpdateTitle();
	}
}

void ExploreFrame::OnQuitClicked( wxCommandEvent& event )
{
	Close();
}

void ExploreFrame::OnFileListSelectionChanged( wxDataViewEvent& event )
{
	const WADArchiveEntry* entry = GetSelectedEntry();
	bool fileSelected = (entry != NULL);
	m_menubar->Enable(ID_REPLACE, fileSelected && m_fileListCtrl->GetSelectedItemsCount() > 0 && !m_archive->GetReadOnly());
	m_menubar->Enable(ID_EXTRACT, fileSelected || m_fileListCtrl->GetSelectedItemsCount() > 1);
	m_toolBar->EnableTool(ID_EXTRACT, fileSelected || m_fileListCtrl->GetSelectedItemsCount() > 1);
	m_menubar->Enable(wxID_DELETE, fileSelected && m_fileListCtrl->GetSelectedItemsCount() > 0 && !m_archive->GetReadOnly());
	
	if (m_fileListCtrl->GetSelectedItemsCount() != 1)
		return;

	wxBusyCursor busyCursor;

	m_menubar->Enable(wxID_DELETE, entry && entry->GetStatus() == WADArchiveEntry::Entry_Added);
	if (!entry)
	{
		// Directory selected
		m_previewBookCtrl->ChangeSelection(0);
		return;
	}

	wxLogDebug("Selected: %s", entry->GetFileName());

	wxFileName fn(entry->GetFileName(), wxPATH_UNIX);
	wxString fileExt = fn.GetExt();

	if (fileExt.IsSameAs("png", false) ||
		fileExt.IsSameAs("jpg", false) ||
		fileExt.IsSameAs("tga", false))
	{
		m_previewBookCtrl->ChangeSelection(1);

		ImagePanel* imgPanel = (ImagePanel*) m_previewBookCtrl->GetCurrentPage();
		imgPanel->m_previewBitmap->SetBitmap(m_archive->ExtractBitmap(*entry));
		imgPanel->Layout();
	}
	else if (fileExt.IsSameAs("meta", false))
	{
		m_previewBookCtrl->ChangeSelection(2);

		// Load texture pack
		wxMemoryOutputStream oStr;
		m_archive->Extract(*entry, oStr);
		wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
		wxMemoryInputStream iStr(buffer->GetBufferStart(), buffer->GetBufferSize());

		wxFileName imageFN(entry->GetFileName(), wxPATH_UNIX);
		imageFN.SetExt("png");
		WADArchiveEntry imgEntry = m_archive->GetEntry(imageFN.GetFullPath(wxPATH_UNIX));

		TexturePackPanel* texPanel = (TexturePackPanel*)m_previewBookCtrl->GetCurrentPage();
		texPanel->LoadTexture(iStr, m_archive->ExtractBitmap(imgEntry));
	}
	else if (fileExt.IsSameAs("fnt", false) ||
			fileExt.IsSameAs("ini", false) ||
			fileExt.IsSameAs("vsh", false) ||
			fileExt.IsSameAs("fsh", false) ||
			fileExt.IsSameAs("cg", false) )
	{
		m_previewBookCtrl->ChangeSelection(3);
		TextPanel* textPanel = (TextPanel*) m_previewBookCtrl->GetCurrentPage();

		wxMemoryOutputStream oStr;
		m_archive->Extract(*entry, oStr);
		wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
		wxString text((const char*)buffer->GetBufferStart(), buffer->GetBufferSize());
#if !defined(__WXMSW__)
		text.Replace("\r\n", "\n");
#endif
		textPanel->m_textCtrl->SetValue(text);
	}
	else if (fn.GetName().IsSameAs("hlm2_localization"))
	{
		m_previewBookCtrl->ChangeSelection(4);

		StringTablePanel* stringPanel = (StringTablePanel*) m_previewBookCtrl->GetCurrentPage();
		wxMemoryOutputStream oStr;
		m_archive->Extract(*entry, oStr);
		wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
		stringPanel->LoadStringTable(buffer->GetBufferStart(), buffer->GetBufferSize());
	}
	else
		m_previewBookCtrl->ChangeSelection(0);
}

void ExploreFrame::OnFileListDoubleClick( wxMouseEvent& event )
{
// TODO: Implement OnFileListDoubleClick
}

void ExploreFrame::UpdateTitle()
{
	bool modified = m_archive.get() && m_archive->IsModified();

	wxString modStr;

#if defined(__WXOSX__)
	OSXSetModified(modified);
#else
	if (modified)
		modStr = "*";
#endif

	bool saveAvailable = m_archive.get() && !m_archive->GetReadOnly() && modified;
	m_menubar->Enable(wxID_SAVE, saveAvailable);
	m_toolBar->EnableTool(wxID_SAVE, saveAvailable);
	m_menubar->Enable(wxID_SAVEAS, saveAvailable);

	wxString newTitle = modStr;

	if (!m_patchFileName.empty())
	{
		wxFileName archiveFN(m_patchFileName);
		newTitle += wxString::Format("%s - %s", archiveFN.GetName(), wxTheApp->GetAppDisplayName());
	} 
	else
		newTitle += wxTheApp->GetAppDisplayName();

	SetTitle(newTitle);
}

