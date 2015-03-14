/*
** HLMWadExplorer
**
** Coypright (C) 2015 Tobias Taschner <github@tc84.de>
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
	wxUint8 version;
	iStr.Read(&version, sizeof(version));
	char id[16];
	id[15] = 0;
	iStr.Read(&id, 15);
	if (version != 0xF || wxStrcmp(id, "AGTEXTUREPACKER") != 0)
	{
		wxLogWarning("Invalid texture file format");
		return;
	}

	// Skip unknown value
	iStr.SeekI(4, wxFromCurrent);

	iStr.Read(&m_width, sizeof(m_width));
	iStr.Read(&m_height, sizeof(m_height));

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

	for (wxUint32 frameIndex = 0; frameIndex < frameCount; frameIndex++)
	{
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

	// Skip additional data (timing data?, flags?)
	iStr.SeekI(16, wxFromCurrent);
}
