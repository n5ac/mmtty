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



//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <math.h>
#include "fir.h"

//-------------------------------------------------
// ＦＩＲフィルタのたたき込み演算
double __fastcall DoFIR(double *hp, double *zp, double d, int tap)
{
	memcpy(zp, &zp[1], sizeof(double)*tap);
	zp[tap] = d;
	d = 0.0;
	for( int i = 0; i <= tap; i++, hp++, zp++ ){
		d += (*zp) * (*hp);
	}
	return d;
}
//---------------------------------------------------------------------------
CIIRTANK::CIIRTANK()
{
	b1 = b2 = a0 = z1 = z2 = 0;
	SetFreq(2000.0, SampFreq, 50.0);
}
//---------------------------------------------------------------------------
void CIIRTANK::SetFreq(double f, double smp, double bw)
{
	double lb1, lb2, la0;
	lb1 = 2 * exp(-PI * bw/smp) * cos(2 * PI * f / smp);
	lb2 = -exp(-2*PI*bw/smp);
	if( bw ){
#if 0
		const double _gt[]={18.0, 26.0, 20.0, 20.0};
		la0 = sin(2 * PI * f/smp) / (_gt[SampType] * 50 / bw);
#else
		la0 = sin(2 * PI * f/smp) / ((smp/6.0) / bw);
#endif
	}
	else {
		la0 = sin(2 * PI * f/smp);
	}
	b1 = lb1; b2 = lb2; a0 = la0;
}
//---------------------------------------------------------------------------
double CIIRTANK::Do(double d)
{
	d *= a0;
	d += (z1 * b1);
	d += (z2 * b2);
	z2 = z1;
	if( fabs(d) < 1e-37 ) d = 0.0;
	z1 = d;
	return d;
}
//---------------------------------------------------------------------------
CLMS::CLMS()
{
	Z = new double[TAPMAX+1];
	H = new double[TAPMAX+1];
	D = new double[DELAYMAX+1];
	memset(Z, 0, sizeof(double[TAPMAX+1]));
	memset(H, 0, sizeof(double[TAPMAX+1]));
	memset(D, 0, sizeof(double[DELAYMAX+1]));

	m_lmsADJSC = 1.0 / double(32768 * 32768);			// スケール調整値
	m_lmsErr = m_lmsMErr = 0;

	m_Type = 1;					// 0-LMS, 1-NOTICH
	m_lmsNotch = 0;
	m_lmsNotch2 = 0;
	m_twoNotch = 0;
	m_Tap = 56;
	m_NotchTap = 72;
	m_lmsInv = 0;
	m_lmsDelay = 0;				// LMS Delay
	m_lmsAGC = 0;				// LMS AGC
	m_lmsMU2 = 0.003;			// LMS 2μ
	m_lmsGM = 0.9999;			// LMS γ
	m_bpf = 1;
	SetWindow(2125, 2125+170);
}

CLMS::~CLMS()
{
	delete[] D;
	delete[] H;
	delete[] Z;
}

void CLMS::Copy(CLMS &other)
{
	m_Type = other.m_Type;
	m_Tap = other.m_Tap;
	m_NotchTap = other.m_NotchTap;
	m_lmsInv = other.m_lmsInv;			// LMS InvOutput
	m_lmsDelay = other.m_lmsDelay;		// LMS Delay
	m_lmsAGC = other.m_lmsAGC;			// LMS AGC
	m_lmsMU2 = other.m_lmsMU2;			// LMS 2μ
	m_lmsGM = other.m_lmsGM;			// LMS γ
	m_lmsNotch = other.m_lmsNotch;
	m_lmsNotch2 = other.m_lmsNotch2;
	m_twoNotch = other.m_twoNotch;
	m_bpf = other.m_bpf;
	SetWindow(m_MarkFreq, m_SpaceFreq);
}

void CLMS::GetFW(double &fl, double &fh, double fq)
{
	double fw;
	if( fq < m_MarkFreq ){
		fw = m_MarkFreq - fq;
		if( fw < 80.0 ){
			fw = 15.0;
		}
		else {
			fw *= 0.5;
		}
	}
	else if( fq > m_SpaceFreq ){
		fw = fq - m_SpaceFreq;
		if( fw < 80.0 ){
			fw = 15.0;
		}
		else {
			fw *= 0.5;
		}
	}
	else {
		fq = (m_MarkFreq + m_SpaceFreq)/2.0;
		fw = 15.0;
	}
	fh = fq + fw;
	fl = fq - fw;
}

void CLMS::SetWindow(double mfq, double sfq)
{
	m_MarkFreq = mfq;
	m_SpaceFreq = sfq;
	if( m_Type ){
		if( m_lmsNotch && (m_lmsNotch2 == m_lmsNotch) ) m_lmsNotch2 = 0;
		double fl, fh;
		double c = (m_MarkFreq + m_SpaceFreq)/2;
		if( !m_lmsNotch || ((m_lmsNotch >= m_MarkFreq) && (m_lmsNotch <= m_SpaceFreq)) ){
			m_lmsNotch = c;
			GetFW(fl, fh, c);
			MakeFilter(H, m_NotchTap, ffBEF, SampFreq, fl, fh, 10, 1.0);
			if( (m_lmsNotch2 >= m_MarkFreq) && (m_lmsNotch2 <= m_SpaceFreq) ){
				m_lmsNotch2 = 0;
			}
			if( m_lmsNotch2 ){
				GetFW(fl, fh, m_lmsNotch2);
				MakeFilter(HBPF, m_NotchTap, ffBEF, SampFreq, fl, fh, 10, 1.0);
			}
		}
		else {
			GetFW(fl, fh, m_lmsNotch);
			MakeFilter(H, m_NotchTap, ffBEF, SampFreq, fl, fh, 10, 1.0);
			if( (m_lmsNotch2 >= m_MarkFreq) && (m_lmsNotch2 <= m_SpaceFreq) ){
				m_lmsNotch2 = c;
			}
			if( m_lmsNotch2 ){
				GetFW(fl, fh, m_lmsNotch2);
				MakeFilter(HBPF, m_NotchTap, ffBEF, SampFreq, fl, fh, 10, 1.0);
			}
		}
	}
	else {
		MakeFilter(HBPF, m_Tap, ffBPF, SampFreq, m_MarkFreq - 150, m_SpaceFreq + 150, 60, 0.00002);
	}
}
//-------------------------------------------------
// 適応フィルタの演算
double CLMS::Do(double d)
{
	double a = 0.0;
	int i;
	double *zp = Z;
	double *hp = H;
	if( m_Type ){
		if( !m_NotchTap ) return d;	// スルーの時
		// ノッチフィルタ
		memcpy(Z, &Z[1], sizeof(double)*m_NotchTap);
		Z[m_NotchTap] = d;
		for( i = 0; i <= m_NotchTap; i++, zp++, hp++ ){
			a += (*zp) * (*hp);
		}
		if( m_lmsNotch2 && m_twoNotch ){
			memcpy(D, &D[1], sizeof(double)*m_NotchTap);
			D[m_NotchTap] = a;
			zp = D;
			hp = HBPF;
			a = 0;
			for( i = 0; i <= m_NotchTap; i++, zp++, hp++ ){
				a += (*zp) * (*hp);
			}
		}
		return a;
	}
	else {
		if( !m_Tap ) return d;	// スルーの時
		// トランスバーサルフィルタ
		memcpy(Z, &Z[1], sizeof(double)*m_Tap);
		Z[m_Tap] = D[0];
		for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
			a += (*zp) * (*hp);
		}
	}
	// 誤差計算
	m_lmsErr = d - a;
	m_lmsMErr = m_lmsErr * m_lmsMU2 * m_lmsADJSC;	// lmsADJSC = 1/(32768 * 32768) スケーリング調整値

	// 遅延器の移動
	if( m_lmsDelay ) memcpy(D, &D[1], sizeof(double)*m_lmsDelay);
	D[m_lmsDelay] = d;

	// 係数更新
	zp = Z;
	hp = H;
	if( m_lmsAGC ){
		double sum = 0.0;
		for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
			*hp = (m_lmsMErr * (*zp)) + (*hp * m_lmsGM);
			if( m_bpf ) *hp += HBPF[i];
			sum += fabs(*hp);
		}
		if( sum >= 1e-3 ) a /= sum;
	}
	else {
		for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
			*hp = (m_lmsMErr * (*zp)) + (*hp * m_lmsGM);
			if( m_bpf ) *hp += HBPF[i];
		}
	}
	return m_lmsInv ? m_lmsErr : a;
}

