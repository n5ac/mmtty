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

#include "math.h"
#include "CLX.h"

///----------------------------------------------------------------
///	•¡‘f”‰‰ŽZƒNƒ‰ƒX
void CLX::Div(const CLX &ref)
{
	double R, J, D;
	R = r * ref.r + j * ref.j;
	J = j * ref.r - r * ref.j;
	D = ref.r * ref.r + ref.j * ref.j;
    if( D ){
		r = R/D;
		j = J/D;
    }
    else {
		r = R >= 0 ? 1e100 : -1e100;
		j = J >= 0 ? 1e100 : -1e100;
    }
}

CLX CLX::Sqrt(void)
{
	CLX	a;

    double	R, ST;
    R = sqrt(sqrt(r*r + j*j));
	ST = 0;
    if( r ) ST = atan2(j, r)/2.0;
    a.r = R*cos(ST);
    a.j = R*sin(ST);
	return a;
}

CLX operator+(const double R, const CLX &ref)
{
	CLX a(R);
	a+=ref;
	return a;
}
CLX operator-(const double R, const CLX &ref)
{
	CLX a(R);
	a-=ref;
	return a;
}
CLX operator*(const double R, const CLX &ref)
{
	CLX a(R);
	a*=ref;
	return a;
}
CLX operator/(const double R, const CLX &ref)
{
	CLX a(R);
	a/=ref;
	return a;
}
#pragma package(smart_init)
