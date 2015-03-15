#include "TexturePackPanel.h"

#include <wx/imaggif.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <wx/anidecod.h>
#include <wx/wfstream.h>
#include <wx/quantize.h>

TexturePackPanel::TexturePackPanel( wxWindow* parent )
:
BaseTexturePackPanel( parent )
{

}

void TexturePackPanel::LoadTexture(wxInputStream& iStr, wxBitmap bitmap)
{
	m_bitmap = bitmap;
	wxSharedPtr<wxGraphicsContext> dstGC(wxGraphicsContext::Create(this));
	if (m_bitmap.IsOk())
		m_drawBitmap = dstGC->CreateBitmap(bitmap);

	m_exportGIFButton->Disable();
	m_frameDelaySpinCtrl->Disable();

	m_texturePack = new TexturePack(iStr);

	m_textureListBox->Freeze();
	m_textureListBox->Clear();

	for (auto texture = m_texturePack->begin(); texture != m_texturePack->end(); ++texture)
	{
		m_textureListBox->Append(texture->GetName());
	}
	m_textureListBox->Thaw();

	if (!m_texturePack->empty())
	{
		m_textureListBox->Select(0);
		wxCommandEvent evt;
		OnTextureListBoxSelected(evt);
	}
}

void TexturePackPanel::OnTextureListBoxSelected( wxCommandEvent& event )
{
	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());

	m_frameSpinCtrl->SetMax(tex.size() - 1);
	m_frameSpinCtrl->SetValue(0);
	m_exportGIFButton->Enable(tex.size() > 1);
	m_frameDelaySpinCtrl->Enable(tex.size() > 1);

	wxSpinEvent evt;
	OnFrameSpinCtrlChanged(evt);
}

void TexturePackPanel::OnFrameSpinCtrlChanged(wxSpinEvent& event)
{
	UpdateFrameImage();

	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());
	const Frame& frame = tex.at(m_frameSpinCtrl->GetValue());

	m_sizeStaticText->SetLabel(wxString::Format("%dx%d", frame.GetSize().GetWidth(), frame.GetSize().GetHeight()));
	m_offsetStaticText->SetLabel(wxString::Format("%d,%d", frame.GetOffset().x, frame.GetOffset().y));

	m_infoSizer->Layout();
}

void TexturePackPanel::UpdateFrameImage()
{
	const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());
	if (tex.empty() || !m_bitmap.IsOk())
	{
		m_frameBitmap->SetBitmap(wxNullBitmap);
		return;
	}

	const Frame& frame = tex.at(m_frameSpinCtrl->GetValue());


	m_frameBitmap->SetBitmap(GetFrameBitmap(frame, m_zoomSpinCtrl->GetValue()));
	m_framePanel->Refresh();
	m_framePanel->Layout();
}

wxBitmap TexturePackPanel::GetFrameBitmap(const Frame frame, int zoom)
{
	wxSize imgSize = frame.GetSize();
	imgSize *= zoom;

	wxBitmap frameBmp(imgSize, m_bitmap.GetDepth());
	wxMemoryDC dstDC;
	dstDC.SelectObject(frameBmp);
	dstDC.SetBrush(wxBrush(m_colourPicker->GetColour()));
	dstDC.SetPen(wxPen(m_colourPicker->GetColour()));
	dstDC.DrawRectangle(wxPoint(0, 0), imgSize);

	wxSharedPtr<wxGraphicsContext> dstGC(wxGraphicsContext::Create(dstDC));
	dstGC->SetAntialiasMode(wxANTIALIAS_NONE);
	wxGraphicsBitmap srcSubBmp = dstGC->CreateSubBitmap(m_drawBitmap, frame.GetOffset().x, frame.GetOffset().y, frame.GetSize().GetWidth(), frame.GetSize().GetHeight());
	dstGC->DrawBitmap(srcSubBmp, 0, 0, imgSize.GetWidth(), imgSize.GetHeight());
	dstGC.reset();

	return frameBmp;
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

void TexturePackPanel::OnExportGIFClicked(wxCommandEvent& event)
{
	wxFileName exFileName(m_textureListBox->GetString(m_textureListBox->GetSelection()), wxPATH_UNIX);
	exFileName.SetExt("gif");
	wxString defaultFileName = exFileName.GetFullName();

	wxFileDialog fileDlg(this, _("Select target gif"),
		wxStandardPaths::Get().GetDocumentsDir(),
		defaultFileName, "*.gif", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		wxBusyCursor busyCursor;

		wxImageArray images;
		const Texture& tex = m_texturePack->at(m_textureListBox->GetSelection());
		for (auto frame = tex.begin(); frame != tex.end(); ++frame)
		{
			wxImage frameImg(GetFrameBitmap(*frame).ConvertToImage());

			wxQuantize::Quantize(frameImg, frameImg, 256);

			images.Add(frameImg);
		}

		wxGIFHandler* gifHandler = (wxGIFHandler*) wxImage::FindHandler(wxBITMAP_TYPE_GIF);

		wxTempFileOutputStream oStr(fileDlg.GetPath());

		gifHandler->SaveAnimation(images, &oStr, true, m_frameDelaySpinCtrl->GetValue());
		oStr.Commit();
	}
}