//*********************************************************************
// CDECM2	1/2デシメータ処理クラス
//
CDECM2::CDECM2()
{
	memset(Z1, 0, sizeof(Z1));
	memset(Z2, 0, sizeof(Z2));
	::MakeFilter(H, 36, ffLPF, SampFreq, 2900, 3000, 70, 1.0);
}

double CDECM2::Do(double d1, double d2)
{
	memcpy(Z1, &Z1[1], sizeof(double)*18);
	memcpy(Z2, &Z2[1], sizeof(double)*17);
	Z1[18] = d1;
	Z2[17] = d2;

	double a;
	a = Z1[0] * H[36];
	a += Z2[0] * H[35];
	a += Z1[1] * H[34];
	a += Z2[1] * H[33];
	a += Z1[2] * H[32];
	a += Z2[2] * H[31];
	a += Z1[3] * H[30];
	a += Z2[3] * H[29];
	a += Z1[4] * H[28];
	a += Z2[4] * H[27];
	a += Z1[5] * H[26];
	a += Z2[5] * H[25];
	a += Z1[6] * H[24];
	a += Z2[6] * H[23];
	a += Z1[7] * H[22];
	a += Z2[7] * H[21];
	a += Z1[8] * H[20];
	a += Z2[8] * H[19];
	a += Z1[9] * H[18];
	a += Z2[9] * H[17];
	a += Z1[10] * H[16];
	a += Z2[10] * H[15];
	a += Z1[11] * H[14];
	a += Z2[11] * H[13];
	a += Z1[12] * H[12];
	a += Z2[12] * H[11];
	a += Z1[13] * H[10];
	a += Z2[13] * H[9];
	a += Z1[14] * H[8];
	a += Z2[14] * H[7];
	a += Z1[15] * H[6];
	a += Z2[15] * H[5];
	a += Z1[16] * H[4];
	a += Z2[16] * H[3];
	a += Z1[17] * H[2];
	a += Z2[17] * H[1];
	a += Z1[18] * H[0];
	return a;
}

#if 0
//*********************************************************************
// CDECM2H	1/2デシメータ処理クラス 64tap
//
CDECM2H::CDECM2H()
{
	memset(Z1, 0, sizeof(Z1));
	memset(Z2, 0, sizeof(Z2));
	::MakeFilter(H, 64, ffLPF, SampFreq, 2900, 3000, 70, 1.0);
}

double CDECM2H::Do(double d1, double d2)
{
	memcpy(Z1, &Z1[1], sizeof(double)*32);
	memcpy(Z2, &Z2[1], sizeof(double)*31);
	Z1[32] = d1;
	Z2[31] = d2;

	double a;
	a = Z1[0] * H[64];
	a += Z2[0] * H[63];
	a += Z1[1] * H[62];
	a += Z2[1] * H[61];
	a += Z1[2] * H[60];
	a += Z2[2] * H[59];
	a += Z1[3] * H[58];
	a += Z2[3] * H[57];
	a += Z1[4] * H[56];
	a += Z2[4] * H[55];
	a += Z1[5] * H[54];
	a += Z2[5] * H[53];
	a += Z1[6] * H[52];
	a += Z2[6] * H[51];
	a += Z1[7] * H[50];
	a += Z2[7] * H[49];
	a += Z1[8] * H[48];
	a += Z2[8] * H[47];
	a += Z1[9] * H[46];
	a += Z2[9] * H[45];
	a += Z1[10] * H[44];
	a += Z2[10] * H[43];
	a += Z1[11] * H[42];
	a += Z2[11] * H[41];
	a += Z1[12] * H[40];
	a += Z2[12] * H[39];
	a += Z1[13] * H[38];
	a += Z2[13] * H[37];
	a += Z1[14] * H[36];
	a += Z2[14] * H[35];
	a += Z1[15] * H[34];
	a += Z2[15] * H[33];
	a += Z1[16] * H[32];
	a += Z2[16] * H[31];
	a += Z1[17] * H[30];
	a += Z2[17] * H[29];
	a += Z1[18] * H[28];
	a += Z2[18] * H[27];
	a += Z1[19] * H[26];
	a += Z2[19] * H[25];
	a += Z1[20] * H[24];
	a += Z2[20] * H[23];
	a += Z1[21] * H[22];
	a += Z2[21] * H[21];
	a += Z1[22] * H[20];
	a += Z2[22] * H[19];
	a += Z1[23] * H[18];
	a += Z2[23] * H[17];
	a += Z1[24] * H[16];
	a += Z2[24] * H[15];
	a += Z1[25] * H[14];
	a += Z2[25] * H[13];
	a += Z1[26] * H[12];
	a += Z2[26] * H[11];
	a += Z1[27] * H[10];
	a += Z2[27] * H[9];
	a += Z1[28] * H[8];
	a += Z2[28] * H[7];
	a += Z1[29] * H[6];
	a += Z2[29] * H[5];
	a += Z1[30] * H[4];
	a += Z2[30] * H[3];
	a += Z1[31] * H[2];
	a += Z2[31] * H[1];
	a += Z1[32] * H[0];
	return a;
}
#endif
#if 0
//*********************************************************************
// CINTP2	×2インタポーレータ処理クラス
//
CINTP2::CINTP2()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 36, ffLPF, SampFreq, 2900, 3000, 70, 2.0);
}

void CINTP2::Do(double &d1, double &d2, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*18);
	Z[18] = d;

	d1 = Z[0] * H[36];
	d2 = Z[1] * H[35];
	d1 += Z[1] * H[34];
	d2 += Z[2] * H[33];
	d1 += Z[2] * H[32];
	d2 += Z[3] * H[31];
	d1 += Z[3] * H[30];
	d2 += Z[4] * H[29];
	d1 += Z[4] * H[28];
	d2 += Z[5] * H[27];
	d1 += Z[5] * H[26];
	d2 += Z[6] * H[25];
	d1 += Z[6] * H[24];
	d2 += Z[7] * H[23];
	d1 += Z[7] * H[22];
	d2 += Z[8] * H[21];
	d1 += Z[8] * H[20];
	d2 += Z[9] * H[19];
	d1 += Z[9] * H[18];
	d2 += Z[10] * H[17];
	d1 += Z[10] * H[16];
	d2 += Z[11] * H[15];
	d1 += Z[11] * H[14];
	d2 += Z[12] * H[13];
	d1 += Z[12] * H[12];
	d2 += Z[13] * H[11];
	d1 += Z[13] * H[10];
	d2 += Z[14] * H[9];
	d1 += Z[14] * H[8];
	d2 += Z[15] * H[7];
	d1 += Z[15] * H[6];
	d2 += Z[16] * H[5];
	d1 += Z[16] * H[4];
	d2 += Z[17] * H[3];
	d1 += Z[17] * H[2];
	d2 += Z[18] * H[1];
	d1 += Z[18] * H[0];
}

