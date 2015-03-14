#include "TexturePackPanel.h"

TexturePackPanel::TexturePackPanel( wxWindow* parent )
:
BaseTexturePackPanel( parent )
{

}

void TexturePackPanel::LoadTexture(wxInputStream& iStr, wxBitmap bitmap)
{
	m_bitmap = bitmap;
	wxSharedPtr<wxGraphicsContext> dstGC(wxGraphicsContext::Create(this));
	m_drawBitmap = dstGC->CreateBitmap(bitmap);

	m_texturePack = new TexturePack(iStr);

	m_textureListBox->Clear();

	for (auto texture = m_texturePack->begin(); texture != m_texturePack->end(); ++texture)
	{
		m_textureListBox->Append(texture->GetName());
	}

	m_textureListBox->Select(0);
	wxCommandEvent evt;
	OnTextureListBoxSelected(evt);
}

void TexturePackPanel::OnTextureListBoxSelected( wxCommandEvent& event )
{
	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());

	m_frameSpinCtrl->SetMax(tex.size() - 1);
	m_frameSpinCtrl->SetValue(0);

	wxSpinEvent evt;
	OnFrameSpinCtrlChanged(evt);
}

void TexturePackPanel::OnFrameSpinCtrlChanged(wxSpinEvent& event)
{
	UpdateFrameImage();
}

void TexturePackPanel::UpdateFrameImage()
{
	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());
	if (tex.empty())
	{
		m_frameBitmap->SetBitmap(wxNullBitmap);
		return;
	}

	const Frame& frame = tex.at(m_frameSpinCtrl->GetValue());

	wxSize imgSize = frame.GetSize();
	imgSize *= m_zoomSpinCtrl->GetValue();

	wxBitmap frameBmp(imgSize, m_bitmap.GetDepth());
	wxMemoryDC dstDC;
	dstDC.SelectObject(frameBmp);
	dstDC.SetBrush(wxBrush(m_colourPicker->GetColour()));
	dstDC.DrawRectangle(wxPoint(0, 0), imgSize);

	wxSharedPtr<wxGraphicsContext> dstGC(wxGraphicsContext::Create(dstDC));
	dstGC->SetAntialiasMode(wxANTIALIAS_NONE);
	wxGraphicsBitmap srcSubBmp = dstGC->CreateSubBitmap(m_drawBitmap, frame.GetOffset().x, frame.GetOffset().y, frame.GetSize().GetWidth(), frame.GetSize().GetHeight());
	dstGC->DrawBitmap(srcSubBmp, 0, 0, imgSize.GetWidth(), imgSize.GetHeight());
	dstGC.reset();

	dstDC.SelectObject(wxNullBitmap);

	m_frameBitmap->SetBitmap(frameBmp);
	m_framePanel->Refresh();
	m_framePanel->Layout();
}

void TexturePackPanel::OnFrameSpinCtrlEnterPressed( wxCommandEvent& event )
{
	UpdateFrameImage();
}

void TexturePackPanel::OnColourPickerChanged(wxColourPickerEvent& event)
{
	UpdateFrameImage();
}

void TexturePackPanel::OnZoomSpinCtrlChanged(wxSpinEvent& event)
{
	UpdateFrameImage();
}
