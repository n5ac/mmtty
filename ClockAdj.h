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



//----------------------------------------------------------------------------
#ifndef ClockAdjH
#define ClockAdjH
//----------------------------------------------------------------------------
/* JA7UDE 0428
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>
*/
#include "Comlib.h"
#include "sound.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
//----------------------------------------------------------------------------
#define	AGCAVG	4
//----------------------------------------------------------------------------
class TClockAdjDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TPanel *Panel;
	TPaintBox *PaintBox;
	TEdit *EditClock;
	TLabel *Label1;
	TUpDown *UpDown;
	TTimer *Timer;
	TComboBox *MarkFreq;
	TLabel *Label2;
	TLabel *LPPM;
	TTrackBar *TBGAIN;
	TUpDown *UDMark;
	TSpeedButton *SBAGC;
	TLabel *LG;
	TSpeedButton *SBHelp;
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall UpDownClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PaintBoxPaint(TObject *Sender);
	void __fastcall PaintBoxMouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	
	void __fastcall MarkFreqChange(TObject *Sender);
	void __fastcall TBGAINChange(TObject *Sender);

	void __fastcall UDMarkClick(TObject *Sender, TUDBtnType Button);
	void __fastcall SBAGCClick(TObject *Sender);
	void __fastcall PaintBoxMouseMove(TObject *Sender, TShiftState Shift, int X,
	int Y);
	
	void __fastcall SBHelpClick(TObject *Sender);
private:
	Graphics::TBitmap *pBitmap;
	TSound	*pSound;
	CFSKDEM	*pDem;
	CTICK	*pTick;

	int		m_DisEvent;

	int		m_agcMin;
	int		m_agcMax;
	int		m_agcSumMax;

	DYNAMIC HPALETTE __fastcall GetPalette(void);
	void __fastcall UpdateTB(void);
	void __fastcall UpdatePPM(void);
	void __fastcall PaintCursor(void);
	int		m_CursorX;

	void __fastcall DrawMessage(LPCSTR p);
	double __fastcall GetPointSamp(void);
	int		m_Point;
	int		m_PointX;
	int		m_PointY;
	int		m_PointX2;
	int		m_PointY2;

	AnsiString	m_Help;
public:
	virtual __fastcall TClockAdjDlg(TComponent* AOwner);
	__fastcall TClockAdjDlg::~TClockAdjDlg();

	int __fastcall Execute(TSound *p, double &Samp);
};
//----------------------------------------------------------------------------
//extern TClockAdjDlg *ClockAdjDlg;
//----------------------------------------------------------------------------
#endif
