#include "TexturePackPanel.h"

TexturePackPanel::TexturePackPanel( wxWindow* parent )
:
BaseTexturePackPanel( parent )
{

}

void TexturePackPanel::LoadTexture(wxInputStream& iStr, wxBitmap bitmap)
{
	m_bitmap = bitmap;

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

void TexturePackPanel::OnFrameSpinCtrlChanged( wxSpinEvent& event )
{
	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());
	if (tex.empty())
	{
		m_frameBitmap->SetBitmap(wxNullBitmap);
		return;
	}

	const Frame& frame = tex.at(m_frameSpinCtrl->GetValue());

	wxMemoryDC srcDC;
	srcDC.SelectObject(m_bitmap);

	wxBitmap frameBmp(frame.GetSize(), m_bitmap.GetDepth());
	wxMemoryDC dstDC;
	dstDC.SelectObject(frameBmp);
	dstDC.SetBrush(*wxWHITE_BRUSH);
	dstDC.DrawRectangle(wxPoint(0, 0), frame.GetSize());

	dstDC.Blit(wxPoint(0, 0), frameBmp.GetSize(), &srcDC, frame.GetOffset());

	dstDC.SelectObject(wxNullBitmap);

	m_frameBitmap->SetBitmap(frameBmp);
	m_framePanel->Refresh();
	m_framePanel->Layout();
}

void TexturePackPanel::OnFrameSpinCtrlEnterPressed( wxCommandEvent& event )
{
	wxSpinEvent evt;
	OnFrameSpinCtrlChanged(evt);
}
