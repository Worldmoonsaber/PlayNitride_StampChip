#pragma region function_declare

#include "pch.h"
#include "AOILib_MB0STPchipV2.h"


#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include "../MBstampchip_V1/MBstp_libV1.h"


#pragma region Main
void STPchip_calcenter(thresP thresParm, ImgP imageParm, SettingP chipsetting, sizeTD target, unsigned char* imageIN,
					unsigned int* imageOUT, unsigned char* imageGray, float boolResult[], float outputLEDX[], float outputLEDY[])
{

	
	Mat rawimg, cropedRImg, gauBGR;
	Mat Gimg, drawF2;

	Point piccenter;
	Point2f creteriaPoint;
	Point IMGoffset=Point(0,0);

	
	

	//output parameters::
	Point crossCenter;
	double creteriaDist = double(chipsetting.xpitch[0]);
	float boolflag = 0;

	Mat image_input(imageParm.rows, imageParm.cols, CV_8UC1, &imageIN[0]); // THIS IS THE INPUT IMAGE, POINTER TO DATA			
	image_input.copyTo(rawimg);

	Mat image_output(imageParm.rows, imageParm.cols, CV_8UC4, &imageOUT[0]);
	Mat thres_output(imageParm.rows, imageParm.cols, CV_8UC1, &imageGray[0]);

	try
	{
		if (rawimg.empty())
		{
			boolflag = 8;
			throw "something wrong::input image failure";
		} //check if image is empty

	} //try loop
	catch (const char* message)
	{

		std::cout << "check catch state:: " << boolflag << endl;


	}//catch loop

	if (thresParm.thresmode == 0)
	{
		if (thresParm.fgmax[imageParm.PICmode] < 2 || thresParm.fgmax[imageParm.PICmode] < thresParm.fgmin[imageParm.PICmode]
			|| thresParm.fgmin[imageParm.PICmode] < 0)
		{
			boolflag = 1.0;
		}
	}
	else if (thresParm.thresmode == 3 || 4)
	{
		if (thresParm.bgmax[imageParm.PICmode] < thresParm.fgmax[imageParm.PICmode])
		{
			boolflag = 1.0;
		}
		else
		{
			if (thresParm.bgmax[imageParm.PICmode] & 1)
			{
				thresParm.bgmax[imageParm.PICmode] = thresParm.bgmax[imageParm.PICmode];
				thresParm.fgmax[imageParm.PICmode] = thresParm.fgmax[imageParm.PICmode];
			}
			else
			{
				thresParm.bgmax[imageParm.PICmode] = thresParm.bgmax[imageParm.PICmode] + 1;
				thresParm.fgmax[imageParm.PICmode] = thresParm.fgmax[imageParm.PICmode];
			}
		}
	}
	else
	{
		boolflag = 1.0;
	}

	

	if (boolflag == 0) //&& imageParm.Outputmode == 0
	{
		/*image with CROP  process :::*/
		//Point piccenter;
		//piccenter = find_piccenter(rawimg);
		////std::cout << "pic center is ::" << piccenter.x << " , " << piccenter.y << endl;	
		//IMGoffset.x = piccenter.x - int(imageParm.cols * 0.5);  //2736-600*0.5=2476
		//IMGoffset.y = piccenter.y - int(imageParm.rows * 0.5);  //1824-600*0.5=1564
		//Rect Cregion(IMGoffset.x, IMGoffset.y, imageParm.cols, imageParm.rows);
		//cropedRImg = CropIMG(rawimg, Cregion);

		///*///*image without CROP  process :::*/
		//sizeParm.CsizeW = rawimg.size[0];
		//sizeParm.CsizeH = sizeParm.CsizeW;
		rawimg.copyTo(cropedRImg);

		


		creteriaPoint = find_piccenter(cropedRImg);


		thresP_ _thresParm;
		

		_thresParm.thresmode = thresParm.thresmode;

		for (int i = 0; i < 3; i++)
		{
			_thresParm.bgmax[i] = thresParm.bgmax[i];
			_thresParm.bgmin[i] = thresParm.bgmin[i];
			_thresParm.fgmax[i] = thresParm.fgmax[i];
			_thresParm.fgmin[i] = thresParm.fgmin[i];
		}
		
		ImgP_ _imageParm;

		_imageParm.correctTheta= imageParm.correctTheta;
		_imageParm.imgcols= imageParm.cols;
		_imageParm.imgrows= imageParm.rows;
		_imageParm.Outputmode= imageParm.Outputmode;
		_imageParm.PICmode= imageParm.PICmode;

		SettingP_ _chipsetting;

		_chipsetting.carx = chipsetting.carx;
		_chipsetting.cary = chipsetting.cary;

		for (int i = 0; i < 4; i++)
			_chipsetting.interval[i] = chipsetting.interval[i];

		for (int i = 0; i < 3; i++)
		{
			_chipsetting.xpitch[i] = chipsetting.xpitch[i];
			_chipsetting.ypitch[i] = chipsetting.ypitch[i];
		}


		sizeTD_ _target;

		_target.TDheight=target.TDheight;
		_target.TDmaxH = target.TDmaxH;
		_target.TDmaxW = target.TDmaxW;
		_target.TDminH = target.TDminH;
		_target.TDminW = target.TDminW;
		_target.TDwidth = target.TDwidth;



		//start to ISP//////////////////////////
		std::tie(boolflag, Gimg, crossCenter, drawF2) = STPchip_singlephase(boolflag, cropedRImg, _thresParm, _target, creteriaPoint, IMGoffset, _chipsetting, creteriaDist, _imageParm);
		
		
	}

	std::cout << "check img state:: " << boolflag << endl;
	std::cout << "check center is ::" << crossCenter << endl;

	

	/*  :::::::OUTPUT area:::::::  */
	outputLEDX[0] = crossCenter.x ;
	outputLEDY[0] = crossCenter.y ;
	Gimg.copyTo(thres_output);
	drawF2.copyTo(image_output);
	boolResult[0] = boolflag;
}


#pragma endregion Main

