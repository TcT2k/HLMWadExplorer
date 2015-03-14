/*
** HLMWadExplorer
**
** Coypright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include <wx/wx.h>

#include <wx/vector.h>
#include <wx/filename.h>

class WADArchiveEntry
{
public:
	WADArchiveEntry(const wxString& fileName, wxInt64 size, wxInt64 offset):
		m_fileName(fileName),
		m_size(size),
		m_offset(offset)
	{

	}
	
	WADArchiveEntry(const wxString& fileName, const wxString& sourceFileName):
		m_fileName(fileName)
	{
		SetSourceFileName(sourceFileName);
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
	
	const wxString& GetSourceFileName() const
	{
		return m_sourceFileName;
	}
	
	void SetSourceFileName(const wxString& value)
	{
		m_sourceFileName = value;
		m_offset = 0;
		m_size = wxFileName::GetSize(value).ToULong();
	}

private:
	wxString m_fileName;
	wxInt64 m_size;
	wxInt64 m_offset;
	wxString m_sourceFileName;
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

	const WADArchiveEntry& GetEntry(const wxString& fileName) const;

	const wxString& GetFileName() const
	{
		return m_fileName;
	}

	void Remove(size_t itemIndex);
	
	void Add(const WADArchiveEntry& entry);
	
	void Replace(size_t itemIndex, const wxString& sourceFileName);

	bool Extract(const WADArchiveEntry& entry, const wxString& targetFileName);

	bool Extract(const WADArchiveEntry& entry, wxOutputStream& oStr);

	wxBitmap ExtractBitmap(const WADArchiveEntry& entry);
	
	bool Save();
	
	bool Save(wxOutputStream& oStr);
	
	bool Save(const wxString& targetFileName);
	
	bool IsModified() const
	{
		return m_modified;
	}

private:
	wxString m_fileName;
	wxVector<WADArchiveEntry> m_entries;
	wxFileOffset m_dataOffset;
	bool m_modified;

	void ParseFile();
};
