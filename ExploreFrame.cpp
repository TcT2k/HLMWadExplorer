/*
** HLMWadExplorer
**
** Coypright (C) 2015 Tobias Taschner <github@tc84.de>
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
#if defined(__WXMSW__)
#include <wx/msw/registry.h>
#endif

#include "TexturePack.h"
#include "TexturePackPanel.h"
#include "StringTablePanel.h"
#include "VersionInfo.h"

class FileDataModel : public wxDataViewVirtualListModel
{
public:
	FileDataModel(ExploreFrame* frame, WADArchive* archive) :
		wxDataViewVirtualListModel(archive->GetFilteredEntryCount()),
		m_frame(frame),
		m_archive(archive)
	{

	}

	virtual unsigned int GetColumnCount() const
	{
		return 3;
	}

	// return type as reported by wxVariant
	virtual wxString GetColumnType(unsigned int col) const
	{
		if (col == 0)
			return "bool";
		else
			return wxString();
	}

	void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
	{
		const WADArchiveEntry& entry = m_archive->GetFilteredEntry(row);
		switch (col)
		{
			case 0:
				variant = m_frame->m_patchEntries.find(row) != m_frame->m_patchEntries.end();
				break;
			case 1:
				variant = entry.GetFileName();
				break;
			case 2:
				variant = wxFileName::GetHumanReadableSize(wxULongLong(entry.GetSize()));
				break;
		}

	}

	bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
	{
		if (col == 0)
		{
			if (variant.GetBool())
				m_frame->m_patchEntries.insert(row);
			else
				m_frame->m_patchEntries.erase(row);
			m_frame->m_menubar->Enable(ID_PATCH_CREATE, !m_frame->m_patchEntries.empty());

			return true;
		} else
			return false;
	}

private:
	ExploreFrame* m_frame;
	WADArchive* m_archive;
};


ExploreFrame::ExploreFrame( wxWindow* parent ):
	BaseExploreFrame( parent ),
	m_archive(NULL),
	m_backupAvailable(false)
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
	m_menubar->Enable(ID_PATCH_APPLY, false);
	m_toolBar->EnableTool(ID_PATCH_APPLY, false);
	m_menubar->Enable(ID_PATCH_PREPARE, false);
	m_menubar->Enable(ID_PATCH_CREATE, false);
	m_ignoreSearch = true;

	m_toolBar->InsertStretchableSpace(6);
	m_toolBar->Realize();

	m_fileListToggleColumn = m_fileListCtrl->AppendToggleColumn(_("Patch"), 0, wxDATAVIEW_CELL_ACTIVATABLE, 
		wxDLG_UNIT(this, wxSize(16, -1)).GetWidth(), wxALIGN_CENTER, wxCOL_HIDDEN);
	m_fileListCtrl->AppendTextColumn(_("Name"), 1, wxDATAVIEW_CELL_INERT, 250);
	m_fileListCtrl->AppendTextColumn(_("Size"), 2, wxDATAVIEW_CELL_INERT, -1, wxALIGN_RIGHT);

	m_fileHistory.UseMenu(m_fileMenu);
	m_fileHistory.Load(*wxConfigBase::Get());
	if (m_fileHistory.GetCount() == 0)
	{
		// Initialize with default file names
		wxFileName defaultFN(GetGameBasePath(), "hlm2_data_desktop.wad");
		m_fileHistory.AddFileToHistory(defaultFN.GetFullPath());
		defaultFN.SetName("hlm2_patch_desktop");
		m_fileHistory.AddFileToHistory(defaultFN.GetFullPath());
	}

	m_previewBookCtrl->AddPage(new wxPanel(m_previewBookCtrl), "General", true);
	m_previewBookCtrl->AddPage(new ImagePanel(m_previewBookCtrl), "Image", false);
	m_previewBookCtrl->AddPage(new TexturePackPanel(m_previewBookCtrl), "Texture", false);
	m_previewBookCtrl->AddPage(new TextPanel(m_previewBookCtrl), "Text", false);
	m_previewBookCtrl->AddPage(new StringTablePanel(m_previewBookCtrl), "String Table", false);

	m_preparingPatch = false;

	Bind(wxEVT_MENU, &ExploreFrame::OnRecentFileClicked, this, wxID_FILE1, wxID_FILE9);

	wxPersistenceManager::Get().RegisterAndRestore(this);
}

ExploreFrame::~ExploreFrame()
{
}

void ExploreFrame::SelectItem(size_t index)
{
	wxDataViewItem item((void*)(index + 1));
	m_fileListCtrl->UnselectAll();
	m_fileListCtrl->Select(item);
	m_fileListCtrl->EnsureVisible(item);
	wxDataViewEvent evt;
	OnFileListSelectionChanged(evt);
}

void ExploreFrame::OnAboutClicked( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutInfo;
	aboutInfo.SetName(wxTheApp->GetAppDisplayName());
	aboutInfo.SetDescription(_("HLM Wad Extractor"));
	aboutInfo.SetCopyright("(C) 2015");
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

	static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->Reset(m_archive->GetFilteredEntryCount());
	if (m_archive->GetFilteredEntryCount() > 0)
		SelectItem(0);
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
				wxBusyInfo busyInfo(_("Writing file..."));
				wxBusyCursor busyCursor;
				m_archive->Save();
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

void ExploreFrame::OnOpenClicked( wxCommandEvent& event )
{
	wxFileDialog fileDlg(this, _("Select WAD file"), GetGameBasePath(), wxString(), "*.wad", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);

	if (fileDlg.ShowModal() == wxID_OK)
	{
		OpenFile(fileDlg.GetPath());
	}
}

void ExploreFrame::OpenFile(const wxString& filename)
{
	m_ignoreSearch = true;
	m_searchCtrl->SetValue("");
	m_searchCtrl->Enable();
	m_ignoreSearch = false;

	m_archive = new WADArchive(filename);
	wxObjectDataPtr<FileDataModel> model(new FileDataModel(this, m_archive.get()));
	m_fileListCtrl->UnselectAll();
	m_fileListCtrl->AssociateModel(model.get());
	m_fileListCtrl->Refresh();
	m_menubar->Enable(ID_EXTRACT, true);
	m_toolBar->EnableTool(ID_EXTRACT, true);
	m_menubar->Enable(wxID_ADD, !m_archive->GetReadOnly());
	m_menubar->Enable(wxID_SAVE, !m_archive->GetReadOnly());
	m_toolBar->EnableTool(wxID_SAVE, !m_archive->GetReadOnly());
	m_menubar->Enable(wxID_SAVEAS, !m_archive->GetReadOnly());
	m_menubar->Enable(ID_PATCH_APPLY, !m_archive->GetReadOnly());
	m_toolBar->EnableTool(ID_PATCH_APPLY, !m_archive->GetReadOnly());
	m_menubar->Enable(ID_PATCH_PREPARE, !m_archive->GetReadOnly());
	UpdateTitle();
	CheckBackup();
	if (m_archive->GetFilteredEntryCount() > 0)
	{
		SelectItem(0);
		m_fileListCtrl->SetFocus();
	}

	m_fileHistory.AddFileToHistory(filename);
	m_fileHistory.Save(*wxConfigBase::Get());
}

void ExploreFrame::OnSaveClicked( wxCommandEvent& event )
{
	if (!ConfirmBackup())
		return;
	
	if (wxMessageBox(_("Are you sure you want to overwrite the WAD?"), _("Warning"), wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxYES)
	{
		wxBusyInfo busyInfo(_("Writing file..."));
		wxBusyCursor busyCursor;
		wxString fileName = m_archive->GetFileName();

		// Save archive
		m_archive->Save();

		// Reopen archive
		OpenFile(fileName);

		UpdateTitle();
	}
}

void ExploreFrame::OnSaveAsClicked( wxCommandEvent& event )
{
	wxFileName saveAsFN(m_archive->GetFileName());
	
	wxFileDialog fileDlg(this, _("Select destination filename"), saveAsFN.GetPath(), saveAsFN.GetFullName(), "*.wad", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		wxBusyInfo busyInfo(_("Writing file..."));
		wxBusyCursor busyCursor;
		m_archive->Save(fileDlg.GetPath());
		OpenFile(fileDlg.GetPath());
	}
}

void ExploreFrame::OnRestoreClicked( wxCommandEvent& event )
{
	wxMessageDialog msgDlg(this, _("Are you sure you wan't to restore this file from the backup copy?"), _("Warning"), wxICON_WARNING | wxYES_NO);
	msgDlg.SetYesNoLabels(_("Restore"), _("Don't Restore"));
	if (msgDlg.ShowModal() == wxID_YES)
	{
		wxBusyInfo busyInfo(_("Restoring from backup..."));
		wxBusyCursor busyCursor;
		if (!wxCopyFile(m_archive->GetFileName() + "_backup", m_archive->GetFileName(), true))
			wxLogError(_("Backup could not be restored"));
		else {
			wxString restoredFN = m_archive->GetFileName();
			OpenFile(restoredFN);
		}
	}
}

const WADArchiveEntry& ExploreFrame::GetSelectedEntry() const
{
	size_t index = (size_t) m_fileListCtrl->GetSelection().GetID() - 1;
	return m_archive->GetFilteredEntry(index);
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
				const WADArchiveEntry& entry = m_archive->GetFilteredEntry((size_t) it->GetID() - 1);
				wxFileName targetFileName(entry.GetFileName(), wxPATH_UNIX);
				targetFileName.Normalize(wxPATH_NORM_ALL, dirDlg.GetPath());
				wxLogDebug("Extracting to: %s", targetFileName.GetFullPath());

				if (!targetFileName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
					break;
				m_archive->Extract(entry, targetFileName.GetFullPath());
			}
		}
		wxLogInfo(_("Files extracted to %s"), dirDlg.GetPath());
	} else {
		const WADArchiveEntry& entry = GetSelectedEntry();

		wxFileName fn(entry.GetFileName(), wxPATH_UNIX);

		wxString fileName = fn.GetFullName();
		wxFileDialog fileDlg(this, _("Select target for file extraction"), extractFolder, fileName, 
			wxFileSelectorDefaultWildcardStr, wxFD_OVERWRITE_PROMPT | wxFD_SAVE);
		if (fileDlg.ShowModal() == wxID_OK)
		{
			wxBusyInfo busyInfo(_("Extracting data..."));
			wxBusyCursor busyCursor;

			m_archive->Extract(entry, fileDlg.GetPath());
		}

		wxLogInfo(_("File extracted to %s"), fileDlg.GetPath());
	}
}

void ExploreFrame::OnReplaceClicked( wxCommandEvent& event )
{
	const WADArchiveEntry& entry = GetSelectedEntry();
	wxFileName entryFN(entry.GetFileName());
	
	wxFileDialog fileDlg(this, wxString::Format(_("Select file to replace %s"), entry.GetFileName()), wxString(),
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
				m_archive->Extract(entry, oStr);
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

		size_t index = (size_t) m_fileListCtrl->GetSelection().GetID() - 1;
		m_archive->ReplaceFiltered(index, fileDlg.GetPath());
		static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->RowChanged(index);
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
			m_archive->Add(WADArchiveEntry(newEntryName, fileDlg.GetPath()));
			static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->RowAppended();

			UpdateTitle();
		}
	}
}

void ExploreFrame::OnDeleteClicked( wxCommandEvent& event )
{
	const WADArchiveEntry& entry = GetSelectedEntry();
	if (wxMessageBox(wxString::Format(_("Are you sure you want to delete %s?"), entry.GetFileName()),
									  _("Warning"), wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxYES)
	{
		size_t index = (size_t) m_fileListCtrl->GetSelection().GetID() - 1;
		m_archive->Remove(index);
		static_cast<FileDataModel*>(m_fileListCtrl->GetModel())->RowDeleted(index);
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
	m_menubar->Enable(ID_REPLACE, m_fileListCtrl->GetSelectedItemsCount() > 0 && !m_archive->GetReadOnly());
	m_menubar->Enable(ID_EXTRACT, m_fileListCtrl->GetSelectedItemsCount() > 0);
	m_toolBar->EnableTool(ID_EXTRACT, m_fileListCtrl->GetSelectedItemsCount() > 0);
	m_menubar->Enable(wxID_DELETE, m_fileListCtrl->GetSelectedItemsCount() > 0 && !m_archive->GetReadOnly());
	
	if (m_fileListCtrl->GetSelectedItemsCount() != 1)
		return;

	wxBusyCursor busyCursor;

	size_t index = (size_t)m_fileListCtrl->GetSelection().GetID() - 1;
	const WADArchiveEntry& entry = m_archive->GetFilteredEntry(index);
	wxLogDebug("Selected: %s", entry.GetFileName());

	wxFileName fn(entry.GetFileName(), wxPATH_UNIX);
	wxString fileExt = fn.GetExt();

	if (fileExt.IsSameAs("png", false) ||
		fileExt.IsSameAs("jpg", false))
	{
		m_previewBookCtrl->ChangeSelection(1);

		ImagePanel* imgPanel = (ImagePanel*) m_previewBookCtrl->GetCurrentPage();
		imgPanel->m_previewBitmap->SetBitmap(m_archive->ExtractBitmap(entry));
		imgPanel->Layout();
	}
	else if (fileExt.IsSameAs("meta", false))
	{
		m_previewBookCtrl->ChangeSelection(2);

		// Load texture pack
		wxMemoryOutputStream oStr;
		m_archive->Extract(entry, oStr);
		wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
		wxMemoryInputStream iStr(buffer->GetBufferStart(), buffer->GetBufferSize());

		wxFileName imageFN(entry.GetFileName(), wxPATH_UNIX);
		imageFN.SetExt("png");
		WADArchiveEntry imgEntry = m_archive->GetEntry(imageFN.GetFullPath(wxPATH_UNIX));

		TexturePackPanel* texPanel = (TexturePackPanel*)m_previewBookCtrl->GetCurrentPage();
		texPanel->LoadTexture(iStr, m_archive->ExtractBitmap(imgEntry));
	}
	else if (fileExt.IsSameAs("fnt", false) ||
			fileExt.IsSameAs("ini", false) ||
			fileExt.IsSameAs("vsh", false) ||
			 fileExt.IsSameAs("fsh", false))
	{
		m_previewBookCtrl->ChangeSelection(3);
		TextPanel* textPanel = (TextPanel*) m_previewBookCtrl->GetCurrentPage();

		wxMemoryOutputStream oStr;
		m_archive->Extract(entry, oStr);
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
		m_archive->Extract(entry, oStr);
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

void ExploreFrame::OnPatchApplyClicked(wxCommandEvent& event)
{
	if (!ConfirmBackup())
		return;
	
	wxString patchFolder = wxStandardPaths::Get().GetDocumentsDir();
	wxFileDialog fileDlg(this, _("Select patch wad to apply"), patchFolder, wxString(),
		"*.patchwad", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		WADArchive patchArchive(fileDlg.GetPath());

		wxMessageDialog msgDlg(this, wxString::Format(_("Applying patch\n\nThis patch contains %d new/modified files.\nAre you sure you want to apply this patch?"),
			patchArchive.GetEntryCount()), _("Warning"),
			wxICON_WARNING | wxOK | wxCANCEL | wxCANCEL_DEFAULT);
		msgDlg.SetOKLabel(_("Patch"));

		if (msgDlg.ShowModal() == wxID_OK)
		{
			wxBusyCursor busyCursor;
			wxBusyInfo busyInfo("Applying patch...");

			// Apply patch entries
			for (size_t patchIndex = 0; patchIndex < patchArchive.GetEntryCount(); patchIndex++)
			{
				m_archive->Patch(patchArchive, patchArchive.GetEntry(patchIndex));
			}

			wxString fileName = m_archive->GetFileName();

			// Save archive
			m_archive->Save();

			// Reopen archive
			OpenFile(fileName);

			wxLogInfo(_("Patch applied"));
		}
	}
}

void ExploreFrame::OnPatchPrepareClicked(wxCommandEvent& event)
{
	if (m_preparingPatch)
	{
		m_fileListToggleColumn->SetHidden(true);
		m_menubar->Check(ID_PATCH_PREPARE, false);
		m_preparingPatch = false;
		m_patchEntries.clear();
		m_menubar->Enable(ID_PATCH_CREATE, false);
	} else {
		m_fileListToggleColumn->SetHidden(false);
		m_menubar->Check(ID_PATCH_PREPARE, true);
		m_preparingPatch = true;
	}
}

void ExploreFrame::OnPatchCreateClicked(wxCommandEvent& event)
{
	wxString patchFolder = wxStandardPaths::Get().GetDocumentsDir();
	wxFileDialog fileDlg(this, _("Select output for patch file"), patchFolder, wxString(),
		"*.patchwad", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		{
			wxBusyCursor busyCursor;
			wxBusyInfo busyInfo("Writing patch file...");

			WADArchive patchArchive(fileDlg.GetPath(), true);

			for (auto it = m_patchEntries.begin(); it != m_patchEntries.end(); ++it)
			{
				patchArchive.Add(WADArchiveEntry(m_archive->GetEntry(*it), m_archive.get()));
			}
			patchArchive.Save();
		}

		OnPatchPrepareClicked(event);

		wxMessageDialog msgDlg(this, wxString::Format(_("Patch saved to %s"), fileDlg.GetPath()), _("Information"), wxICON_INFORMATION | wxYES_NO);
		msgDlg.SetYesNoLabels(_("Open Folder"), wxID_CLOSE);

		if (msgDlg.ShowModal() == wxID_YES)
		{
			wxFileName folderFN(fileDlg.GetPath());
			folderFN.SetFullName(wxString());

			wxLaunchDefaultApplication(folderFN.GetPath());
		}
	}
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
	if (m_archive.get())
	{
		wxFileName archiveFN(m_archive->GetFileName());
		SetTitle(wxString::Format("%s%s - %s", modStr, archiveFN.GetName(), wxTheApp->GetAppDisplayName()));
	} else {
		SetTitle(wxTheApp->GetAppDisplayName());
	}
}

bool ExploreFrame::ConfirmBackup()
{
	if (m_backupAvailable)
		return true;
	
	wxMessageDialog msgDlg(this, _("Backup recommended\n\nBefore modifing the file it is recommend to create a backup.\nDo you wish to create a backup now?"),
					_("Warning"), wxICON_WARNING | wxYES_NO | wxCANCEL);
	msgDlg.SetYesNoLabels(_("Backup"), _("Don't backup"));

	switch (msgDlg.ShowModal()) {
		case wxID_YES:
		{
			wxBusyInfo busyInfo(_("Creating backup..."));
			wxBusyCursor busyCursor;
			if (!wxCopyFile(m_archive->GetFileName(), m_archive->GetFileName() + "_backup"))
			{
				wxLogError(_("Backup could not be created"));
				return false;
			} else {
				CheckBackup();
				return true;
			}
		}
		
		case wxID_NO:
			return true;
			
		default:
			return false;
	}
}

void ExploreFrame::CheckBackup()
{
	m_backupAvailable = wxFileExists(m_archive->GetFileName() + "_backup");
	m_menubar->Enable(ID_RESTORE, m_backupAvailable);
}
