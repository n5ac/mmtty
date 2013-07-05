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
#ifndef CLXH
#define CLXH
//---------------------------------------------------------------------------
///----------------------------------------------------------------
///	ï°ëfêîââéZÉNÉâÉX
class CLX {
public:
	double	r;
	double	j;
	inline CLX(void){};
	inline CLX(const double R){
		r = R;
		j = 0;
	};
	inline CLX(const double R, const double J){
		r = R;
		j = J;
	};
	inline CLX(CLX &ref){
		r = ref.r;
		j = ref.j;
	};
	inline void Add(const CLX &ref){
		r += ref.r;
		j += ref.j;
	};
	inline void Add(const double &R){
		r += R;
	};
	inline void Sub(const CLX &ref){
		r -= ref.r;
		j -= ref.j;
	};
	inline void Sub(const double &R){
		r -= R;
	};
    inline double Abs(void){
		return sqrt(r * r + j * j);
    };
    inline double vAbs(void){
		return r * r + j * j;
    };
	inline void Mul(CLX &z, const CLX &ref){
		z.r = r * ref.r - j * ref.j;
		z.j = r * ref.j + j * ref.r;
	}
	inline void Mul(const CLX &ref){
		CLX z;
		z.r = r * ref.r - j * ref.j;
		z.j = r * ref.j + j * ref.r;
		r = z.r; j = z.j;
    };
	void Div(const CLX &ref);
    inline void Mul(const double &R){
		r *= R; j *= R;
    }
    inline void Div(const double &R){
		r /= R; j /= R;
    }
    CLX Sqrt(void);
	inline void PhDiff(CLX &z, const CLX &ref){
		z.r = r * ref.r + j * ref.j;
		z.j = j * ref.r - r * ref.j;

    };
    inline double Phase(void){
		if( r && j ){
			return atan2(j, r);
        }
        else {
			return 0;
        }
    };
    inline BOOL IsSame(const CLX &ref){
		return ((r == ref.r) && (j == ref.j));
    };
	inline CLX& operator=(const CLX &ref){
		if( this != &ref ){
			r = ref.r;
			j = ref.j;
		}
		return *this;
	};
	inline CLX& operator+=(const CLX &ref){
		Add(ref);
		return *this;
	};
	inline CLX& operator+=(const double R){
		Add(R);
		return *this;
	};
	inline CLX& operator-=(const CLX &ref){
		Sub(ref);
		return *this;
	};
	inline CLX& operator-=(const double R){
		Sub(R);
		return *this;
	};
	inline CLX& operator*=(const CLX &ref){
		Mul(ref);
		return *this;
	};
	inline CLX& operator*=(const double R){
		Mul(R);
		return *this;
	};
	inline CLX& operator/=(const CLX &ref){
		Div(ref);
		return *this;
	};
	inline CLX& operator/=(const double R){
		Div(R);
		return *this;
	};
	inline CLX operator+(const CLX &ref) const {
		CLX a(r, j);
		a+=ref;
		return a;
	};
	inline CLX operator+(const double R) const {
		CLX a(r, j);
		a+=R;
		return a;
	};
	inline CLX operator-(const CLX &ref) const {
		CLX a(r, j);
		a-=ref;
		return a;
	};
	inline CLX operator-(const double R) const {
		CLX a(r, j);
		a-=R;
		return a;
	};
	inline CLX operator*(const CLX &ref) const {
		CLX a(r, j);
		a*=ref;
		return a;
	};
	inline CLX operator*(const double R) const {
		CLX a(r, j);
		a*=R;
		return a;
	};
	inline CLX operator/(const CLX &ref) const {
		CLX a(r, j);
		a/=ref;
		return a;
	};
	inline CLX operator/(const double R) const {
		CLX a(r, j);
		a/=R;
		return a;
	};
	friend CLX operator+(const double R, const CLX &ref);
	friend CLX operator-(const double R, const CLX &ref);
	friend CLX operator*(const double R, const CLX &ref);
	friend CLX operator/(const double R, const CLX &ref);
};
#endif
