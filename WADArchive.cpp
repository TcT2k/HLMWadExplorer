/*
** HLMWadExplorer
**
** Coypright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include "WADArchive.h"

#include <wx/wfstream.h>

WADArchive::WADArchive(const wxString& fileName):
	m_fileName(fileName)
{
	ParseFile();
}

void WADArchive::ParseFile()
{
	wxFileInputStream iStr(m_fileName);

	wxUint32 fileCount;
	iStr.Read(&fileCount, sizeof(fileCount));
	wxLogDebug("Found %d files in wad:", fileCount, m_fileName);

	m_entries.reserve(fileCount);

	for (size_t fileIndex = 0; fileIndex < fileCount; fileIndex++)
	{
		wxUint32 fileNameLength;
		iStr.Read(&fileNameLength, sizeof(fileNameLength));
		
		wxCharBuffer fnBuf(fileNameLength);
		iStr.Read(fnBuf.data(), fileNameLength);
		wxString fileName(fnBuf);

		wxUint64 fileSize;
		iStr.Read(&fileSize, sizeof(fileSize));

		wxUint64 fileOffset;
		iStr.Read(&fileOffset, sizeof(fileOffset));

		m_entries.push_back( WADArchiveEntry(fileName, fileSize, fileOffset) );
	}

	m_dataOffset = iStr.SeekI(0, wxFromCurrent);
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
	wxFileInputStream iStr(m_fileName);
	iStr.SeekI(m_dataOffset + entry.GetOffset());

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
