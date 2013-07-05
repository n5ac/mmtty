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



#ifndef FFT_H
#define FFT_H
//---------------------------------------------------------------------------
#define FFT_BUFSIZE 2048
#define SQRT_FFT_SIZE 46//sqrt(2048)

class CFFT
{
public:
	CFFT();
	virtual ~CFFT();
	void	InitFFT(void);

	int		m_CollectFFT;
	void	CollectFFT(double *lp, int size);
	void	CalcFFT(int size, double gain, int stg);
	void	TrigFFT(void);
	void	ClearBuf(void){
		memset(m_fft, 0, sizeof(m_fft));
		memset(pStgBuf, 0, sizeof(double[FFT_BUFSIZE]));
		TrigFFT();
	};
	int		m_FFTDIS;
	int		m_fft[2048];
private:
	void	Calc(double * InBuf, int size, double gain, int stg, int* OutBuf);
	BOOL	m_StgSW;
	int		m_StgSize;
	double	m_StgScale;
	double	m_StgK;

	int		m_CollectFlag;
	int		m_CollectFFTCount;
	double	*m_CollectFFTBuf;

	double	*m_tSinCos;
	double	*m_tWindow;
	double	*pStgBuf;
	int		*m_Work;
	void makewt(int nw, int *ip, double *w);
	void makect(int nc, int *ip, double *c);
	void bitrv2(int n, int *ip, double *a);
	void cftfsub(int n, double *a, double *w);
	void rftfsub(int n, double *a, int nc, double *c);
	void cft1st(int n, double *a, double *w);
	void cftmdl(int n, int l, double *a, double *w);
};

#endif
