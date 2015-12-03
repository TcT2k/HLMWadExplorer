/*
** HLMWadExplorer
**
** Copyright (C) 2015 Tobias Taschner <github@tc84.de>
**
** Licensed under GPL v3 or later
*/
#ifndef TexturePack_H
#define TexturePack_H

#include <wx/wx.h>
#include <wx/vector.h>

class TexturePack;

class Frame
{
public:
	Frame(wxInputStream& iStr);

	const wxPoint& GetOffset() const
	{
		return m_offset;
	}

	const wxSize& GetSize() const
	{
		return m_size;
	}

	wxFloat32 GetUStart() const
	{
		return m_uStart;
	}

	wxFloat32 GetVStart() const
	{
		return m_vStart;
	}

	wxFloat32 GetUEnd() const
	{
		return m_uEnd;
	}

	wxFloat32 GetVEnd() const
	{
		return m_vEnd;
	}

private:
	wxPoint m_offset;
	wxSize m_size;
	wxFloat32 m_uStart;
	wxFloat32 m_vStart;
	wxFloat32 m_uEnd;
	wxFloat32 m_vEnd;

	void Parse(wxInputStream& iStr);
};

class Texture: public wxVector<Frame>
{
public:
	Texture(TexturePack* pack, wxInputStream& iStr);

	const wxString& GetName() const
	{
		return m_name;
	}

private:
	TexturePack* m_pack;
	wxString m_name;

	void Parse(wxInputStream& iStr);
};

class TexturePack: public wxVector<Texture>
{
public:
	TexturePack(wxInputStream& iStr);

	wxUint32 GetVersion() const
	{
		return m_version;
	}

private:
	wxUint32 m_width;
	wxUint32 m_height;
	wxUint32 m_version;

	void Parse(wxInputStream& iStr);

};

#endif // TexturePack_H
