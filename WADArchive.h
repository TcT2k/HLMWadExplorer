/*
** HLMWadExplorer
**
** Copyright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include <wx/wx.h>

#include <wx/vector.h>
#include <wx/filename.h>

class WADArchive;

class WADArchiveEntry
{
public:
	WADArchiveEntry(const wxString& fileName, wxInt64 size, wxInt64 offset):
		m_fileName(fileName),
		m_size(size),
		m_offset(offset),
		m_sourceArchive(NULL)
	{

	}

	WADArchiveEntry(const WADArchiveEntry& entry, const WADArchive* sourceArchive):
		m_fileName(entry.GetFileName()),
		m_size(entry.GetSize()),
		m_offset(entry.GetOffset()),
		m_sourceFileName(entry.GetSourceFileName()),
		m_sourceArchive(sourceArchive)
	{

	}
	
	WADArchiveEntry(const wxString& fileName, const wxString& sourceFileName):
		m_fileName(fileName),
		m_sourceArchive(NULL)
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
	
	const WADArchive* GetSourceArchive() const
	{
		return m_sourceArchive;
	}

private:
	wxString m_fileName;
	wxInt64 m_size;
	wxInt64 m_offset;
	wxString m_sourceFileName;
	const WADArchive* m_sourceArchive;
};

class WADArchive
{
public:
	enum WADFormat
	{
		FmtHM2,
		FmtHM1,
		FmtHM2v2
	};

	WADArchive(const wxString& fileName, bool createFile = false);

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

	void Patch(const WADArchive& patchArchive, const WADArchiveEntry& patchEntry);

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

	bool GetReadOnly() const
	{
		return m_readOnly;
	}

	/// Returns false if no entries matched the filter
	bool ApplyFilter(const wxString& filter);

	const WADArchiveEntry& GetFilteredEntry(size_t index) const
	{
		return *m_filteredEntries[index];
	}

	size_t GetFilteredEntryCount() const
	{
		return m_filteredEntries.size();
	}

	void ReplaceFiltered(size_t itemIndex, const wxString& sourceFileName);

private:
	bool m_readOnly;
	WADFormat m_format;
	wxString m_fileName;
	wxVector<WADArchiveEntry> m_entries;
	wxVector<WADArchiveEntry*> m_filteredEntries;
	wxFileOffset m_dataOffset;
	bool m_modified;

	void ParseFile();
};
