//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMTTY.

// MMTTY is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMTTY is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY.  If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "ClockAdj.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TClockAdjDlg *ClockAdjDlg;
static int	s_Gain = 0x0108;
//---------------------------------------------------------------------
__fastcall TClockAdjDlg::TClockAdjDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	pBitmap = new Graphics::TBitmap();
	pBitmap->Width = PaintBox->Width;
	pBitmap->Height = PaintBox->Height;
//	pBitmap->Palette = MmttyWd->UsrPal;

	switch(SampType){
		case 0:
			UpDown->Max = 11599;
			UpDown->Min = 10000;
			break;
		case 1:
			UpDown->Max = 9999;
			UpDown->Min = 7000;
			break;
		case 2:
			UpDown->Max = 6999;
			UpDown->Min = 5000;
			break;
		case 3:
			UpDown->Max = 12500;
			UpDown->Min = 11600;
			break;
	}
	TRect rc;
	int XL = 0;
	int XR = pBitmap->Width - 1;
	int YT = 0;
	int YB = pBitmap->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	TCanvas *cp = pBitmap->Canvas;
	cp->Brush->Color = clBlack;
	cp->FillRect(rc);

	m_CursorX = XR - 20;

	pDem = NULL;
	pTick = NULL;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	cp->Font->Name = ((TForm *)AOwner)->Font->Name;
	cp->Font->Charset = ((TForm *)AOwner)->Font->Charset;
	cp->Font->Color = clWhite;
	cp->Font->Size = 10;
	int FH = cp->TextHeight("A");
	int Y = 5;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = "Calibrating the Sound Card with a Time Standard Broadcast Station";
//        "Adjust Sampling Frequency using an off-air time signal";
//		Caption = "Adjust Sampling freq. (Recive WWV tick sound on Mark freq.)";
		CancelBtn->Caption = "Cancel";

		cp->TextOut(10, Y, "1.Tune your radio to WWV or another 1-second tick standard.  Set the radio display"); Y+=FH;
		cp->TextOut(10, Y, "  to the carrier frequency."); Y+=FH;
		cp->TextOut(10, Y, "2.Wait for about three minutes.  If you are using 11025 Hz. calibration, you should"); Y+=FH;
		cp->TextOut(10, Y, "  see two lines of marks (vertical or slanted), corresponding to the 1-second tick"); Y+=FH;
		cp->TextOut(10, Y, "  sound bursts transmitted by the time-standard broadcast station.  You will see only"); Y+=FH;
		cp->TextOut(10, Y, "  one line with 8000 Hz. or 6000 Hz."); Y+=FH;
		cp->TextOut(10, Y, "3.Right-click to move the vertical green line to the tick line.  This allows you to"); Y+=FH;
		cp->TextOut(10, Y, "  compare the tick line to vertical."); Y+=FH;
		cp->TextOut(10, Y, "4.Left-click a low tick burst mark (bottom one if possible), and move the cursor to"); Y+=FH;
		cp->TextOut(10, Y, "  the top of the line.  You will see a yellow line on the display."); Y+=FH;
		cp->TextOut(10, Y, "5.Overlay the yellow line with the tick mark line, and left-click a high burst mark"); Y+=FH;
		cp->TextOut(10, Y, "  (top one if possible)."); Y+=FH;
		cp->TextOut(10, Y, "6.This will automatically put the correct clock frequency in the adjust window."); Y+=FH;
		cp->TextOut(10, Y, "7.Click OK to leave the setup display and to memorize the new value."); Y+=FH;
		cp->TextOut(10, Y, "8.Restart MMTTY for the new clock value to take effect.");

#if 0
		cp->TextOut(10, 10, "1.Receive WWV in AM or SSB mode.  Tune the radio to the carrier frequency.");
		cp->TextOut(10, 30, "2.Let this adjustment display run for a few minutes, and look for one or two");
		cp->TextOut(10, 50, "  almost-vertical lines of dots.  If you do not see this, wait for up to ten minutes.");
		cp->TextOut(10, 70, "3.If your sound card clock is far out of adjustment, the lines will be very slanted.");
		cp->TextOut(10, 90, "4.Click the 1st point on the slanted line.");
		cp->TextOut(10,110, "5.And then click the 2nd point on the same line. The value of clock will be revised");
		cp->TextOut(10,130, "  automatically.  If there is distance of 1st point and 2nd point, precision is better.");
		cp->TextOut(10,160, "If the first degree of slant is very big, please repeat once again, because an error");
		cp->TextOut(10,180, "remains a little.");
		cp->TextOut(10,200, "WWV - 2.5, 5, 10MHz // GBR - 60KHz // RWM - 4.996, 9.996, 14.996MHz");
		cp->TextOut(10,240, "Left click  - Start adjustment");
		cp->TextOut(10,260, "Right click - Move vertical cursor");
