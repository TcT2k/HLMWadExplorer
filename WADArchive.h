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
	enum EntryStatus
	{
		Entry_Original,
		Entry_Replaced,
		Entry_Added
	};

	WADArchiveEntry(const wxString& fileName, wxInt64 size, wxInt64 offset):
		m_status(Entry_Original),
		m_fileName(fileName),
		m_size(size),
		m_offset(offset),
		m_sourceArchive(NULL)
	{

	}

	WADArchiveEntry(const WADArchiveEntry& entry, const WADArchive* sourceArchive):
		m_status(Entry_Original),
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

	EntryStatus GetStatus() const
	{
		return m_status;
	}

	void SetStatus(EntryStatus status)
	{
		m_status = status;
	}

private:
	EntryStatus m_status;
	wxString m_fileName;
	wxInt64 m_size;
	wxInt64 m_offset;
	wxString m_sourceFileName;
	const WADArchive* m_sourceArchive;
};

class WADDirEntry
{
public:
	WADDirEntry(WADDirEntry* parent)
	{
		m_parent = parent;
	}

	virtual ~WADDirEntry()
	{ }

	virtual size_t GetChildCount() const = 0;

	virtual WADDirEntry* GetChild(size_t index) = 0;

	virtual WADArchiveEntry* GetEntry() const = 0;

	const wxString& GetName() const
	{
		return m_name;
	}

	WADDirEntry* GetParent() const
	{
		return m_parent;
	}

	bool empty() const
	{
		return GetChildCount() == 0;
	}

protected:
	wxString m_name;

private:
	WADDirEntry* m_parent;
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

	bool Extract(const WADArchiveEntry& entry, const wxString& targetFileName);

	bool Extract(const WADArchiveEntry& entry, wxOutputStream& oStr);

	wxBitmap ExtractBitmap(const WADArchiveEntry& entry);
	
	bool Write();
	
	bool Write(wxOutputStream& oStr);
	
	bool Write(const wxString& targetFileName);

	bool CreatePatch(const wxString& targetFileName);

	bool LoadPatch(const wxString& patchFileName);

	bool IsModified() const
	{
		return m_modified;
	}

	bool GetReadOnly() const
	{
		return m_readOnly;
	}

	WADFormat GetFormat() const
	{
		return m_format;
	}

	/// Returns false if no entries matched the filter
	bool ApplyFilter(const wxString& filter);

	WADDirEntry* GetRootDir() const
	{
		return m_rootDir.get();
	}

	void Replace(WADDirEntry* dir, const wxString& sourceFileName);

	WADDirEntry* FindDir(const wxString& path, bool createOnDemand = false);

private:
	bool m_readOnly;
	WADFormat m_format;
	wxString m_fileName;
	wxVector<WADArchiveEntry> m_entries;
	wxSharedPtr<WADDirEntry> m_rootDir;
	wxFileOffset m_dataOffset;
	bool m_modified;
	wxSharedPtr<WADArchive> m_patchArchive;

	void ParseFile();
};