//*********************************************************************
// CDECM3	1/3デシメータ処理クラス
//
CDECM3::CDECM3()
{
	memset(Z1, 0, sizeof(Z1));
	memset(Z2, 0, sizeof(Z2));
	memset(Z3, 0, sizeof(Z3));
	::MakeFilter(H, 48, ffLPF, SampFreq, 1300, 3000, 70, 1.0);
}

double CDECM3::Do(double d1, double d2, double d3)
{
	memcpy(Z1, &Z1[1], sizeof(double)*16);
	memcpy(Z2, &Z2[1], sizeof(double)*15);
	memcpy(Z3, &Z3[1], sizeof(double)*15);
	Z1[16] = d1;
	Z2[15] = d2;
	Z3[15] = d3;

	double a;
	a = Z1[0] * H[48];
	a += Z3[0] * H[47];
	a += Z2[0] * H[46];
	a += Z1[1] * H[45];
	a += Z3[1] * H[44];
	a += Z2[1] * H[43];
	a += Z1[2] * H[42];
	a += Z3[2] * H[41];
	a += Z2[2] * H[40];
	a += Z1[3] * H[39];
	a += Z3[3] * H[38];
	a += Z2[3] * H[37];
	a += Z1[4] * H[36];
	a += Z3[4] * H[35];
	a += Z2[4] * H[34];
	a += Z1[5] * H[33];
	a += Z3[5] * H[32];
	a += Z2[5] * H[31];
	a += Z1[6] * H[30];
	a += Z3[6] * H[29];
	a += Z2[6] * H[28];
	a += Z1[7] * H[27];
	a += Z3[7] * H[26];
	a += Z2[7] * H[25];
	a += Z1[8] * H[24];
	a += Z3[8] * H[23];
	a += Z2[8] * H[22];
	a += Z1[9] * H[21];
	a += Z3[9] * H[20];
	a += Z2[9] * H[19];
	a += Z1[10] * H[18];
	a += Z3[10] * H[17];
	a += Z2[10] * H[16];
	a += Z1[11] * H[15];
	a += Z3[11] * H[14];
	a += Z2[11] * H[13];
	a += Z1[12] * H[12];
	a += Z3[12] * H[11];
	a += Z2[12] * H[10];
	a += Z1[13] * H[9];
	a += Z3[13] * H[8];
	a += Z2[13] * H[7];
	a += Z1[14] * H[6];
	a += Z3[14] * H[5];
	a += Z2[14] * H[4];
	a += Z1[15] * H[3];
	a += Z3[15] * H[2];
	a += Z2[15] * H[1];
	a += Z1[16] * H[0];
	return a;
}

//*********************************************************************
// CINTP3	×3インタポーレータ処理クラス
//
CINTP3::CINTP3()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 48, ffLPF, SampFreq, 1300, 3000, 70, 3.0);
}

void CINTP3::Do(double &d1, double &d2, double &d3, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*16);
	Z[16] = d;

	d1 = Z[0] * H[48];
	d3 = Z[1] * H[47];
	d2 = Z[1] * H[46];
	d1 += Z[1] * H[45];
	d3 += Z[2] * H[44];
	d2 += Z[2] * H[43];
	d1 += Z[2] * H[42];
	d3 += Z[3] * H[41];
	d2 += Z[3] * H[40];
	d1 += Z[3] * H[39];
	d3 += Z[4] * H[38];
	d2 += Z[4] * H[37];
	d1 += Z[4] * H[36];
	d3 += Z[5] * H[35];
	d2 += Z[5] * H[34];
	d1 += Z[5] * H[33];
	d3 += Z[6] * H[32];
	d2 += Z[6] * H[31];
	d1 += Z[6] * H[30];
	d3 += Z[7] * H[29];
	d2 += Z[7] * H[28];
	d1 += Z[7] * H[27];
	d3 += Z[8] * H[26];
	d2 += Z[8] * H[25];
	d1 += Z[8] * H[24];
	d3 += Z[9] * H[23];
	d2 += Z[9] * H[22];
	d1 += Z[9] * H[21];
	d3 += Z[10] * H[20];
	d2 += Z[10] * H[19];
	d1 += Z[10] * H[18];
	d3 += Z[11] * H[17];
	d2 += Z[11] * H[16];
	d1 += Z[11] * H[15];
	d3 += Z[12] * H[14];
	d2 += Z[12] * H[13];
	d1 += Z[12] * H[12];
	d3 += Z[13] * H[11];
	d2 += Z[13] * H[10];
	d1 += Z[13] * H[9];
	d3 += Z[14] * H[8];
	d2 += Z[14] * H[7];
	d1 += Z[14] * H[6];
	d3 += Z[15] * H[5];
	d2 += Z[15] * H[4];
	d1 += Z[15] * H[3];
	d3 += Z[16] * H[2];
	d2 += Z[16] * H[1];
	d1 += Z[16] * H[0];
}
#endif


/**********************************************************************
	ＦＩＲフィルタ設計関数
**********************************************************************/
/*
====================================================
	ベッセル関数
====================================================
*/
static double I0(double x)
{
	double	sum, xj;
	int		j;

	sum = 1.0;
	xj = 1.0;
	j = 1;
	while(1){
		xj *= ((0.5 * x) / (double)j);
		sum += (xj*xj);
		j++;
		if( ((0.00000001 * sum) - (xj*xj)) > 0 ) break;
	}
	return(sum);
}

/*
====================================================
	ＦＩＲフィルタの設計
====================================================
*/
void MakeFilter(double *HP, int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	FIR	fir;

	fir.typ = type;
	fir.n = tap;
	fir.fs = fs;
	fir.fcl = fcl;
	fir.fch = fch;
	fir.att = att;
	fir.gain = gain;
	MakeFilter(HP, &fir);
}

