#ifndef __MergeDialog__
#define __MergeDialog__

#include "HLMWADFrames.h"

class MergeDialog : public BaseMergeDialog
{
public:
	MergeDialog( wxWindow* parent );
	
	void SetBaseWAD(const wxString& fileName)
	{
		m_baseFilePicker->SetFileName(fileName);
	}
	
	wxString GetBaseWAD()
	{
		return m_baseFilePicker->GetFileName().GetFullPath();
	}
	
	void SetPatchWAD(const wxString& fileName)
	{
		m_patchFilePicker->SetFileName(fileName);
	}
	
	wxString GetPatchWAD()
	{
		return m_patchFilePicker->GetFileName().GetFullPath();
	}
	
	void OnRestoreClicked(wxCommandEvent& event);
	
	bool TransferDataFromWindow();
	
	bool ConfirmBackup();
	
	void CheckBackup();
	
	wxString GetBackupFileName() const;
	
private:
	bool m_backupAvailable;
};

#endif // __MergeDialog__
