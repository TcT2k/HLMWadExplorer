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

class FileDataModel : public wxDataViewVirtualListModel
{
public:
	FileDataModel(WADArchive* archive):
		wxDataViewVirtualListModel(archive->GetEntryCount()),
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

	void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
	{
		const WADArchiveEntry& entry = m_archive->GetEntry(row);
		switch (col)
		{
			case 0:
				variant = entry.GetFileName();
				break;
			case 1:
				variant = wxFileName::GetHumanReadableSize(entry.GetSize());
				break;
		}

	}

	bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
	{
		return false;
	}

private:
	WADArchive* m_archive;
};


ExploreFrame::ExploreFrame( wxWindow* parent ):
	BaseExploreFrame( parent ),
	m_archive(NULL)
{
	SetTitle(wxTheApp->GetAppDisplayName());
	m_menubar->Enable(ID_EXTRACT, false);
	m_menubar->Enable(wxID_SAVE, false);
	m_menubar->Enable(wxID_SAVEAS, false);
	m_fileListNameColumn->SetWidth(150);
	m_fileListSizeColumn->SetAlignment(wxALIGN_RIGHT);
}

void ExploreFrame::OnAboutClicked( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutInfo;
	aboutInfo.SetName(wxTheApp->GetAppDisplayName());
	aboutInfo.SetDescription(_("HLM Wad Extractor"));
	aboutInfo.SetCopyright("(C) 2015");
	aboutInfo.SetWebSite("https://github.com/TcT2k/HLMWadExplorer");
	aboutInfo.AddDeveloper("Tobias Taschner");
	wxAboutBox(aboutInfo);
}

void ExploreFrame::OnOpenClicked( wxCommandEvent& event )
{
	wxString hlmPath;
#if defined(__WXOSX__)
	hlmPath = wxFileName::GetHomeDir() + "/Library/Application Support/Steam/steamapps/common/Hotline Miami 2/HotlineMiami2.app/Contents/Resources/";
#elif defined(__WXMSW__
	
#endif
	
	wxFileDialog fileDlg(this, _("Select WAD file"), hlmPath, wxString(), "*.wad", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);

	if (fileDlg.ShowModal() == wxID_OK)
	{
		OpenFile(fileDlg.GetPath());
	}
}

void ExploreFrame::OpenFile(const wxString& filename)
{
	m_archive = new WADArchive(filename);
	wxObjectDataPtr<FileDataModel> model(new FileDataModel(m_archive.get()));
	m_fileListCtrl->AssociateModel(model.get());
	m_menubar->Enable(ID_EXTRACT, true);
	m_menubar->Enable(wxID_SAVE, true);
	m_menubar->Enable(wxID_SAVEAS, true);
	
	wxFileName inputFN(filename);
	SetTitle(wxString::Format("%s - %s", wxTheApp->GetAppDisplayName(), inputFN.GetName()));
}

void ExploreFrame::OnSaveClicked( wxCommandEvent& event )
{
	if (wxMessageBox(_("Are you sure you want to overwrite the WAD?"), _("Warning"), wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxOK)
	{
		wxBusyInfo busyInfo(_("Writing file..."));
		wxBusyCursor busyCursor;
		m_archive->Save();
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
				const WADArchiveEntry& entry = m_archive->GetEntry((size_t) it->GetID() - 1);
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
		size_t index = (size_t) m_fileListCtrl->GetSelection().GetID() - 1;
		const WADArchiveEntry& entry = m_archive->GetEntry(index);

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

void ExploreFrame::OnQuitClicked( wxCommandEvent& event )
{
	Close();
}

void ExploreFrame::OnFileListSelectionChanged( wxDataViewEvent& event )
{
	if (m_fileListCtrl->GetSelectedItemsCount() != 1)
		return;
		
	size_t index = (size_t)m_fileListCtrl->GetSelection().GetID() - 1;
	const WADArchiveEntry& entry = m_archive->GetEntry(index);
	wxLogDebug("Selected: %s", entry.GetFileName());

	wxFileName fn(entry.GetFileName(), wxPATH_UNIX);

	if (fn.GetExt().IsSameAs("png", false) ||
		fn.GetExt().IsSameAs("jpg", false))
	{
		// Load preview picture
		wxMemoryOutputStream oStr;
		m_archive->Extract(entry, oStr);

		wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
		wxMemoryInputStream iStr(buffer->GetBufferStart(), buffer->GetBufferSize());
		wxImage img(iStr);
		m_previewBitmap->SetBitmap(wxBitmap(img));
		m_previewPanel->Layout();
	} else
		m_previewBitmap->SetBitmap(wxNullBitmap);
}

void ExploreFrame::OnFileListDoubleClick( wxMouseEvent& event )
{
// TODO: Implement OnFileListDoubleClick
}