void MakeFilter(double *HP, FIR *fp)
{
	int		j, m;
	double	alpha, win, fm, w0, sum;
	double	*hp;

	if( fp->typ == ffHPF ){
		fp->fc = 0.5*fp->fs - fp->fcl;
	}
	else if( fp->typ != ffLPF ){
		fp->fc = (fp->fch - fp->fcl)/2.0;
	}
	else {
		fp->fc = fp->fcl;
	}
	if( fp->att >= 50.0 ){
		alpha = 0.1102 * (fp->att - 8.7);
	}
	else if( fp->att >= 21 ){
		alpha = (0.5842 * pow(fp->att - 21.0, 0.4)) + (0.07886 * (fp->att - 21.0));
	}
	else {
		alpha = 0.0;
	}

	hp = fp->hp;
	sum = PI*2.0*fp->fc/fp->fs;
	if( fp->att >= 21 ){		// インパルス応答と窓関数を計算
		for( j = 0; j <= (fp->n/2); j++, hp++ ){
			fm = (double)(2 * j)/(double)fp->n;
			win = I0(alpha * sqrt(1.0-(fm*fm)))/I0(alpha);
			if( !j ){
				*hp = fp->fc * 2.0/fp->fs;
			}
			else {
				*hp = (1.0/(PI*(double)j))*sin((double)j*sum)*win;
			}
		}
	}
	else {						// インパルス応答のみ計算
		for( j = 0; j <= (fp->n/2); j++, hp++ ){
			if( !j ){
				*hp = fp->fc * 2.0/fp->fs;
			}
			else {
				*hp = (1.0/(PI*(double)j))*sin((double)j*sum);
			}
		}
	}
	hp = fp->hp;
	sum = *hp++;
	for( j = 1; j <= (fp->n/2); j++, hp++ ) sum += 2.0 * (*hp);
	hp = fp->hp;
	if( sum > 0.0 ){
		for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) /= sum;
	}

	// 周波数変換

	if( fp->typ == ffHPF ){
		hp = fp->hp;
		for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) *= cos((double)j*PI);
	}
	else if( fp->typ != ffLPF ){
		w0 = PI * (fp->fcl + fp->fch) / fp->fs;
		if( fp->typ == ffBPF ){
			hp = fp->hp;
			for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) *= 2.0*cos((double)j*w0);
		}
		else {
			hp = fp->hp;
			*hp = 1.0 - (2.0 * (*hp));
			for( hp++, j = 1; j <= (fp->n/2); j++, hp++ ) (*hp) *= -2.0*cos((double)j*w0);
		}
	}
	for( m = fp->n/2, hp = &fp->hp[m], j = m; j >= 0; j--, hp-- ){
		*HP++ = (*hp) * fp->gain;
	}
	for( hp = &fp->hp[1], j = 1; j <= (fp->n/2); j++, hp++ ){
		*HP++ = (*hp) * fp->gain;
	}
}

//---------------------------------------------------------------------------
//ＦＩＲフィルタ（ヒルベルト変換フィルタ）の設計
// Added by JE3HHT on Aug.2010
void __fastcall MakeHilbert(double *H, int N, double fs, double fc1, double fc2)
{
	int L = N / 2;
    double T = 1.0 / fs;

	double W1 = 2 * PI * fc1;
	double W2 = 2 * PI * fc2;

	// 2*fc2*T*SA((n-L)*W2*T) - 2*fc1*T*SA((n-L)*W1*T)

	double w;
	int n;
	double x1, x2;
	for( n = 0; n <= N; n++ ){
		if( n == L ){
			x1 = x2 = 0.0;
		}
		else if( (n - L) ){
			x1 = ((n - L) * W1 * T);
			x1 = cos(x1) / x1;
			x2 = ((n - L) * W2 * T);
			x2 = cos(x2) / x2;
		}
		else {
			x1 = x2 = 1.0;
		}
		w = 0.54 - 0.46 * cos(2*PI*n/(N));
		H[n] = -(2 * fc2 * T * x2 - 2 * fc1 * T * x1) * w;
	}

	if( N < 8 ){
		w = 0;
    	for( n = 0; n <= N; n++ ){
			w += fabs(H[n]);
    	}
		if( w ){
			w = 1.0 / w;
			for( n = 0; n <= N; n++ ){
				H[n] *= w;
			}
    	}
	}
}

