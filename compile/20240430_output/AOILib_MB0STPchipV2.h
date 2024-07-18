#pragma once

/*
Chip function:
single-phase chip: version==Uchip_singlephaseDownV2_1
dual-phase chip: version==Uchip_dualphase
*/

typedef struct
{
	int interval[4];
	int xpitch[3]; //in unit of pixel  xpitch[0]=250pixel
	int ypitch[3];
	int carx;
	int cary;

}SettingP;

typedef struct
{

	double TDwidth; //135
	double TDmaxW;  //1.25
	double TDminW;  //0.9
	double TDheight; //75
	double TDmaxH;   //1.5
	double TDminH;   //0.7

}sizeTD;


typedef struct 
{
	int thresmode; //0:gray ; 1:RGB ; 2:HSV
	int bgmax[3];   //[99999,99999,99999]
	int bgmin[3];   //[99999,99999,99999]
	int fgmax[3];   //[110,99999,99999]
	int fgmin[3];   //[0,99999,99999]
}thresP;

typedef struct
{
	int PICmode; //0
	int Outputmode; //0: center chip 
	int cols; //1500
	int rows; //1500    
	double correctTheta;
}ImgP;



__declspec(dllexport)  void STPchip_calcenter(thresP thresParm, ImgP imageParm, SettingP chipsetting, sizeTD target, unsigned char* imageIN,
						unsigned int* imageOUT, unsigned char* imageGray, float boolResult[], float outputLEDX[], float outputLEDY[]);