#endif
	}
	else {
		cp->TextOut(10, Y, "1.JJYをAMまたはSSBで受信し、この画面の下にあるMarkに1000Hzまたは"); Y+=FH;
		cp->TextOut(10, Y, "  1600Hzを設定します."); Y+=FH;
		cp->TextOut(10, Y, "2.SSBの場合は１秒チック音がMarkに重なるように受信周波数を調整します."); Y+=FH;
		cp->TextOut(10, Y, "3.しばらく受信して縦または斜めラインが表示されるのを待ちます（数分）."); Y+=FH;
		cp->TextOut(10, Y, "4.線が垂直の場合はクロック値は正確です。斜めになっている場合は、"); Y+=FH;
		cp->TextOut(10, Y, "  その斜め線の最初の端点をクリックします."); Y+=FH;
		cp->TextOut(10, Y, "5.更に同じ斜め線の反対側の端点をクリックするとクロック値が自動的に"); Y+=FH;
		cp->TextOut(10, Y, "  設定されます.（2点間の距離が長いほうが正確です）"); Y+=FH;
		cp->TextOut(10, Y, "傾斜が非常に大きい場合、上記の手続きをもう一度繰り返して下さい."); Y+=FH+FH;
		cp->TextOut(10, Y, "JJYの代わりに、JMH,JMGなどのFAX放送の0.5秒毎のデッドセクタを利用しても"); Y+=FH;
		cp->TextOut(10, Y, "OKのようです."); Y+=FH;
		cp->TextOut(10, Y, "左クリック - 調整の開始"); Y+=FH;
		cp->TextOut(10, Y, "右クリック - 垂直カーソル移動");
	}
	m_agcMax = 8192;
	m_agcMin = 0;
	m_agcSumMax = 8192 * AGCAVG;
	m_Point = 0;
