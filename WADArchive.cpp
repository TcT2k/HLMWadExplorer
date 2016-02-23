/*
** HLMWadExplorer
**
** Copyright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include "WADArchive.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>

//
// FileWADDirEntry
//

class FileWADDirEntry : public WADDirEntry
{
public:
	FileWADDirEntry(WADDirEntry* parent, WADArchiveEntry* entry):
		WADDirEntry(parent)
	{
		m_entry = entry;
		wxFileName fn(entry->GetFileName());
		m_name = fn.GetFullName();
	}

	virtual size_t GetChildCount() const override
	{
		return 0;
	}

	virtual WADDirEntry* GetChild(size_t index) const override
	{
		return NULL;
	}

	virtual WADArchiveEntry* GetEntry() const override
	{
		return m_entry;
	}

private:
	WADArchiveEntry* m_entry;
};

//
// FolderWADDirEntry
//
class FolderWADDirEntry : public WADDirEntry
{
public:
	FolderWADDirEntry(WADDirEntry* parent, const wxString& name):
		WADDirEntry(parent)
	{
		m_name = name;
	}

	virtual size_t GetChildCount() const override
	{
		return m_entries.size();
	}

	virtual WADDirEntry* GetChild(size_t index) const override
	{
		return m_entries[index].get();
	}

	virtual WADArchiveEntry* GetEntry() const override
	{
		return NULL;
	}

	WADDirEntry* AddFolder(const wxString& name)
	{
		wxSharedPtr<WADDirEntry> folder(new FolderWADDirEntry(this, name));
		m_entries.push_back(folder);
		return folder.get();
	}

	WADDirEntry* AddFile(WADArchiveEntry* entry)
	{
		wxSharedPtr<WADDirEntry> folder(new FileWADDirEntry(this, entry));
		m_entries.push_back(folder);
		return folder.get();
	}

private:
	wxVector< wxSharedPtr<WADDirEntry> > m_entries;
};

//
// WADDirEntry
//
wxString WADDirEntry::GetPath() const
{
	wxString path = m_name;

	WADDirEntry* parent = GetParent();
	while (parent && !parent->GetName().empty())
	{
		path.insert(0, parent->GetName() + "/");
		parent = parent->GetParent();
	}

	return path;
}

size_t WADDirEntry::GetDirCount() const
{
	size_t dirCount = 0;

	for (size_t i = 0; i < GetChildCount(); ++i)
	{
		WADDirEntry* subDir = GetChild(i);
		if (!subDir->GetEntry())
			dirCount++;

		dirCount += subDir->GetDirCount();
	}

	return dirCount;
}

//
// WADArchive
//

WADArchive::WADArchive(const wxString& fileName, bool createFile) :
	m_readOnly(false),
	m_format(FmtHM2),
	m_fileName(fileName),
	m_modified(false)
{
	if (!createFile)
		ParseFile();
}

void WADArchive::ParseFile()
{
	wxFileName wadFN(m_fileName);
	if (wadFN.GetName().IsSameAs("HotlineMiami_GL", false))
		m_format = FmtHM1;

	wxFileInputStream iStr(m_fileName);

	char fileId[5];
	iStr.Read(fileId, 4);
	fileId[4] = 0;
	if (strcmp(fileId, "AGAR") == 0)
	{
		m_format = FmtHM2v2;

		wxUint32 majorVer;
		wxUint32 minorVer;
		iStr.Read(&majorVer, sizeof(majorVer));
		iStr.Read(&minorVer, sizeof(minorVer));
		wxUint32 extHeaderSize;
		iStr.Read(&extHeaderSize, sizeof(extHeaderSize));
		iStr.SeekI(extHeaderSize, wxFromCurrent);
		wxLogDebug("Opened file format: %d.%d (%d bytes extra header data)", majorVer, minorVer, extHeaderSize);
	} else
		iStr.SeekI(0);

	if (m_format == FmtHM1)
	{
		m_readOnly = true;

		wxUint32 dataOffset;
		iStr.Read(&dataOffset, sizeof(dataOffset));

		m_dataOffset = dataOffset;
	}

	wxUint32 fileCount;
	iStr.Read(&fileCount, sizeof(fileCount));
	wxLogDebug("Found %d files in wad: %s", fileCount, m_fileName);

	m_entries.reserve(fileCount);

	for (size_t fileIndex = 0; fileIndex < fileCount; fileIndex++)
	{
		wxUint32 fileNameLength;
		iStr.Read(&fileNameLength, sizeof(fileNameLength));

		wxCharBuffer fnBuf(fileNameLength);
		iStr.Read(fnBuf.data(), fileNameLength);
		wxString fileName(fnBuf);

		if (m_format != FmtHM1)
		{
			wxUint64 fileSize;
			iStr.Read(&fileSize, sizeof(fileSize));

			wxUint64 fileOffset;
			iStr.Read(&fileOffset, sizeof(fileOffset));

			m_entries.push_back(WADArchiveEntry(fileName, fileSize, fileOffset));
		}
		else
		{
			wxUint32 fileSize;
			iStr.Read(&fileSize, sizeof(fileSize));

			wxUint32 fileOffset;
			iStr.Read(&fileOffset, sizeof(fileOffset));

			m_entries.push_back(WADArchiveEntry(fileName, fileSize, fileOffset));
		}
	}

	if (m_format == FmtHM2v2)
	{
		// Parse directory table (currently unused but required to determine data offset)
		wxUint32 dirCount;
		iStr.Read(&dirCount, sizeof(dirCount));
		for (int dirIndex = 0; dirIndex < dirCount; dirIndex++)
		{
			wxUint32 dirNameLength;
			iStr.Read(&dirNameLength, sizeof(dirNameLength));

			if (dirNameLength)
			{
				wxCharBuffer dirNameBuf(dirNameLength);
				iStr.Read(dirNameBuf.data(), dirNameLength);
				wxString dirName(dirNameBuf);
			}

			// Parse directory
			wxUint32 entryCount;
			iStr.Read(&entryCount, sizeof(entryCount));

			for (int entryIndex = 0; entryIndex < entryCount; entryIndex++)
			{
				wxUint32 entryNameLength;
				iStr.Read(&entryNameLength, sizeof(entryNameLength));

				wxCharBuffer entryNameBuf(entryNameLength);
				iStr.Read(entryNameBuf.data(), entryNameLength);
				wxString entryName(entryNameBuf);

				wxUint8 entryType;
				iStr.Read(&entryType, sizeof(entryType));

			}
		}
	}

	if (m_format != FmtHM1)
		m_dataOffset = iStr.SeekI(0, wxFromCurrent);

	ApplyFilter("");
}

bool WADArchive::Extract(const WADArchiveEntry& entry, const wxString& targetFileName)
{
	wxTempFileOutputStream oStr(targetFileName);
	if (!oStr.IsOk())
		return false;

	if (Extract(entry, oStr))
		return oStr.Commit();
	else
		return false;
}

bool WADArchive::Extract(const WADArchiveEntry& entry, wxOutputStream& oStr)
{
	wxString inputFileName = entry.GetSourceFileName();
	if (inputFileName.empty())
	{
		if (entry.GetSourceArchive())
			inputFileName = entry.GetSourceArchive()->GetFileName();
		else
			inputFileName = m_fileName;
	}
	
	wxFileInputStream iStr(inputFileName);
	if (entry.GetSourceFileName().empty())
	{
		wxFileOffset archiveOffset = (entry.GetSourceArchive()) ? entry.GetSourceArchive()->m_dataOffset : m_dataOffset;
		iStr.SeekI(archiveOffset + entry.GetOffset());
	}

	static int MAX_BUFFER_SIZE = 32 * 1024;
	char* buffer = new char[MAX_BUFFER_SIZE];

	wxInt64 copiedSize = 0;
	while (copiedSize < entry.GetSize())
	{
		size_t readSize = MAX_BUFFER_SIZE;
		if (copiedSize + readSize > entry.GetSize())
			readSize = entry.GetSize() - copiedSize;

		iStr.Read(buffer, readSize);
		oStr.Write(buffer, readSize);

		copiedSize += readSize;
	}

	delete[] buffer;

	return true;
}

wxBitmap WADArchive::ExtractBitmap(const WADArchiveEntry& entry)
{
	// Suppress image loading warnings
	wxLogNull nullLog;

	// Load preview picture
	wxMemoryOutputStream oStr;
	Extract(entry, oStr);

	wxStreamBuffer* buffer = oStr.GetOutputStreamBuffer();
	wxMemoryInputStream iStr(buffer->GetBufferStart(), buffer->GetBufferSize());
	wxImage img(iStr);

	if (!img.IsOk())
		return wxNullBitmap;
	else
		return wxBitmap(img);
}

bool WADArchive::Write()
{
	return Write(m_fileName);
}

static void WriteFileName(const wxString& str, wxOutputStream& oStr)
{
	wxScopedCharBuffer utf8FN = str.utf8_str();
	wxUint32 fileNameLength = utf8FN.length();
	oStr.Write(&fileNameLength, sizeof(fileNameLength));
	oStr.Write(utf8FN.data(), fileNameLength);
}

static void WriteDir(WADDirEntry* dir, wxOutputStream& oStr)
{
	WriteFileName(dir->GetPath(), oStr);

	wxUint32 entryCount = dir->GetChildCount();
	oStr.Write(&entryCount, sizeof(entryCount));

	// Write entries
	for (size_t i = 0; i < entryCount; ++i)
	{
		WADDirEntry* entry = dir->GetChild(i);
		WriteFileName(entry->GetName(), oStr);
		wxUint8 type = (entry->GetEntry()) ? 0 : 1;
		oStr.Write(&type, sizeof(type));
	}

	// Write child directories
	for (size_t i = 0; i < entryCount; ++i)
	{
		WADDirEntry* entry = dir->GetChild(i);
		if (!entry->GetEntry())
			WriteDir(entry, oStr);
	}
}

bool WADArchive::Write(wxOutputStream& oStr)
{
	ApplyFilter("");

	if (m_format == FmtHM2v2)
	{
		// Write header
		const char* id = "AGAR";
		oStr.Write(id, 4);
		wxUint32 majorVer = 1;
		wxUint32 minorVer = 1;
		oStr.Write(&majorVer, sizeof(majorVer));
		oStr.Write(&minorVer, sizeof(minorVer));
		wxUint32 extHeaderSize = 0;
		oStr.Write(&extHeaderSize, sizeof(extHeaderSize));
	}

	// Write directory
	wxUint32 fileCount = m_entries.size();
	oStr.Write(&fileCount, sizeof(fileCount));
	
	wxInt64 currentDataOffset = 0;
	
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		WriteFileName(entry->GetFileName(), oStr);
		
		wxUint64 fileSize = entry->GetSize();
		oStr.Write(&fileSize, sizeof(fileSize));
		
		oStr.Write(&currentDataOffset, sizeof(currentDataOffset));
		
		currentDataOffset += entry->GetSize();
	}

	if (m_format == FmtHM2v2)
	{
		// Write directory map
		wxUint32 dirCount = m_rootDir->GetDirCount() + 1;
		oStr.Write(&dirCount, sizeof(dirCount));

		WriteDir(m_rootDir.get(), oStr);
	}
	
	// Write file data
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
		Extract(*entry, oStr);
	
	m_modified = false;
	
	return true;
}

bool WADArchive::Write(const wxString& targetFileName)
{
	wxTempFileOutputStream oStr(targetFileName);
	if (!oStr.IsOk())
		return false;
	
	if (Write(oStr))
		return oStr.Commit();
	else
		return false;
}

bool WADArchive::CreatePatch(const wxString& targetFileName)
{
	WADArchive patchArchive(targetFileName, true);
	patchArchive.m_format = FmtHM2v2;

	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (entry->GetStatus() != WADArchiveEntry::Entry_Original)
			patchArchive.Add(WADArchiveEntry(*entry, this));
	}

	if (patchArchive.Write())
	{
		m_modified = false;
		return true;
	}
	else
		return false;
}

bool WADArchive::LoadPatch(const wxString& patchFileName)
{
	if (m_patchArchive.get())
	{
		// TODO: reset entries
		wxLogError("Only one patch wad can be loaded");
		return false;
	}

	m_patchArchive.reset(new WADArchive(patchFileName));

	for (int patchIndex = 0; patchIndex < m_patchArchive->GetEntryCount(); ++patchIndex)
	{
		const WADArchiveEntry& patchEntry = m_patchArchive->GetEntry(patchIndex);

		for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
		{
			if (entry->GetFileName().IsSameAs(patchEntry.GetFileName()))
			{
				wxLogDebug("Patch entry: %s", patchEntry.GetFileName());
				*entry = WADArchiveEntry(patchEntry, m_patchArchive.get());
				entry->SetStatus(WADArchiveEntry::Entry_Replaced);

				break;
			}
		}
	}

	return true;
}

const WADArchiveEntry& WADArchive::GetEntry(const wxString& fileName) const
{
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (entry->GetFileName().IsSameAs(fileName))
			return *entry;
	}

	static WADArchiveEntry nullEntry("", 0, 0);
	return nullEntry;
}

void WADArchive::Remove(size_t itemIndex)
{
	m_entries.erase(m_entries.begin() + itemIndex);
	m_modified = true;
}

void WADArchive::Add(const WADArchiveEntry& entry)
{
	m_entries.push_back(entry);
	m_modified = true;
}

void WADArchive::Replace(size_t itemIndex, const wxString& sourceFileName)
{
	m_entries[itemIndex].SetSourceFileName(sourceFileName);
	m_entries[itemIndex].SetStatus(WADArchiveEntry::Entry_Replaced);
	m_modified = true;
}

void WADArchive::Replace(WADDirEntry* dir, const wxString& sourceFileName)
{
	dir->GetEntry()->SetSourceFileName(sourceFileName);
	dir->GetEntry()->SetStatus(WADArchiveEntry::Entry_Replaced);
	m_modified = true;
}

bool WADArchive::ApplyFilter(const wxString& filter)
{
	m_rootDir.reset(new FolderWADDirEntry(NULL, ""));

	wxString lowerFilter = filter.Lower();

	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (lowerFilter.empty() ||
			entry->GetFileName().Lower().Find(lowerFilter) != wxNOT_FOUND)
		{
			WADDirEntry* dir = FindDir(entry->GetFileName(), true);
			static_cast<FolderWADDirEntry*>(dir)->AddFile(entry);
		}
	}

	if (!m_rootDir->GetChildCount())
	{
		// Reset filter
		ApplyFilter("");

		return false;
	} else
		return true;
}

WADDirEntry* WADArchive::FindDir(const wxString& path, bool createOnDemand)
{
	WADDirEntry* result = m_rootDir.get();
	if (!result)
		return NULL;

	wxFileName fn(path);
	for (auto it = fn.GetDirs().begin(); it != fn.GetDirs().end(); ++it)
	{
		WADDirEntry* subDir = NULL;
		for (size_t i = 0; i < result->GetChildCount() && !subDir; ++i)
		{
			WADDirEntry* entry = result->GetChild(i);
			if (!entry->GetEntry() && entry->GetName() == *it)
			{
				subDir = entry;
			}
		}

		if (!subDir)
		{
			if (createOnDemand)
				result = static_cast<FolderWADDirEntry*>(result)->AddFolder(*it);
			else
				return NULL;
		}
		else
			result = subDir;
	}

	return result;
}

