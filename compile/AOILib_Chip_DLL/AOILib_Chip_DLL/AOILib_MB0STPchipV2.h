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
	int thresmode; 
	int bgmax[3];   
	int bgmin[3];   
	int fgmax[3];   
	int fgmin[3];   
}thresP;

typedef struct
{
	int PICmode; 
	int Outputmode; 
	int cols; //1500
	int rows; //1500    
	double correctTheta;
}ImgP;



__declspec(dllexport)  void STPchip_calcenter(thresP thresParm, ImgP imageParm, SettingP chipsetting, sizeTD target, unsigned char* imageIN,
						unsigned int* imageOUT, unsigned char* imageGray, float boolResult[], float outputLEDX[], float outputLEDY[]);


