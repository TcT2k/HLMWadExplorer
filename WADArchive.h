/*
** HLMWadExplorer
**
** Coypright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include <wx/wx.h>

#include <wx/vector.h>

class WADArchiveEntry
{
public:
	WADArchiveEntry(const wxString& fileName, wxInt64 size, wxInt64 offset):
		m_fileName(fileName),
		m_size(size),
		m_offset(offset)
	{

	}

	const wxString& GetFileName() const
	{
		return m_fileName;
	}

	wxInt64 GetSize() const
	{
		return m_size;
	}

	wxInt64 GetOffset() const
	{
		return m_offset;
	}

private:
	wxString m_fileName;
	wxInt64 m_size;
	wxInt64 m_offset;
};

class WADArchive
{
public:
	WADArchive(const wxString& fileName);

	size_t GetEntryCount() const
	{
		return m_entries.size();
	}

	const WADArchiveEntry& GetEntry(size_t index) const
	{
		return m_entries[index];
	}
	
	const wxString& GetFileName() const
	{
		return m_fileName;
	}

	bool Extract(const WADArchiveEntry& entry, const wxString& targetFileName);

	bool Extract(const WADArchiveEntry& entry, wxOutputStream& oStr);
	
	bool Save();
	
	bool Save(wxOutputStream& oStr);
	
	bool Save(const wxString& targetFileName);

private:
	wxString m_fileName;
	wxVector<WADArchiveEntry> m_entries;
	wxFileOffset m_dataOffset;

	void ParseFile();
};
