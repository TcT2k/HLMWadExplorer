#include "MergeDialog.h"

#include "wx/hyperlink.h"
#include "wx/log.h"
#include "wx/msgdlg.h"
#include "wx/busyinfo.h"

MergeDialog::MergeDialog( wxWindow* parent ):
	BaseMergeDialog( parent )
{
	m_backupAvailable = false;
	
	GetSizer()->SetMinSize(wxDLG_UNIT(this, wxSize(260, 60)));
	m_sdbSizerOK->SetLabel(_("Merge"));
	
	wxHyperlinkCtrl* linkCtrl = new wxHyperlinkCtrl(this, wxID_ANY, _("Restore Base WAD"), "");
	m_sdbSizer->Insert(0, linkCtrl, wxSizerFlags().Center().Border());
	m_sdbSizer->InsertSpacer(1, -1);
	m_sdbSizer->Layout();
	linkCtrl->Bind(wxEVT_COMMAND_HYPERLINK, &MergeDialog::OnRestoreClicked, this);
	
	Fit();
	Center();
}

void MergeDialog::OnRestoreClicked(wxCommandEvent& event)
{
	if (!wxFileExists(GetBackupFileName()))
	{
		wxLogError(_("Backup file not found"));
		return;
	}
	
	bool success = false;
	
	wxMessageDialog msgDlg(this, _("Are you sure you wan't to restore this file from the backup copy?"), _("Warning"), wxICON_WARNING | wxYES_NO);
	msgDlg.SetYesNoLabels(_("Restore"), _("Don't Restore"));
	if (msgDlg.ShowModal() == wxID_YES)
	{
		wxBusyInfo busyInfo(_("Restoring from backup..."));
		wxBusyCursor busyCursor;
		if (!wxCopyFile(GetBackupFileName(), m_baseFilePicker->GetFileName().GetFullPath(), true))
			wxLogError(_("Backup could not be restored"));
		else
			success = true;
	}
	
	if (success)
		wxMessageBox(_("Backup successfully restored"), _("Information"), wxICON_INFORMATION|wxOK, this);
}

bool MergeDialog::TransferDataFromWindow()
{
	if (!BaseMergeDialog::TransferDataFromWindow())
		return false;
	
	if (!m_baseFilePicker->GetFileName().Exists())
	{
		wxLogError(_("Base file not found"));
		return false;
	}
	
	if (!m_patchFilePicker->GetFileName().Exists())
	{
		wxLogError(_("Patch file not found"));
		return false;
	}
	
	CheckBackup();
	
	if (!ConfirmBackup())
		return false;
	
	return true;
}

bool MergeDialog::ConfirmBackup()
{
	if (m_backupAvailable)
		return true;
	
	wxMessageDialog msgDlg(this, _("Backup recommended\n\nBefore modifying the file it is recommend to create a backup.\nDo you wish to create a backup now?"),
						   _("Warning"), wxICON_WARNING | wxYES_NO | wxCANCEL);
	msgDlg.SetYesNoLabels(_("Backup"), _("Don't backup"));
	
	switch (msgDlg.ShowModal()) {
		case wxID_YES:
		{
			wxBusyInfo busyInfo(_("Creating backup..."));
			wxBusyCursor busyCursor;
			if (!wxCopyFile(m_baseFilePicker->GetFileName().GetFullPath(), GetBackupFileName()))
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

void MergeDialog::CheckBackup()
{
	m_backupAvailable = wxFileExists(GetBackupFileName());
}

wxString MergeDialog::GetBackupFileName() const
{
	wxString backupFileName = m_baseFilePicker->GetFileName().GetFullPath() + "_bkup";
	return backupFileName;
}