//	SBHelp->Visible = !JanHelp.IsEmpty();
	SBHelp->Visible = FALSE;
}
//---------------------------------------------------------------------
__fastcall TClockAdjDlg::~TClockAdjDlg()
{
	pDem = NULL;
	pTick = NULL;
	pBitmap->Palette = NULL;
	delete pBitmap;
	pBitmap = NULL;
}
//---------------------------------------------------------------------------
// 現在の論理パレットを返す（TControl::GetPaletteのオーバライド関数）
HPALETTE __fastcall TClockAdjDlg::GetPalette(void)
{
	pBitmap->Palette = MmttyWd->UsrPal;
	return MmttyWd->UsrPal;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UpdateTB(void)
{
	TBGAIN->Enabled = !SBAGC->Down;
	LG->Font->Color = SBAGC->Down ? clGrayText : clBlack;
}
//---------------------------------------------------------------------
void __fastcall TClockAdjDlg::UpdatePPM(void)
{
	int d = ((UpDown->Position - SampBase) * 1000000.0 / SampBase) + 0.5;

	char bf[64];
	sprintf(bf, "%d ppm", d);
	LPPM->Caption = bf;
}
//---------------------------------------------------------------------
int __fastcall TClockAdjDlg::Execute(TSound *p, double &Samp)
{
	pSound = p;
	pDem = &p->FSKDEM;
	pTick = &pDem->Tick;
	pTick->m_Samp = int(Samp);
	InitColorTable(clBlack, clWhite);
	MmttyWd->ReqPaletteChange();
	if( (Samp > UpDown->Max) || (Samp < UpDown->Max) ){
		Samp = SampFreq;
	}
	EditClock->Text = int(Samp);
	UpDown->Position = SHORT(Samp);
	pDem->Tick.Init();
	pDem->m_Tick = 1;
	m_DisEvent++;
	double SaveMarkFreq = pDem->GetMarkFreq();
	double SaveSpaceFreq = pDem->GetSpaceFreq();
	int SaveAFC = sys.m_AFC;
	int SaveBPF = pSound->m_bpf;
	int	SaveType = pSound->FSKDEM.m_type;
	pSound->m_bpf = 0;
	sys.m_AFC = 0;
	if( SaveType == 2 ) pSound->FSKDEM.m_type = 0;
	UDMark->Position = 1000;
	pDem->SetMarkFreq(1000.0);
	pDem->SetSpaceFreq(1200.0);
	TBGAIN->Position = s_Gain & 0x00ff;
	SBAGC->Down = s_Gain & 0xff00 ? 1 : 0;
	UpdateTB();
	m_DisEvent--;
	UpdatePPM();
	int r = ShowModal();
	InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
	s_Gain = (s_Gain & 0x00ff) | (SBAGC->Down ? 0x0100 : 0);
	pDem->m_Tick = 0;
	pDem->SetMarkFreq(SaveMarkFreq);
	pDem->SetSpaceFreq(SaveSpaceFreq);
	sys.m_AFC = SaveAFC;
	pSound->m_bpf = SaveBPF;
	pSound->FSKDEM.m_type = SaveType;
	MmttyWd->ReqPaletteChange();
	if( r == IDOK ){
		Samp = UpDown->Position;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TClockAdjDlg::TimerTimer(TObject *Sender)
{
	if( pBitmap == NULL ) return;
	if( pDem == NULL ) return;
	if( pTick == NULL ) return;

	int *p = pTick->GetData();
	if( p == NULL ) return;

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	int XL = 0;
	int XR = pBitmap->Width - 1;
	int YT = 0;
	int YB = pBitmap->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;

	TRect src(rc);
	src.Bottom--;
	rc.Top++;
	tp->CopyRect(rc, tp, src);

	int mx = -MAXINT;
	int mi = MAXINT;
	int x, xx, d;
	double k1 = 16384.0/double(m_agcMax - m_agcMin);
	double k2;
	if( SBAGC->Down ){
		k2 = 128.0 / 16384.0;
	}
	else {
		k2 = (128.0 * (s_Gain & 0x00ff)) / (4096 * 16);
	}
	for( x = 0; x < XR; x++){
		xx = (x * pTick->m_Samp)/pBitmap->Width;
		d = p[xx];
		if( mx < d ) mx = d;
		if( mi > d ) mi = d;
		if( SBAGC->Down ){
			d -= m_agcMin;
			if( m_agcMax > 0 ) d = double(d) * k1;
		}
		d = double(d) * k2;

		if( d >= 128 ) d = 127;
		if( d <= 0 ) d = 0;
		tp->Pixels[x][0] = TColor(ColorTable[127-d] | sys.d_PaletteMask);
	}
	m_agcSumMax -= m_agcMax;
	m_agcSumMax += mx;
	if( m_agcSumMax < (4096*AGCAVG) ) m_agcSumMax = (4096*AGCAVG);
	m_agcMax = m_agcSumMax / AGCAVG;
	m_agcMin = mi;
	if( m_agcMin > 2048 ) m_agcMin = 2048;
	m_PointY++;
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
#if 0
	PaintBox->Canvas->Font->Color = clWhite;
	char bf[128];
	sprintf(bf, "curMax:%d", mx);
	PaintBox->Canvas->TextOut( 500, 200, bf );
	sprintf(bf, "agcMax:%d", m_agcMax);
	PaintBox->Canvas->TextOut( 500, 220, bf );
	sprintf(bf, "agcMin:%d", m_agcMin);
	PaintBox->Canvas->TextOut( 500, 240, bf );
	sprintf(bf, "SumMax:%d", m_agcSumMax);
	PaintBox->Canvas->TextOut( 500, 260, bf );
#endif
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::DrawMessage(LPCSTR p)
{
	int xr = PaintBox->Canvas->TextWidth(p);
	int xl = (PaintBox->Width - xr)/2;
	xr += xl;
	int FH = PaintBox->Canvas->TextHeight(p);
	int VC = PaintBox->Height - FH;
	PaintBox->Canvas->Pen->Color = clWhite;
	PaintBox->Canvas->Brush->Color = clBlack;
	PaintBox->Canvas->RoundRect(xl-10, VC-FH, xr+10, VC+FH, 10, 10);
	PaintBox->Canvas->Font->Color = clWhite;
	PaintBox->Canvas->TextOut(xl, VC-FH/2, p);
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintCursor(void)
{
	PaintBox->Canvas->Pen->Color = clLime;
	PaintBox->Canvas->Pen->Style = psDot;
	PaintBox->Canvas->MoveTo(m_CursorX, 0);
	int rop = ::SetROP2(PaintBox->Canvas->Handle, R2_MASKPENNOT);
	PaintBox->Canvas->LineTo(m_CursorX, pBitmap->Height - 1);
	::SetROP2((HDC)PaintBox->Canvas, rop);
	if( m_Point ){
		PaintBox->Canvas->Pen->Color = clYellow;
		PaintBox->Canvas->Pen->Style = psSolid;
		PaintBox->Canvas->MoveTo(m_PointX, m_PointY);
		::SetROP2(PaintBox->Canvas->Handle, R2_MASKPENNOT);
		PaintBox->Canvas->LineTo(m_PointX2, m_PointY2);
		::SetROP2((HDC)PaintBox->Canvas, rop);
		char bf[256];
		if( Font->Charset != SHIFTJIS_CHARSET ){
			sprintf( bf, "OK : Left button,  Cancel : Right button   Clock=%.1lf", GetPointSamp());
		}
		else {
			sprintf( bf, "決定 : 左ボタン,  中止 : 右ボタン   Clock=%.1lf", GetPointSamp());
		}
		DrawMessage(bf);
		if( PaintBox->Cursor != crCross ) PaintBox->Cursor = crCross;
	}
	else {
		if( PaintBox->Cursor != crDefault ) PaintBox->Cursor = crDefault;
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UpDownClick(TObject *Sender, TUDBtnType Button)
{
	if( pTick == NULL ) return;

	pTick->m_Samp = UpDown->Position;
	UpdatePPM();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxPaint(TObject *Sender)
{
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxMouseUp(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		if( m_Point ){
			m_Point = 0;
			if( m_PointY2 != m_PointY ){
				int newc = int(GetPointSamp() + 0.5);
				pTick->m_Samp = newc;
				UpDown->Position = SHORT(newc);
				ModalResult = mrOk;
			}
		}
		else {
			m_PointX2 = m_PointX = X;
			m_PointY2 = m_PointY = Y;
			m_Point = 1;
		}
	}
	else if( m_Point ){
		m_Point = 0;
	}
	else {
		m_CursorX = X;
	}
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::MarkFreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	int	dd;
	if( sscanf(AnsiString(MarkFreq->Text).c_str(), "%lu", &dd ) == 1){	//JA7UDE 0428
		if( (dd >= 300) && (dd <= 2700) ){
			m_DisEvent++;
			UDMark->Position = SHORT(dd);
			pDem->SetMarkFreq(dd);
			pDem->SetSpaceFreq(dd + 200);
			m_DisEvent--;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::TBGAINChange(TObject *Sender)
{
	s_Gain = (s_Gain & 0xff00) | TBGAIN->Position;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UDMarkClick(TObject *Sender, TUDBtnType Button)
{
	if( m_DisEvent ) return;

	pDem->SetMarkFreq(UDMark->Position);
	pDem->SetSpaceFreq(UDMark->Position + 200);
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::SBAGCClick(TObject *Sender)
{
	UpdateTB();
}
//---------------------------------------------------------------------------
double __fastcall TClockAdjDlg::GetPointSamp(void)
{
	if( m_PointY == m_PointY2 ) return pTick->m_Samp;
	double d = (m_PointX2 - m_PointX);
	d = d * pTick->m_Samp / pBitmap->Width;
	d /= (m_PointY - m_PointY2);
	d += pTick->m_Samp;
	return d;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxMouseMove(TObject *Sender,
	TShiftState Shift, int X, int Y)
{
	if( m_Point ){
		m_PointX2 = X;
		m_PointY2 = Y;
		PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
		PaintCursor();
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::SBHelpClick(TObject *Sender)
{
#if 1
	ShowHtmlHelp();
#else
	ShowHelp(25);
#endif
}
//---------------------------------------------------------------------------