//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//	H(ejωT) = [Σ0]Hm cos(mωT) - j[Σ1]Hm sin(mωT)
//
void DrawGraph(Graphics::TBitmap *pBitmap, const double *H, int Tap, int Over, int &nmax, int init, TColor col)
{
	int		k, x, y;
	double	f, gdb, g, pi2t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 7; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/8.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 5; i++ ){
			y = (int)(double(i) * double(YB - YT)/4.0 + YT);
			sprintf( bf, "-%2u", (80 / 4)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	double ra, im;
	pi2t = 2.0 * PI / fs;
	tp->Pen->Color = col;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		if( Tap ){
			ra = im = 0.0;
			for( k = 0; k <= Tap; k++ ){
				ra += H[k] * cos(pi2t*f*k);
				if( k ) im -= H[k] * sin(pi2t*f*k);
			}
			if( ra * im ){
				g = sqrt(ra * ra + im * im);
			}
			else {
				g = 0.0;
			}
		}
		else {
			g = 1.0;
		}
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 80.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/80.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

void DrawGraph2(Graphics::TBitmap *pBitmap, const double *H1, int Tap1, const double *H2, int Tap2, int Over, int &nmax, int init, TColor col)
{
	int		k, x, y;
	double	f, gdb, g, pi2t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 7; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/8.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 5; i++ ){
			y = (int)(double(i) * double(YB - YT)/4.0 + YT);
			sprintf( bf, "-%2u", (80 / 4)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	double ra, im;
	pi2t = 2.0 * PI / fs;
	tp->Pen->Color = col;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		if( Tap1 ){
			ra = im = 0.0;
			for( k = 0; k <= Tap1; k++ ){
				ra += H1[k] * cos(pi2t*f*k);
				if( k ) im -= H1[k] * sin(pi2t*f*k);
			}
			if( ra * im ){
				g = sqrt(ra * ra + im * im);
			}
			else {
				g = 0.0;
			}
		}
		else {
			g = 1.0;
		}
		if( Tap2 ){
			ra = im = 0.0;
			for( k = 0; k <= Tap2; k++ ){
				ra += H2[k] * cos(pi2t*f*k);
				if( k ) im -= H2[k] * sin(pi2t*f*k);
			}
			if( ra * im ){
				g *= sqrt(ra * ra + im * im);
			}
			else {
				g *= 0.0;
			}
		}
		else {
			g *= 1.0;
		}
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 80.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/80.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//
void DrawGraphIIR(Graphics::TBitmap *pBitmap, double a0, double a1, double a2, double b1, double b2, int Over, int &nmax, int init, TColor col)
{
	int		x, y;
	double	f, gdb, g, pi2t, pi4t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 5; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/6.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 4; i++ ){
			y = (int)(double(i) * double(YB - YT)/3.0 + YT);
			sprintf( bf, "-%2u", (60 / 3)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	pi2t = 2.0 * PI / fs;
	pi4t = 2.0 * pi2t;
	tp->Pen->Color = col;
	double	A, B, C, D, P, R;
	double	cosw, sinw, cos2w, sin2w;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		cosw = cos(pi2t*f);
		sinw = sin(pi2t*f);
		cos2w = cos(pi4t*f);
		sin2w = sin(pi4t*f);
		A = a0 + a1 * cosw + a2 * cos2w;
		B = 1 + b1 * cosw + b2 * cos2w;
		C = a1 * sinw + a2 * sin2w;
		D = b1 * sinw + b2 * sin2w;
		P = A*A + C*C;
		R = B*B + D*D;
		g = sqrt(P/R);
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 60.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/60.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//
void DrawGraphIIR(Graphics::TBitmap *pBitmap, CIIR *ip, int Over, int &nmax, int init, TColor col)
{
	int		x, y;
	double	f, gdb, g, pi2t, pi4t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 5; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/6.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 4; i++ ){
			y = (int)(double(i) * double(YB - YT)/3.0 + YT);
			sprintf( bf, "-%2u", (60 / 3)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	pi2t = 2.0 * PI / fs;
	pi4t = 2.0 * pi2t;
	tp->Pen->Color = col;
	double	A, B, C, D, P, R;
	double	cosw, sinw, cos2w, sin2w;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		cosw = cos(pi2t*f);
		sinw = sin(pi2t*f);
		cos2w = cos(pi4t*f);
		sin2w = sin(pi4t*f);
		g = 1.0;
		double *ap = ip->A;
		double *bp = ip->B;
		for( i = 0; i < ip->m_order/2; i++, ap += 3, bp += 2 ){
/*
		A = a0 + a1 * cosw + a2 * cos2w;
		B = 1 + b1 * cosw + b2 * cos2w;
		C = a1 * sinw + a2 * sin2w;
		D = b1 * sinw + b2 * sin2w;
*/
			A = bp[0] + bp[1] * cosw + bp[0] * cos2w;
			B = 1 + -ap[1] * cosw + -ap[2] * cos2w;
			C = bp[1] * sinw + bp[0] * sin2w;
			D = -ap[1] * sinw + -ap[2] * sin2w;
			P = A*A + C*C;
			R = B*B + D*D;
			g *= sqrt(P/R);
		}
		if( ip->m_order & 1 ){
			A = bp[0] + bp[1] * cosw;
			B = 1 + -ap[1] * cosw;
			C = bp[1] * sinw;
			D = -ap[1] * sinw;
			P = A*A + C*C;
			R = B*B + D*D;
			g *= sqrt(P/R);
		}
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 60.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/60.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

double asinh(double x)
{
	return log(x + sqrt(x*x+1.0));
}

//------------------------------------------------------------------
// bc : 0-バターワース, 1-チェビシフ
// rp : 通過域のリップル
void MakeIIR(double *A, double *B, double fc, double fs, int order, int bc, double rp)
{
	double	w0, wa, u, zt, x;
	int		j, n;

	if( bc ){		// チェビシフ
		u = 1.0/double(order)*asinh(1.0/sqrt(pow(10.0,0.1*rp)-1.0));
	}
	wa = tan(PI*fc/fs);
	w0 = 1.0;
	n = (order & 1) + 1;
	double *pA = A;
	double *pB = B;
	double d1, d2;
	for( j = 1; j <= order/2; j++, pA+=3, pB+=2 ){
		if( bc ){	// チェビシフ
			d1 = sinh(u)*cos(n*PI/(2*order));
			d2 = cosh(u)*sin(n*PI/(2*order));
			w0 = sqrt(d1 * d1 + d2 * d2);
			zt = sinh(u)*cos(n*PI/(2*order))/w0;
		}
		else {		// バターワース
			w0 = 1.0;
			zt = cos(n*PI/(2*order));
		}
		pA[0] = 1 + wa*w0*2*zt + wa*w0*wa*w0;
		pA[1] = -2 * (wa*w0*wa*w0 - 1)/pA[0];
		pA[2] = -(1.0 - wa*w0*2*zt + wa*w0*wa*w0)/pA[0];
		pB[0] = wa*w0*wa*w0 / pA[0];
		pB[1] = 2*pB[0];
		n += 2;
	}
	if( bc && !(order & 1) ){
		x = pow( 1.0/pow(10.0,rp/20.0), 1/double(order/2) );
		pB = B;
		for( j = 1; j <= order/2; j++, pB+=2 ){
			pB[0] *= x;
			pB[1] *= x;
		}
	}
	if( order & 1 ){
		if( bc ) w0 = sinh(u);
		j = (order / 2);
		pA = A + (j*3);
		pB = B + (j*2);
		pA[0] = 1 + wa*w0;
		pA[1] = -(wa*w0 - 1)/pA[0];
		pB[0] = wa*w0/pA[0];
		pB[1] = pB[0];
	}
}

//---------------------------------------------------------------------------
CIIR::CIIR()
{
	m_order = 0;
	A = new double[IIRMAX*3];
	B = new double[IIRMAX*2];
	Z = new double[IIRMAX*2];
	memset(A, 0, sizeof(double[IIRMAX*3]));
	memset(B, 0, sizeof(double[IIRMAX*2]));
	memset(Z, 0, sizeof(double[IIRMAX*2]));
}

CIIR::~CIIR()
{
	if( A != NULL ) delete[] A;
	if( B != NULL ) delete[] B;
	if( Z != NULL ) delete[] Z;
}

void CIIR::Clear(void)
{
	memset(Z, 0, sizeof(double[IIRMAX*2]));
}

void CIIR::MakeIIR(double fc, double fs, int order, int bc, double rp)
{
	m_order = order;
	m_bc = bc;
	m_rp = rp;
	::MakeIIR(A, B, fc, fs, order, bc, rp);
}

double CIIR::Do(double d)
{
	double *pA = A;
	double *pB = B;
	double *pZ = Z;
	double o;
	for( int i = 0; i < m_order/2; i++, pA+=3, pB+=2, pZ+=2 ){
		d += pZ[0] * pA[1] + pZ[1] * pA[2];
		o = d * pB[0] + pZ[0] * pB[1] + pZ[1] * pB[0];
		pZ[1] = pZ[0];
		if( fabs(d) < 1e-37 ) d = 0.0;
		pZ[0] = d;
		d = o;
	}
	if( m_order & 1 ){
		d += pZ[0] * pA[1];
		o = d * pB[0] + pZ[0] * pB[0];
		if( fabs(d) < 1e-37 ) d = 0.0;
		pZ[0] = d;
		d = o;
	}
	return d;
}

//*********************************************************************
// CINTPXY	XYScope 用 ×2インタポーレータ処理クラス
//
CINTPXY2::CINTPXY2()
{
	iir.MakeIIR(2800, DemSamp * 2.0, 14, 0, 0);
}

void __fastcall CINTPXY2::Do(double *p, double d)
{
	*p++ = iir.Do(d);
	*p = iir.Do(d);
}

#if 0
//*********************************************************************
// CINTPXY	XYScope 用 ×4インタポーレータ処理クラス
//
CINTPXY4::CINTPXY4()
{
	iir.MakeIIR(2800, DemSamp * 4.0, 24, 0, 0);
}

void __fastcall CINTPXY4::Do(double *p, double d)
{
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p = iir.Do(d);
}

//*********************************************************************
// CINTPXY	XYScope 用 ×8インタポーレータ処理クラス
//
CINTPXY8::CINTPXY8()
{
	iir.MakeIIR(2800, DemSamp * 8.0, 30, 0, 0);
}

void __fastcall CINTPXY8::Do(double *p, double d)
{
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p++ = iir.Do(d);
	*p = iir.Do(d);
}

//*********************************************************************
// CINTPXYFIR	XYScope 用 ×2インタポーレータ処理クラス
//
CINTPXY2FIR::CINTPXY2FIR()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 32, ffLPF, DemSamp * 2.0, 2800, 3000, 40, 1.7);
}

void CINTPXY2FIR::Do(double *dp, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*16);
	Z[16] = d;

	dp[0] = Z[0] * H[32];
	dp[1] = Z[1] * H[31];
	dp[0] += Z[1] * H[30];
	dp[1] += Z[2] * H[29];
	dp[0] += Z[2] * H[28];
	dp[1] += Z[3] * H[27];
	dp[0] += Z[3] * H[26];
	dp[1] += Z[4] * H[25];
	dp[0] += Z[4] * H[24];
	dp[1] += Z[5] * H[23];
	dp[0] += Z[5] * H[22];
	dp[1] += Z[6] * H[21];
	dp[0] += Z[6] * H[20];
	dp[1] += Z[7] * H[19];
	dp[0] += Z[7] * H[18];
	dp[1] += Z[8] * H[17];
	dp[0] += Z[8] * H[16];
	dp[1] += Z[9] * H[15];
	dp[0] += Z[9] * H[14];
	dp[1] += Z[10] * H[13];
	dp[0] += Z[10] * H[12];
	dp[1] += Z[11] * H[11];
	dp[0] += Z[11] * H[10];
	dp[1] += Z[12] * H[9];
	dp[0] += Z[12] * H[8];
	dp[1] += Z[13] * H[7];
	dp[0] += Z[13] * H[6];
	dp[1] += Z[14] * H[5];
	dp[0] += Z[14] * H[4];
	dp[1] += Z[15] * H[3];
	dp[0] += Z[15] * H[2];
	dp[1] += Z[16] * H[1];
	dp[0] += Z[16] * H[0];
}
#endif

//*********************************************************************
// CINTPXYFIR	XYScope 用 ×4インタポーレータ処理クラス
//
CINTPXY4FIR::CINTPXY4FIR()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 64, ffLPF, DemSamp * 4.0, 2800, 3000, 60, 4.0);
}

void __fastcall CINTPXY4FIR::Do(double *dp, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*16);
	Z[16] = d;

	dp[0] = Z[0] * H[64];
	dp[1] = Z[1] * H[63];
	dp[2] = Z[1] * H[62];
	dp[3] = Z[1] * H[61];
	dp[0] += Z[1] * H[60];
	dp[1] += Z[2] * H[59];
	dp[2] += Z[2] * H[58];
	dp[3] += Z[2] * H[57];
	dp[0] += Z[2] * H[56];
	dp[1] += Z[3] * H[55];
	dp[2] += Z[3] * H[54];
	dp[3] += Z[3] * H[53];
	dp[0] += Z[3] * H[52];
	dp[1] += Z[4] * H[51];
	dp[2] += Z[4] * H[50];
	dp[3] += Z[4] * H[49];
	dp[0] += Z[4] * H[48];
	dp[1] += Z[5] * H[47];
	dp[2] += Z[5] * H[46];
	dp[3] += Z[5] * H[45];
	dp[0] += Z[5] * H[44];
	dp[1] += Z[6] * H[43];
	dp[2] += Z[6] * H[42];
	dp[3] += Z[6] * H[41];
	dp[0] += Z[6] * H[40];
	dp[1] += Z[7] * H[39];
	dp[2] += Z[7] * H[38];
	dp[3] += Z[7] * H[37];
	dp[0] += Z[7] * H[36];
	dp[1] += Z[8] * H[35];
	dp[2] += Z[8] * H[34];
	dp[3] += Z[8] * H[33];
	dp[0] += Z[8] * H[32];
	dp[1] += Z[9] * H[31];
	dp[2] += Z[9] * H[30];
	dp[3] += Z[9] * H[29];
	dp[0] += Z[9] * H[28];
	dp[1] += Z[10] * H[27];
	dp[2] += Z[10] * H[26];
	dp[3] += Z[10] * H[25];
	dp[0] += Z[10] * H[24];
	dp[1] += Z[11] * H[23];
	dp[2] += Z[11] * H[22];
	dp[3] += Z[11] * H[21];
	dp[0] += Z[11] * H[20];
	dp[1] += Z[12] * H[19];
	dp[2] += Z[12] * H[18];
	dp[3] += Z[12] * H[17];
	dp[0] += Z[12] * H[16];
	dp[1] += Z[12] * H[15];
	dp[2] += Z[12] * H[14];
	dp[3] += Z[12] * H[13];
	dp[0] += Z[12] * H[12];
	dp[1] += Z[12] * H[11];
	dp[2] += Z[12] * H[10];
	dp[3] += Z[12] * H[9];
	dp[0] += Z[12] * H[8];
	dp[1] += Z[12] * H[7];
	dp[2] += Z[12] * H[6];
	dp[3] += Z[12] * H[5];
	dp[0] += Z[12] * H[4];
	dp[1] += Z[12] * H[3];
	dp[2] += Z[12] * H[2];
	dp[3] += Z[12] * H[1];
	dp[0] += Z[12] * H[0];
}

//*********************************************************************
// CINTPXYFIR	XYScope 用 ×8インタポーレータ処理クラス
//
CINTPXY8FIR::CINTPXY8FIR()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 96, ffLPF, DemSamp * 8.0, 2800, 3000, 60, 8.0);
}

