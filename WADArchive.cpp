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

		if (m_format == FmtHM2)
		{
			wxUint64 fileSize;
			iStr.Read(&fileSize, sizeof(fileSize));

			wxUint64 fileOffset;
			iStr.Read(&fileOffset, sizeof(fileOffset));

			m_entries.push_back(WADArchiveEntry(fileName, fileSize, fileOffset));
		}
		else if (m_format == FmtHM1) {
			wxUint32 fileSize;
			iStr.Read(&fileSize, sizeof(fileSize));

			wxUint32 fileOffset;
			iStr.Read(&fileOffset, sizeof(fileOffset));

			m_entries.push_back(WADArchiveEntry(fileName, fileSize, fileOffset));
		}
	}

	if (m_format == FmtHM2)
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

bool WADArchive::Save()
{
	return Save(m_fileName);
}

bool WADArchive::Save(wxOutputStream& oStr)
{
	// Write directory
	wxUint32 fileCount = m_entries.size();
	oStr.Write(&fileCount, sizeof(fileCount));
	
	wxInt64 currentDataOffset = 0;
	
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		wxScopedCharBuffer utf8FN = entry->GetFileName().utf8_str();
		wxUint32 fileNameLength = utf8FN.length();
		oStr.Write(&fileNameLength, sizeof(fileNameLength));
		
		oStr.Write(utf8FN.data(), fileNameLength);
		
		wxUint64 fileSize = entry->GetSize();
		oStr.Write(&fileSize, sizeof(fileSize));
		
		oStr.Write(&currentDataOffset, sizeof(currentDataOffset));
		
		currentDataOffset += entry->GetSize();
	}
	
	// Write file data
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
		Extract(*entry, oStr);
	
	m_modified = false;
	
	return true;
}

bool WADArchive::Save(const wxString& targetFileName)
{
	wxTempFileOutputStream oStr(targetFileName);
	if (!oStr.IsOk())
		return false;
	
	if (Save(oStr))
		return oStr.Commit();
	else
		return false;
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
	m_modified = true;
}

void WADArchive::ReplaceFiltered(size_t itemIndex, const wxString& sourceFileName)
{
	m_filteredEntries[itemIndex]->SetSourceFileName(sourceFileName);
	m_modified = true;
}

void WADArchive::Patch(const WADArchive& patchArchive, const WADArchiveEntry& patchEntry)
{
	// find existing entry
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (entry->GetFileName().IsSameAs(patchEntry.GetFileName()))
		{
			*entry = WADArchiveEntry(patchEntry, &patchArchive);
			return;
		}
	}

	m_entries.push_back(WADArchiveEntry(patchEntry, &patchArchive));
}

bool WADArchive::ApplyFilter(const wxString& filter)
{
	m_filteredEntries.clear();
	m_filteredEntries.reserve(m_entries.size());

	wxString lowerFilter = filter.Lower();

	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (lowerFilter.empty() ||
			entry->GetFileName().Lower().Find(lowerFilter) != wxNOT_FOUND)
			m_filteredEntries.push_back(&(*entry));
	}

	if (m_filteredEntries.empty())
	{
		// Reset filter
		ApplyFilter("");

		return false;
	} else {
		wxLogDebug("%ud items for filter %s", m_filteredEntries.size(), filter);
		return true;
	}
}
