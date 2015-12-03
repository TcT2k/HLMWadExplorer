/*
** HLMWadExplorer
**
** Copyright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/

#include "TexturePack.h"

//
// TexturePack
//

TexturePack::TexturePack(wxInputStream& iStr)
{
	Parse(iStr);
}

void TexturePack::Parse(wxInputStream& iStr)
{
	clear();

	// Check header
	wxUint8 idLen;
	iStr.Read(&idLen, sizeof(idLen));
	char id[16];
	id[15] = 0;
	iStr.Read(&id, 15);
	if (idLen != 0xF || wxStrcmp(id, "AGTEXTUREPACKER") != 0)
	{
		wxLogWarning("Invalid texture file format");
		return;
	}

	iStr.Read(&m_version, sizeof(m_version));

	if (m_version >= 2)
	{
		// Skip slice count
		iStr.SeekI(4, wxFromCurrent);
	} else {
		iStr.Read(&m_width, sizeof(m_width));
		iStr.Read(&m_height, sizeof(m_height));
	}

	// Read textures until file ends
	while (!iStr.Eof())
	{
		Texture tex(this, iStr);

		if (!iStr.Eof())
			push_back(tex);
	}
}

//
// Texture
//

Texture::Texture(TexturePack* pack, wxInputStream& iStr):
	m_pack(pack)
{
	Parse(iStr);
}

void Texture::Parse(wxInputStream& iStr)
{
	wxUint8 nameLen;
	iStr.Read(&nameLen, sizeof(nameLen));
	if (iStr.Eof())
		return;

	wxCharBuffer nameBuf(nameLen);
	iStr.Read(nameBuf.data(), nameLen);
	m_name = nameBuf;

	wxUint32 frameCount;
	iStr.Read(&frameCount, sizeof(frameCount));
	reserve(frameCount);

	for (wxUint32 frameIndex = 0; frameIndex < frameCount; frameIndex++)
	{
		// skip atlas index
		if (m_pack->GetVersion() >= 2)
			iStr.SeekI(4, wxFromCurrent);

		push_back(Frame(iStr));
	}
}

//
// Frame
//
Frame::Frame(wxInputStream& iStr)
{
	Parse(iStr);
}

void Frame::Parse(wxInputStream& iStr)
{
	wxUint32 frameWidth;
	wxUint32 frameHeight;
	iStr.Read(&frameWidth, sizeof(frameWidth));
	iStr.Read(&frameHeight, sizeof(frameHeight));
	m_size = wxSize(frameWidth, frameHeight);

	wxUint32 offsetX;
	wxUint32 offsetY;
	iStr.Read(&offsetX, sizeof(offsetX));
	iStr.Read(&offsetY, sizeof(offsetY));
	m_offset = wxPoint(offsetX, offsetY);

	iStr.Read(&m_uStart, sizeof(m_uStart));
	iStr.Read(&m_vStart, sizeof(m_vStart));
	iStr.Read(&m_uEnd, sizeof(m_uEnd));
	iStr.Read(&m_vEnd, sizeof(m_vEnd));
}