void __fastcall CINTPXY8FIR::Do(double *dp, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*12);
	Z[12] = d;

	dp[0] = Z[0] * H[96];
	dp[1] = Z[1] * H[95];
	dp[2] = Z[1] * H[94];
	dp[3] = Z[1] * H[93];
	dp[4] = Z[1] * H[92];
	dp[5] = Z[1] * H[91];
	dp[6] = Z[1] * H[90];
	dp[7] = Z[1] * H[89];

	dp[0] += Z[1] * H[88];
	dp[1] += Z[2] * H[87];
	dp[2] += Z[2] * H[86];
	dp[3] += Z[2] * H[85];
	dp[4] += Z[2] * H[84];
	dp[5] += Z[2] * H[83];
	dp[6] += Z[2] * H[82];
	dp[7] += Z[2] * H[81];

	dp[0] += Z[2] * H[80];
	dp[1] += Z[3] * H[79];
	dp[2] += Z[3] * H[78];
	dp[3] += Z[3] * H[77];
	dp[4] += Z[3] * H[76];
	dp[5] += Z[3] * H[75];
	dp[6] += Z[3] * H[74];
	dp[7] += Z[3] * H[73];

	dp[0] += Z[3] * H[72];
	dp[1] += Z[4] * H[71];
	dp[2] += Z[4] * H[70];
	dp[3] += Z[4] * H[69];
	dp[4] += Z[4] * H[68];
	dp[5] += Z[4] * H[67];
	dp[6] += Z[4] * H[66];
	dp[7] += Z[4] * H[65];

	dp[0] += Z[4] * H[64];
	dp[1] += Z[5] * H[63];
	dp[2] += Z[5] * H[62];
	dp[3] += Z[5] * H[61];
	dp[4] += Z[5] * H[60];
	dp[5] += Z[5] * H[59];
	dp[6] += Z[5] * H[58];
	dp[7] += Z[5] * H[57];

	dp[0] += Z[5] * H[56];
	dp[1] += Z[6] * H[55];
	dp[2] += Z[6] * H[54];
	dp[3] += Z[6] * H[53];
	dp[4] += Z[6] * H[52];
	dp[5] += Z[6] * H[51];
	dp[6] += Z[6] * H[50];
	dp[7] += Z[6] * H[49];

	dp[0] += Z[6] * H[48];
	dp[1] += Z[7] * H[47];
	dp[2] += Z[7] * H[46];
	dp[3] += Z[7] * H[45];
	dp[4] += Z[7] * H[44];
	dp[5] += Z[7] * H[43];
	dp[6] += Z[7] * H[42];
	dp[7] += Z[7] * H[41];

	dp[0] += Z[7] * H[40];
	dp[1] += Z[8] * H[39];
	dp[2] += Z[8] * H[38];
	dp[3] += Z[8] * H[37];
	dp[4] += Z[8] * H[36];
	dp[5] += Z[8] * H[35];
	dp[6] += Z[8] * H[34];
	dp[7] += Z[8] * H[33];

	dp[0] += Z[8] * H[32];
	dp[1] += Z[9] * H[31];
	dp[2] += Z[9] * H[30];
	dp[3] += Z[9] * H[29];
	dp[4] += Z[9] * H[28];
	dp[5] += Z[9] * H[27];
	dp[6] += Z[9] * H[26];
	dp[7] += Z[9] * H[25];

	dp[0] += Z[9] * H[24];
	dp[1] += Z[10] * H[23];
	dp[2] += Z[10] * H[22];
	dp[3] += Z[10] * H[21];
	dp[4] += Z[10] * H[20];
	dp[5] += Z[10] * H[19];
	dp[6] += Z[10] * H[18];
	dp[7] += Z[10] * H[17];

	dp[0] += Z[10] * H[16];
	dp[1] += Z[11] * H[15];
	dp[2] += Z[11] * H[14];
	dp[3] += Z[11] * H[13];
	dp[4] += Z[11] * H[12];
	dp[5] += Z[11] * H[11];
	dp[6] += Z[11] * H[10];
	dp[7] += Z[11] * H[9];

	dp[0] += Z[11] * H[8];
	dp[1] += Z[12] * H[7];
	dp[2] += Z[12] * H[6];
	dp[3] += Z[12] * H[5];
	dp[4] += Z[12] * H[4];
	dp[5] += Z[12] * H[3];
	dp[6] += Z[12] * H[2];
	dp[7] += Z[12] * H[1];

	dp[0] += Z[12] * H[0];
}

