/*
this is a port of Karsten Schmidt's sine and cos Look Up Table classes for java.
 
i changed a few things, for example i wasn't able o make these static so if you plan on using them
you need to create an instance.
everything else is pretty much the same.
these were created with the purpose of serving the ofxPerlin and ofxSimplex classes but might be 
useful in other situations as well.
*/

#pragma once
#include "cinder/CinderMath.h"
#ifndef TWO_PI
 #define TWO_PI	6.283185307179586
#endif

#ifndef TWO_PI
 #define TWO_PI	6.283185307179586f
#endif


#ifndef RAD_TO_DEG
 #define RAD_TO_DEG	57.295779513082321f
#endif

class ofxPerlin;

class ofxSinCosLUT{
protected:
	float SC_PRECISION;
	float SC_INV_PREC;
	int SC_PERIOD;
	float *sinLUT;
	float *cosLUT;
	float _lut_DEG_TO_RAD;
	float _lut_RAD_TO_DEG;
public:
	ofxSinCosLUT(){
		SC_PRECISION = 0.25f;
		SC_INV_PREC = (float)(1.0/SC_PRECISION);
		SC_PERIOD = (int)(360.0f * SC_INV_PREC);
		sinLUT = new float[SC_PERIOD];
		cosLUT = new float[SC_PERIOD];
		_lut_DEG_TO_RAD = (float)(M_PI / 180.0) * SC_PRECISION;
		_lut_RAD_TO_DEG = (float)(180.0 /M_PI) / SC_PRECISION;
		
		for(int i=0; i<SC_PERIOD; i++){
			sinLUT[i] = (float) sin(i * _lut_DEG_TO_RAD);
			cosLUT[i] = (float) cos(i * _lut_DEG_TO_RAD);
		}
	}
	~ofxSinCosLUT(){
		delete[] sinLUT;
		delete[] cosLUT;
	}
	
	float _sin(float theta){
		while(theta < 0){
			theta += TWO_PI;
		}
		return sinLUT[(int)(theta * RAD_TO_DEG) % SC_PERIOD];
	}
	
	float _cos(float theta){
		while(theta < 0){
			theta += TWO_PI;
		}
		return cosLUT[(int)(theta *RAD_TO_DEG) % SC_PERIOD];
	}
	
	friend class ofxPerlin;
	friend class ofxSimplex;
};
