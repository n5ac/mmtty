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
#ifndef FreqDispH
#define FreqDispH
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
*/
//----------------------------------------------------------------------------
#include "fir.h"
#include <System.Classes.hpp>

#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.StdCtrls.hpp>/
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>/
#include <Vcl.StdCtrls.hpp>----------------------------------------------------------------------------
class TFreqDispDlg : public TForm
{
__published:
	TButton *CancelBtn;
	TPanel *Panel;
	TPaintBox *PaintBox;
	TTimer *Timer;
	TButton *BwBtn;
	void __fastcall PaintBoxPaint(TObject *Sender);
	
	void __fastcall BwBtnClick(TObject *Sender);
private:
	Graphics::TBitmap *pBitmap;

	int			m_type;
	double		m_a10, m_a20;
	double		m_b11, m_b12;
	double		m_b21, m_b22;

	CIIR		*m_piir;

	const double *m_H1;
	int			m_Tap1;
	int			m_Over1;
	const double *m_H2;
	int			m_Tap2;
	int			m_Over2;

	double		m_HT[TAPMAX+1];
public:
	virtual __fastcall TFreqDispDlg(TComponent* AOwner);
	__fastcall ~TFreqDispDlg();

	void __fastcall Execute(const double *H, int Tap, int Over);
	void __fastcall Execute(const double *H1, const double *H2, int Tap, int Over);
	void __fastcall Execute(const double *H1, int Tap1, const double *H2, int Tap2, int Over);
	void __fastcall Execute(double a10, double b11, double b12, double a20, double b21, double b22, int Over);
	void __fastcall Execute(CIIR *ip, int Over, double max);
	void __fastcall Execute(const double *H, int Tap, int Over, double max);

	int			m_Max;
};
//----------------------------------------------------------------------------
//extern TFreqDispDlg *FreqDispDlg;
//----------------------------------------------------------------------------
#endif    