#if OVERFIR
//*********************************************************************
// CDECM4	1/4デシメータ処理クラス
//
CDECM4::CDECM4()
{
	const double _mt[]={1.11, 1.25, 1.40, 1.11};
	memset(Z1, 0, sizeof(Z1));
	memset(Z2, 0, sizeof(Z2));
	memset(Z3, 0, sizeof(Z3));
	memset(Z4, 0, sizeof(Z4));
	::MakeFilter(H, 80, ffLPF, SampFreq*4, 2800, 3000, 50, _mt[SampType]);
}

double __fastcall CDECM4::Do(double *dp)
{
	memcpy(Z1, &Z1[1], sizeof(double)*20);
	memcpy(Z2, &Z2[1], sizeof(double)*19);
	memcpy(Z3, &Z3[1], sizeof(double)*19);
	memcpy(Z4, &Z4[1], sizeof(double)*19);
#if 1
	Z4[19] = *dp++;
	Z3[19] = *dp++;
	Z2[19] = *dp++;
	Z1[20] = *dp;
#else
	Z1[20] = *dp++;
	Z2[19] = *dp++;
	Z3[19] = *dp++;
	Z4[19] = *dp;
#endif
	double a;
	a = Z1[0] * H[80];
	a += Z4[0] * H[79];
	a += Z3[0] * H[78];
	a += Z2[0] * H[77];
	a += Z1[1] * H[76];
	a += Z4[1] * H[75];
	a += Z3[1] * H[74];
	a += Z2[1] * H[73];
	a += Z1[2] * H[72];
	a += Z4[2] * H[71];
	a += Z3[2] * H[70];
	a += Z2[2] * H[69];
	a += Z1[3] * H[68];
	a += Z4[3] * H[67];
	a += Z3[3] * H[66];
	a += Z2[3] * H[65];
	a += Z1[4] * H[64];
	a += Z4[4] * H[63];
	a += Z3[4] * H[62];
	a += Z2[4] * H[61];
	a += Z1[5] * H[60];
	a += Z4[5] * H[59];
	a += Z3[5] * H[58];
	a += Z2[5] * H[57];
	a += Z1[6] * H[56];
	a += Z4[6] * H[55];
	a += Z3[6] * H[54];
	a += Z2[6] * H[53];
	a += Z1[7] * H[52];
	a += Z4[7] * H[51];
	a += Z3[7] * H[50];
	a += Z2[7] * H[49];
	a += Z1[8] * H[48];
	a += Z4[8] * H[47];
	a += Z3[8] * H[46];
	a += Z2[8] * H[45];
	a += Z1[9] * H[44];
	a += Z4[9] * H[43];
	a += Z3[9] * H[42];
	a += Z2[9] * H[41];
	a += Z1[10] * H[40];
	a += Z4[10] * H[39];
	a += Z3[10] * H[38];
	a += Z2[10] * H[37];
	a += Z1[11] * H[36];
	a += Z4[11] * H[35];
	a += Z3[11] * H[34];
	a += Z2[11] * H[33];
	a += Z1[12] * H[32];
	a += Z4[12] * H[31];
	a += Z3[12] * H[30];
	a += Z2[12] * H[29];
	a += Z1[13] * H[28];
	a += Z4[13] * H[27];
	a += Z3[13] * H[26];
	a += Z2[13] * H[25];
	a += Z1[14] * H[24];
	a += Z4[14] * H[23];
	a += Z3[14] * H[22];
	a += Z2[14] * H[21];
	a += Z1[15] * H[20];
	a += Z4[15] * H[19];
	a += Z3[15] * H[18];
	a += Z2[15] * H[17];
	a += Z1[16] * H[16];
	a += Z4[16] * H[15];
	a += Z3[16] * H[14];
	a += Z2[16] * H[13];
	a += Z1[17] * H[12];
	a += Z4[17] * H[11];
	a += Z3[17] * H[10];
	a += Z2[17] * H[9];
	a += Z1[18] * H[8];
	a += Z4[18] * H[7];
	a += Z3[18] * H[6];
	a += Z2[18] * H[5];
	a += Z1[19] * H[4];
	a += Z4[19] * H[3];
	a += Z3[19] * H[2];
	a += Z2[19] * H[1];
	a += Z1[20] * H[0];
	return a;
}

//*********************************************************************
// CINTP4	x4インタポーレータ処理クラス
//
CINTP4::CINTP4()
{
	memset(Z, 0, sizeof(Z));
	::MakeFilter(H, 80, ffLPF, SampFreq*4, 2800, 3000, 50, 4.0);
}

