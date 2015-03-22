#include "StringTablePanel.h"

StringTablePanel::StringTablePanel( wxWindow* parent )
:
BaseStringTablePanel( parent )
{

}

void StringTablePanel::LoadStringTable(const void* data, size_t dataSize)
{
	wxGridTableBase* table = m_grid->GetTable();

	m_grid->Freeze();

	table->DeleteRows(0, table->GetRowsCount());
	table->DeleteCols(0, table->GetColsCount());

	const char* dataPos = (const char*) data;

	wxUint32 langCount = *reinterpret_cast<const wxUint32*>(dataPos);
	dataPos += sizeof(langCount);

	table->AppendCols(langCount - 1);

	for (wxUint32 langIndex = 0; langIndex < langCount; langIndex++)
	{
		// Parse string table

		// Skip ID
		dataPos += sizeof(wxUint32);

		wxUint32 idLen = *reinterpret_cast<const wxUint32*>(dataPos);
		dataPos += sizeof(idLen);
		wxString tableId(dataPos, idLen);
		dataPos += idLen + 4;

		if (langIndex > 0)
			table->SetColLabelValue(langIndex - 1, tableId);

		wxUint32 entryCount = *reinterpret_cast<const wxUint32*>(dataPos);
		dataPos += sizeof(entryCount);

		if (langIndex == 0)
			table->AppendRows(entryCount);

		const char* stringData = dataPos;
		stringData += (entryCount + 1) * sizeof(wxUint32);

		const wxUint32* tablePos = reinterpret_cast<const wxUint32*>(dataPos);

		for (wxUint32 entryIndex = 0; entryIndex < entryCount; entryIndex++)
		{
			size_t strLen = tablePos[1] - *tablePos - 1;
			wxString value = wxString::FromUTF8(&stringData[*tablePos], strLen);

			if (langIndex == 0)
				table->SetRowLabelValue(entryIndex, value);
			else
				table->SetValue(entryIndex, langIndex - 1, value);
			tablePos++;
		}

		// Skip to next table
		dataPos += entryCount * sizeof(wxUint32);
		wxUint32 skipSize = *reinterpret_cast<const wxUint32*>(dataPos);
		dataPos += skipSize + 4;
	}

	m_grid->Thaw();
}