void __fastcall CINTP4::Do(double *dp, double d)
{
	memcpy(Z, &Z[1], sizeof(double)*20);
	Z[20] = d;

	dp[0] = Z[0] * H[80];
	dp[1] = Z[1] * H[79];
	dp[2] = Z[1] * H[78];
	dp[3] = Z[1] * H[77];
	dp[0] += Z[1] * H[76];
	dp[1] += Z[2] * H[75];
	dp[2] += Z[2] * H[74];
	dp[3] += Z[2] * H[73];
	dp[0] += Z[2] * H[72];
	dp[1] += Z[3] * H[71];
	dp[2] += Z[3] * H[70];
	dp[3] += Z[3] * H[69];
	dp[0] += Z[3] * H[68];
	dp[1] += Z[4] * H[67];
	dp[2] += Z[4] * H[66];
	dp[3] += Z[4] * H[65];
	dp[0] += Z[4] * H[64];
	dp[1] += Z[5] * H[63];
	dp[2] += Z[5] * H[62];
	dp[3] += Z[5] * H[61];
	dp[0] += Z[5] * H[60];
	dp[1] += Z[6] * H[59];
	dp[2] += Z[6] * H[58];
	dp[3] += Z[6] * H[57];
	dp[0] += Z[6] * H[56];
	dp[1] += Z[7] * H[55];
	dp[2] += Z[7] * H[54];
	dp[3] += Z[7] * H[53];
	dp[0] += Z[7] * H[52];
	dp[1] += Z[8] * H[51];
	dp[2] += Z[8] * H[50];
	dp[3] += Z[8] * H[49];
	dp[0] += Z[8] * H[48];
	dp[1] += Z[9] * H[47];
	dp[2] += Z[9] * H[46];
	dp[3] += Z[9] * H[45];
	dp[0] += Z[9] * H[44];
	dp[1] += Z[10] * H[43];
	dp[2] += Z[10] * H[42];
	dp[3] += Z[10] * H[41];
	dp[0] += Z[10] * H[40];
	dp[1] += Z[11] * H[39];
	dp[2] += Z[11] * H[38];
	dp[3] += Z[11] * H[37];
	dp[0] += Z[11] * H[36];
	dp[1] += Z[12] * H[35];
	dp[2] += Z[12] * H[34];
	dp[3] += Z[12] * H[33];
	dp[0] += Z[12] * H[32];

	dp[1] += Z[13] * H[31];
	dp[2] += Z[13] * H[30];
	dp[3] += Z[13] * H[29];
	dp[0] += Z[13] * H[28];
	dp[1] += Z[14] * H[27];
	dp[2] += Z[14] * H[26];
	dp[3] += Z[14] * H[25];
	dp[0] += Z[14] * H[24];
	dp[1] += Z[15] * H[23];
	dp[2] += Z[15] * H[22];
	dp[3] += Z[15] * H[21];
	dp[0] += Z[15] * H[20];
	dp[1] += Z[16] * H[19];
	dp[2] += Z[16] * H[18];
	dp[3] += Z[16] * H[17];
	dp[0] += Z[16] * H[16];
	dp[1] += Z[17] * H[15];
	dp[2] += Z[17] * H[14];
	dp[3] += Z[17] * H[13];
	dp[0] += Z[17] * H[12];
	dp[1] += Z[18] * H[11];
	dp[2] += Z[18] * H[10];
	dp[3] += Z[18] * H[9];
	dp[0] += Z[18] * H[8];
	dp[1] += Z[19] * H[7];
	dp[2] += Z[19] * H[6];
	dp[3] += Z[19] * H[5];
	dp[0] += Z[19] * H[4];
	dp[1] += Z[20] * H[3];
	dp[2] += Z[20] * H[2];
	dp[3] += Z[20] * H[1];
	dp[0] += Z[20] * H[0];
}
#endif

//*********************************************************************
// COVERLIMIT	高品質リミッタ
//
COVERLIMIT::COVERLIMIT()
{
#if !OVERFIR
	iira.MakeIIR(2800, SampFreq * 4.0, 24, 0, 0);
	iirb.MakeIIR(2800, SampFreq * 4.0, 24, 0, 0);
#endif
}

double COVERLIMIT::Do(double d, double lmt)
{
#if OVERFIR
	double dt[4];

	intp.Do(dt, d);

	dt[0] *= lmt;
	if( dt[0] > 16384.0 ) dt[0] = 16384.0;
	if( dt[0] < -16384.0 ) dt[0] = -16384.0;
	dt[1] *= lmt;
	if( dt[1] > 16384.0 ) dt[1] = 16384.0;
	if( dt[1] < -16384.0 ) dt[1] = -16384.0;
	dt[2] *= lmt;
	if( dt[2] > 16384.0 ) dt[2] = 16384.0;
	if( dt[2] < -16384.0 ) dt[2] = -16384.0;
	dt[3] *= lmt;
	if( dt[3] > 16384.0 ) dt[3] = 16384.0;
	if( dt[3] < -16384.0 ) dt[3] = -16384.0;

	return decm.Do(dt);
#else
	double d1, d2, d3, d4;

	// X4 インターポーレータ
	d1 = iira.Do(d);
	d2 = iira.Do(d);
	d3 = iira.Do(d);
	d4 = iira.Do(d);

	d1 *= lmt;
	if( d1 > 16384.0 ) d1 = 16384.0;
	if( d1 < -16384.0 ) d1 = -16384.0;
	d2 *= lmt;
	if( d2 > 16384.0 ) d2 = 16384.0;
	if( d2 < -16384.0 ) d2 = -16384.0;
	d3 *= lmt;
	if( d3 > 16384.0 ) d3 = 16384.0;
	if( d3 < -16384.0 ) d3 = -16384.0;
	d4 *= lmt;
	if( d4 > 16384.0 ) d4 = 16384.0;
	if( d4 < -16384.0 ) d4 = -16384.0;

	// 1/4 デシメータ
	iirb.Do(d1);
	iirb.Do(d2);
	iirb.Do(d3);
	return iirb.Do(d4);
#endif
}

//*********************************************************************
// CDECM2IIR	1/2デシメータ処理クラス
//
CDECM2IIR::CDECM2IIR()
{
	iir.MakeIIR(2900, SampFreq, 12, 0, 0);
}

double CDECM2IIR::Do(double d1, double d2)
{
	iir.Do(d2);
	return iir.Do(d1);
}

#if 0
//*********************************************************************
// CHILL ヒルベルト変換クラス
//
CHILL::CHILL()
{
	memset(Z, 0, sizeof(Z));
	m_tap = 8;
	m_htap = m_tap / 2;
	m_CenterFreq = 2125 + 170*0.5;
	SetSampFreq(SampFreq);
	SetFreq(2125, 2125+170);
}

void CHILL::SetSampFreq(double samp)
{
	m_SampFreq = samp;
	m_OUT = 32768.0 * m_SampFreq / (2 * PI * 800);
	m_OFF = (2 * PI * m_CenterFreq) / m_SampFreq;
}

void CHILL::SetFreq(double mark, double space)
{
	m_MarkFreq = mark;
	m_SpaceFreq = space;
	m_CenterFreq = (mark + space) * 0.5;
	m_OFF = (2 * PI * m_CenterFreq) / m_SampFreq;

	MakeFilter(H, m_tap, ffLPF, m_SampFreq, m_CenterFreq, m_CenterFreq, 6, 1.0);
	H[m_htap] = 0;
	for( int i = 0; i < m_htap; i++ ){
		H[i] = -H[i];
	}
	m_A[0] = m_A[1] = m_A[2] = m_A[3] = 0;
	m_ph = &Z[m_htap];
}

void CHILL::SetMarkFreq(double mark)
{
	SetFreq(mark, m_SpaceFreq);
}

void CHILL::SetSpaceFreq(double space)
{
	SetFreq(m_MarkFreq, space);
}

double CHILL::Do(double d)
{
	d = DoFIR(H, Z, d, m_tap);
	double a = *m_ph;
	if( a ) a = atan2(d, a);
	d = a - m_A[0];
	m_A[0] = a;
	if( d >= PI ){
		d = d - PI*2;
	}
	else if( d <= -PI ){
		d = d + PI*2;
	}
	d += m_OFF;
	return d * m_OUT;
}
#endif


