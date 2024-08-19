#pragma region function_declare

#include "pch.h"
#include "AOILib_MB0STPchipV2.h"


#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>


using namespace cv;
using namespace std;


/*general operation*/
Point find_piccenter(Mat src);
Mat CropIMG(Mat img, Rect size);
Mat KmeanOP(int knum, Mat src);
bool BrokenChipInspect(Mat src, Rect inspectArea, int knum);
bool checkinframe(Point2f framepoint, Point2f centerpoint, double AceptDist);
vector<vector<Point>> elipsePatch(Mat patterrn, Mat AFFimg, sizeTD target, Point2f creteriaPoint);
std::tuple<vector<double>, vector<Point2f>, vector<RotatedRect>, Mat, bool> defineTDelipse(Mat AFFthres, sizeTD target, Point2f creteriaPoint);

//floodfill
std::tuple<int, Point_<int>> FindMF_pixel(Mat histImg);
std::tuple<Mat, double, int, float> HistFeaturization(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff, Point_<int> cooradd, int fmode);
Mat loopAFF(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff, Point_<int> cooradd, int fmode);
Mat floodfill(Mat src, int px, int py, int newVal, int lodiff, int updiff);



/*chip algorithm */
std::tuple<int, Mat, Point2f, Mat>STPchip_singlephase(float flag, Mat stIMG, thresP thresParm, sizeTD target, Point2f creteriaPoint, Point IMGoffset, SettingP chipsetting, double creteriaDist, ImgP imageParm);

#pragma endregion function_declare


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


		//start to ISP//////////////////////////
		std::tie(boolflag, Gimg, crossCenter, drawF2) = STPchip_singlephase(boolflag, cropedRImg, thresParm, target, creteriaPoint, IMGoffset, chipsetting, creteriaDist, imageParm);
		
		
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


#pragma region GeneralFunction


#pragma region AFFop


std::tuple<int, Point_<int>> FindMF_pixel(Mat histImg)
{
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(histImg, &minVal, &maxVal, &minLoc, &maxLoc);
	return { maxVal,maxLoc };
}

std::tuple<Mat, double, int, float> HistFeaturization(Mat crop, Point_<int> piccenter,
	int coororder, int newVal, int lodiff, int updiff,
	Point_<int> cooradd, int fmode)
{

	Mat histImg, AFFimg, histAFF;
	int imgnumber = 1;
	int imgchannel = 0;
	int imgdimension = 1;
	int histsize = 256;
	float grayrange[2] = { 0,256 };
	const float* grayhistrange = { grayrange };
	bool histuniform = true;
	bool histaccumulate = false;
	float frequentRatio;

	int seedIndex;

	vector<double> disthist, sortdist;
	double  maxVal1, maxVal2, secVal2, remaxVal1;
	Point maxLoc1, secLoc, maxLoc2;

	std::vector < Point2i > coordhist;

	//convert img to hist
	cv::calcHist(&crop, 1, 0, cv::Mat(), histImg, imgdimension, &histsize, &grayhistrange, histuniform, histaccumulate);

	tie(maxVal1, maxLoc1) = FindMF_pixel(histImg);  //find most frequent value and its index(pixel on crop)
	//cout << "firt maxVal is " << maxVal1 << ",,," << maxLoc1 << endl;





	//find all the coordinates on cropimg with the pixel of maxLoc.y
	for (int i = 0; i < crop.rows; i++) {
		for (int j = 0; j < crop.cols; j++)
		{
			if (crop.at<uchar>(i, j) == maxLoc1.y)
			{
				coordhist.push_back(Point(i, j));
			}
		}
	}

	if (fmode == 0)
	{
		//calculate the distance between piccenter and LED center
		for (int i = 0; i < coordhist.size(); i++)
		{
			disthist.push_back(norm(piccenter - coordhist[i]));
		}
		sortdist = disthist;
		std::sort(sortdist.begin(), sortdist.end());

		seedIndex = std::find(disthist.begin(), disthist.end(), sortdist[coororder]) - disthist.begin();
		//cout << "seed is" << coordhist[seedIndex] <<",,,,,"<<piccenter << endl;
		crop.copyTo(AFFimg);
		AFFimg = floodfill(AFFimg, coordhist[seedIndex].x, coordhist[seedIndex].y, newVal, lodiff, updiff);
	}
	else if (fmode == 1)
	{
		crop.copyTo(AFFimg);
		AFFimg = floodfill(AFFimg, cooradd.x, cooradd.y, newVal, lodiff, updiff);
	}



	//rehist floodfill cropped Image
	cv::calcHist(&AFFimg, 1, 0, cv::Mat(), histAFF, imgdimension, &histsize, &grayhistrange, histuniform, histaccumulate);
	tie(maxVal2, maxLoc2) = FindMF_pixel(histAFF);
	//cout << "2nd maxVal is " << maxVal2 << ",,," << maxLoc2 << endl;


	//fid=nd the second maximum hist value in the pic
	Mat hist2nd = histAFF.clone();
	hist2nd.at<float>(maxLoc2) = 0;


	tie(secVal2, secLoc) = FindMF_pixel(hist2nd);
	//cout << "secVal2  is " << secVal2 << ",,," << secLoc << endl;


	frequentRatio = secVal2 / maxVal1;

	return { AFFimg,maxVal2,maxLoc2.y,frequentRatio };
}


Mat loopAFF(Mat crop, Point_<int> piccentercrop, int coororder, int newVal, int lodiff, int updiff,
	Point_<int> cooradd, int fmode)
{
	Mat AFFoutput;
	double maxhistValue, standVal;
	int maxhistpixel;
	float frequentRatio;


	standVal = crop.cols * crop.rows * 0.5 * 0.5;
	tie(AFFoutput, maxhistValue, maxhistpixel, frequentRatio) = HistFeaturization(crop, piccentercrop, coororder, newVal, lodiff, updiff, cooradd, fmode);
	//cout << "current coororder: " << coororder << endl;
	if (maxhistpixel != 0 || maxhistValue <= standVal)
	{
		if (fmode == 0) //computing order mode
		{
			coororder = coororder + 35;  //35		
		}
		else if (fmode == 1)
		{
			cooradd = Point(cooradd.x + 10, cooradd.y + 10); //10
		}
		AFFoutput = loopAFF(crop, piccentercrop, coororder, newVal, lodiff, updiff, cooradd, fmode);


	}
	//cout << "final cooradd: " << cooradd << endl;

	return AFFoutput;
}

Mat floodfill(Mat src, int px, int py, int newVal, int lodiff, int updiff) {


	Mat fImg = src;
	Mat mask = Mat::zeros(fImg.rows + 2, fImg.cols + 2, CV_8UC1);

	Point seed = Point(px, py);
	Rect ccomp;
	floodFill(fImg, mask, seed, newVal, &ccomp, lodiff, updiff, FLOODFILL_FIXED_RANGE);

	mask.release();
	return fImg;

	//Scalar updiff: 3 channels
	//flags=4+FLOODFILL_FIXED_RANGE //4 neighboring
}

#pragma endregion AFFop


#pragma region Generalop

Point find_piccenter(Mat src)
{
	int piccenterx = int(src.size().width * 0.5);
	int piccentery = int(src.size().height * 0.5);
	Point piccenter = Point(piccenterx, piccentery);
	return piccenter;
}

Mat CropIMG(Mat img, Rect size)
{
	Mat croppedIMG;
	img(size).copyTo(croppedIMG);
	return croppedIMG;

}

Mat KmeanOP(int knum, Mat src)
{
	TermCriteria criteria;
	Mat labels, centeres;
	Mat pixelVal, segmentedIMG;

	//define stopping criteria
	criteria = TermCriteria(cv::TermCriteria::EPS + TermCriteria::MAX_ITER, 100, 0.2);
	/* criteria:::
	The algorithm termination criteria, that is, the maximum number of iterations and/or the desired accuracy.
	The accuracy is specified as criteria.epsilon.
	As soon as each of the cluster centers moves by less than criteria.epsilon on some iteration, the algorithm stops.
	*/
	int attempts = 10;
	/*attempts:::
	Flag to specify the number of times the algorithm is executed using different initial labellings.
	The algorithm returns the labels that yield the best compactness(see the last function parameter).
	*/
	int KmeanFlag = cv::KMEANS_RANDOM_CENTERS;
	//	Flag that can take values of cv::KmeansFlags----:cv::KMEANS_RANDOM_CENTERS-Select random initial centers in each attempt.

	Mat samples(src.rows * src.cols, src.channels(), CV_32F); //change to float: Data for clustering
	//knum: //Number of clusters to split the set by
	//labels:  //Input/output integer array that stores the cluster indices for every sample.
	//centeres: //Output matrix of the cluster centers, one row per each cluster center.


	for (int y = 0; y < src.rows; y++)
	{
		for (int x = 0; x < src.cols; x++)
		{
			for (int z = 0; z < src.channels(); z++)
			{
				if (src.channels() == 3)
				{
					samples.at<float>(y + x * src.rows, z) = src.at<Vec3b>(y, x)[z];
				}

				else
				{
					samples.at<float>(y + x * src.rows, z) = src.at<uchar>(y, x); //for gray img
				}

			}
		}
	}
	cv::kmeans(samples, knum, labels, criteria, attempts, KmeanFlag, centeres);

	Mat NewIMG(src.size(), src.type());

	for (int y = 0; y < src.rows; y++)
	{
		for (int x = 0; x < src.cols; x++)
		{
			int cluster_idx = labels.at<int>(y + x * src.rows, 0);
			if (src.channels() == 3)
			{
				for (int z = 0; z < src.channels(); z++) //3 channels
				{
					NewIMG.at<Vec3b>(y, x)[z] = centeres.at<float>(cluster_idx, z);

				}
			}
			else
			{
				NewIMG.at<uchar>(y, x) = centeres.at<float>(cluster_idx, 0); //for gray img
			}


		}
	}


	//std::cout << "finish Kop" << endl;

	return NewIMG;
}
#pragma endregion Generalop



#pragma region BrokenchipCheck

bool BrokenChipInspect(Mat src, Rect inspectArea, int knum)
{
	/*declare parameter*/
	bool InspectState = false;
	Mat Kop;
	std::vector<float> vechist;
	std::vector<int> vechistnonzero;

	int histsize = 256; //256
	float ranges[] = { 0,256 };
	const float* histRanges = { ranges };
	Mat hist, image_gcp;
	int hist_h = 300;
	int hist_w = 256;
	int bin_w = hist_w / histsize;//histogram bins
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));//draw histogram
	double minVal, maxVal, secmaxVal;
	Point minLoc, maxLoc, secmaxLoc;
	/**********************************************/

	image_gcp = CropIMG(src, inspectArea);
	Kop = KmeanOP(knum, image_gcp);
	calcHist(&Kop, 1, 0, Mat(), hist, 1, &histsize, &histRanges, true, false);



	//draw histogram:::
	cv::normalize(hist, hist, 0, hist_h, NORM_MINMAX, -1, Mat());//normalization

	//check histogram via picture::
	for (int i = 1; i < histsize; i++)
	{
		line(histImage, Point((i - 1) * bin_w, hist_h - cvRound(hist.at<float>(i - 1))),
			Point((i)*bin_w, hist_h - cvRound(hist.at<float>(i))), Scalar(255, 0, 0), 2, 8, 0);
	}

	cv::minMaxLoc(hist, &minVal, &maxVal, &minLoc, &maxLoc);
	//std::cout << "check max hist val: " << maxLoc.y << endl;

	//find second largest value::
	Mat Sechist = hist.clone(); // leave original histogram(?)
	Sechist.at<double>(maxLoc) = 0; // careful with the type


	minMaxLoc(Sechist, 0, &secmaxVal, 0, &secmaxLoc);
	/* now secmaxVal is the "second largest" item in your hist*/


	/*Assigns new contents to the vector[vechist], replacing its current contents, and modifying its size accordingly.*/
	vechist.assign(hist.begin<float>(), hist.end<float>());

	for (int j = 0; j < vechist.size(); j++)
	{
		if (vechist[j] != 0)
		{
			vechistnonzero.push_back(j);
			//std::cout << "j is : " << j << endl;
		}
	}
	sort(vechistnonzero.begin(), vechistnonzero.end());




	if ((vechistnonzero[0]) == maxLoc.y
		&& double(vechist[maxLoc.y]) / double(vechist[secmaxLoc.y]) > 1.7
		&& double(vechist[vechistnonzero[2]]) / double(vechist[vechistnonzero[1]]) < 0.3)
		/*this ratio[the abnormal area/the normal area] 1.7 should be modified as an automaticall value*/
	{

		std::cout << "broken chip found :PPP " << endl;
		InspectState = false;
	}
	else
	{
		std::cout << "good chip...." << endl;
		InspectState = true;
	}
	//std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;



	return InspectState;

}

#pragma endregion BrokenchipCheck



#pragma region FocusCheck

std::tuple<vector<double>, vector<Point2f>, vector<RotatedRect>, Mat, bool> defineTDelipse(Mat AFFthres, sizeTD target, Mat markIMG, Point2f creteriaPoint)
{
	/*
		To find target elipse pattern, there are three requirements that needed to be satisfied:
		1. The area of elipse pattern should be in the range between target.TDwidth * target.TDheight * 0.015 & target.TDwidth * target.TDheight * 0.11
		2. The approx size should be equal to 4, 3 ,2.
		3.The angle  of elipse should be in a cetain range.
		If found, return true and some elipse information. otherwise, return false.
	*/
	vector<vector<Point>>  AFFcnt; // Vector for storing contour
	vector<Vec4i> AFFhier;
	vector<RotatedRect> minEllipse;
	vector<double>  AFFdistance;
	vector<Point2f> PointEllip;
	vector<Point> approx;
	RotatedRect RRcnt;
	bool defineflag = false;

	cv::findContours(AFFthres, AFFcnt, AFFhier,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

	//std::cout << "--TD area: " << target.TDwidth * target.TDheight<<" /*/*/*/*/*" << endl;

	for (int i = 0; i < AFFcnt.size(); i++)
	{

		double AFarea = cv::contourArea(AFFcnt[i]);
		cv::approxPolyDP(AFFcnt[i], approx, 10, true);

		Rect bdAFcnt = cv::boundingRect(AFFcnt[i]);

		/*std::cout << "--AFarea: " << AFarea << endl;
		std::cout << "--bdbox: " << bdAFcnt.width << " / " << bdAFcnt.height << endl;*/

		//if (AFarea > target.TDwidth * target.TDheight * 0.015 && AFarea < target.TDwidth * target.TDheight * 0.11) && AFarea< target.TDwidth * target.TDheight*
		if (bdAFcnt.height / target.TDheight > 0.3 && bdAFcnt.width / target.TDwidth < 0.5 && AFarea < target.TDwidth * target.TDheight * 0.11)
		{
			//std::cout << "--approx: " << approx.size() << endl;
			if (approx.size() == 4 || approx.size() == 3 || approx.size() == 2)
			{
				RRcnt = fitEllipse(AFFcnt[i]);
				//std::cout << "--RRcnt.angle: " << RRcnt.angle << endl;

				if (abs(RRcnt.angle) > 175 && abs(RRcnt.angle) < 183 ||
					abs(RRcnt.angle) > 85 && abs(RRcnt.angle) < 93 ||
					abs(RRcnt.angle) > 0 && abs(RRcnt.angle) < 5)
				{
					minEllipse.push_back(RRcnt);
					ellipse(markIMG, RRcnt, Scalar(0, 255, 255), 2);
					//AreaEllip.push_back(RRcnt.size.width * RRcnt.size.height);
					Moments AFFm = (cv::moments(AFFcnt[i], false));
					PointEllip.push_back(Point2i((AFFm.m10 / AFFm.m00), (AFFm.m01 / AFFm.m00)));
					AFFdistance.push_back(norm(creteriaPoint - PointEllip[PointEllip.size() - 1]));
				}
				else
				{
					std::cout << "--question is :: " << " angle issue" << endl;
				}
			}
			else
			{
				std::cout << "--question is :: " << " approx issue" << endl;
			}

		}
		else if (AFarea > 2500 * (target.TDheight * target.TDwidth / (246 * 122))) /*refind ellipse in pattern */
		{
			Mat  reAffthres = Mat::zeros(AFFthres.size(), CV_8UC1);
			cv::drawContours(reAffthres, AFFcnt, i, Scalar(255, 255, 255), -1);
			if (bdAFcnt.width > bdAFcnt.height)
			{
				cv::morphologyEx(reAffthres, reAffthres, cv::MORPH_OPEN, Mat::ones(Size(1, int(20 * bdAFcnt.width / 122)), CV_8UC1), Point(-1, -1), 2);
			}
			else
			{
				cv::morphologyEx(reAffthres, reAffthres, cv::MORPH_OPEN, Mat::ones(Size(int(20 * bdAFcnt.height / 122), 1), CV_8UC1), Point(-1, -1), 2);
			}
			vector<vector<Point>>  ARefindcnt;
			cv::findContours(reAffthres, ARefindcnt, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

			for (int i = 0; i < ARefindcnt.size(); i++)
			{
				double AFarea2 = cv::contourArea(ARefindcnt[i]);
				cv::approxPolyDP(ARefindcnt[i], approx, 10, true);
				Rect bdAFcnt2 = cv::boundingRect(ARefindcnt[i]);
				RRcnt = fitEllipse(ARefindcnt[i]);
				if (bdAFcnt2.height / target.TDheight > 0.3 && bdAFcnt2.width / target.TDwidth < 0.5
					&& AFarea2 < target.TDwidth * target.TDheight * 0.11
					&& approx.size()>1 && approx.size() < 5)
				{
					if (abs(RRcnt.angle) > 175 && abs(RRcnt.angle) < 183 ||
						abs(RRcnt.angle) > 85 && abs(RRcnt.angle) < 93 ||
						abs(RRcnt.angle) > 0 && abs(RRcnt.angle) < 5)
					{
						minEllipse.push_back(RRcnt);
						ellipse(markIMG, RRcnt, Scalar(0, 255, 255), 2);
						//AreaEllip.push_back(RRcnt.size.width * RRcnt.size.height);
						Moments AFFm = (cv::moments(ARefindcnt[i], false));
						PointEllip.push_back(Point2i((AFFm.m10 / AFFm.m00), (AFFm.m01 / AFFm.m00)));
						AFFdistance.push_back(norm(creteriaPoint - PointEllip[PointEllip.size() - 1]));
					}
				}

			}

		}



	}

	if (AFFdistance.size() != 0)
	{
		defineflag = true;
	}
	else
	{
		defineflag = false;
	}


	return { AFFdistance ,PointEllip ,minEllipse ,markIMG,defineflag };
}


vector<vector<Point>> elipsePatch(Mat patterrn, Mat AFFimg, sizeTD target, Point2f creteriaPoint)
{

	Mat elipsepatch = Mat::zeros(patterrn.size(), CV_8UC1);

	patterrn.copyTo(elipsepatch);

	Mat markIMG;
	cvtColor(AFFimg, markIMG, COLOR_GRAY2RGB);

	/*find elipse start:::*/
	double maxValAFF, minValAFF;
	Point minLoc;
	Mat AFFthres;

	vector<vector<Point>>  AFFcnt, TDcnt, elipcnt; // Vector for storing contour
	vector<Vec4i> AFFhier, eliphier;
	vector<RotatedRect> minEllipse;
	vector<double>  AFFdistance;
	vector<Point2f> PointEllip;
	RotatedRect elipRect;
	bool elipflag = false;

	/*find elipse start2:::*/
	Mat elipPointsF, elipPointsInt;


	Mat AFF2 = Mat::zeros(patterrn.size(), CV_8UC1);



	cv::findContours(patterrn, TDcnt,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());


	Moments TDM = (cv::moments(TDcnt[0], false));
	Rect bdTDpat = cv::boundingRect(TDcnt[0]);
	Point patcenter = (Point2i((TDM.m10 / TDM.m00), (TDM.m01 / TDM.m00)));
	//std::cout << "center is pattern:: " << patcenter << endl;

	Mat AFFThresmsk;

	//bitwise_and(AFFimg, AFFimg, AFFThresmsk, patterrn);

	AFFThresmsk = CropIMG(AFFimg, bdTDpat);



	Mat histImg;
	int imgdimension = 1;
	int histsize = 256;
	float grayrange[2] = { 0,256 };
	const float* grayhistrange = { grayrange };
	double  maxVal1, secVal2, minvalhist;
	Point maxLoc1, secLoc;
	int hist_h = 300;//the hieght of histogram=300
	int hist_w = 256; //the width of histogram=256
	int bin_w = hist_w / histsize;//histogram bins
	Mat histplot(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));//draw histogram



	//convert img to hist
	cv::calcHist(&AFFThresmsk, 1, 0, cv::Mat(), histImg, imgdimension, &histsize, &grayhistrange, true, false);

	tie(maxVal1, maxLoc1) = FindMF_pixel(histImg);  //find most frequent value and its index(pixel on crop)


	minMaxLoc(histImg, &minvalhist, 0, 0, 0);

	/*
	Set a creteria for automatically featurazation, in order to discrete each bright part on one chip:
	the creteria value is to find non-zero pixel in histogram, then follow the formula to get the creteria
	formula: [const+(max-min)*0.2]
	the constant is 2, generally const=1 also works.
	the threshold filter will be set at ((maxValAFF- histstdnum) * 0.7)
	*/
	vector<int>pxnonzero;
	for (int y = 0; y < histImg.rows; y++)
	{
		if (histImg.at<float>(y, 0) != 0)
		{
			pxnonzero.push_back(y);

		}
	}

	int histstdnum = 2 + int((pxnonzero[pxnonzero.size() - 1] - pxnonzero[0]) * 0.2); //default fixed value: histdnum=50 
	/*std::cout << "check histstdnum:  " << histstdnum << endl;
	std::cout << "check minvalhist:::: :" << minvalhist << endl;
	std::cout << "check maxVal1 ::: :" << maxVal1 << endl;*/


	for (int y = 0; y < histImg.rows; y++)
	{
		/*std::cout << "current y: " << y << endl;
		std::cout << "current ypixel: " << float(histImg.at<float>(y, 0)) << endl;*/

		if (y < histstdnum)//this should be modified::via histogram ratio
		{
			histImg.at<float>(y, 0) = 0; //for gray img
			//std::cout << "new ypixel: " << float(histImg.at<float>(y, 0)) << endl;

		}



	}
	//std::cout << "index testpixel: " << float(histImg.at<float>(0, 10)) << endl;




	Mat hist2nd = histImg.clone();

	std::tie(secVal2, secLoc) = FindMF_pixel(hist2nd);


	//draw histogram:::
	cv::normalize(histImg, histImg, 0, hist_h, NORM_MINMAX, -1, Mat());//normalization

	//check histogram via picture::
	for (int i = 1; i < histsize; i++)
	{
		line(histplot, Point((i - 1) * bin_w, hist_h - cvRound(histImg.at<float>(i - 1))),
			Point((i)*bin_w, hist_h - cvRound(histImg.at<float>(i))), Scalar(255, 0, 0), 2, 8, 0);
	}

	minMaxLoc(AFFThresmsk, 0, &maxValAFF, 0, 0);



	cv::threshold(AFFimg, AFFthres, int((maxValAFF - histstdnum) * 0.7), 255, THRESH_BINARY); //0.7


	cv::medianBlur(AFFthres, AFFthres, 7);



	std::tie(AFFdistance, PointEllip, minEllipse, markIMG, elipflag) = defineTDelipse(AFFthres, target, markIMG, creteriaPoint);



	if (AFFdistance.size() == 0)
	{
		std::cout << "retry defineTDelipse......" << endl;

		Size CLKelip;

		cv::medianBlur(AFFthres, AFF2, 11);

		if (target.TDheight < target.TDwidth)
		{
			CLKelip = Size(1, 30);
			cv::morphologyEx(AFFthres, AFF2, cv::MORPH_OPEN, Mat::ones(CLKelip, CV_8UC1), Point(-1, -1), 2);
		}
		else
		{
			CLKelip = Size(15, 1);
			cv::morphologyEx(AFFthres, AFFthres, cv::MORPH_CLOSE, Mat::ones(Size(10, 3), CV_8UC1), Point(-1, -1), 2);
			cv::morphologyEx(AFFthres, AFF2, cv::MORPH_OPEN, Mat::ones(CLKelip, CV_8UC1), Point(-1, -1), 1);
			//std::cout << "gggggg/*/*/*/*/ " << endl;
		}
		cvtColor(AFFimg, markIMG, COLOR_GRAY2RGB);
		std::tie(AFFdistance, PointEllip, minEllipse, markIMG, elipflag) = defineTDelipse(AFF2, target, markIMG, creteriaPoint);
	}
	else { elipflag = true; }


	if (elipflag == true)
	{


		auto itAFF = std::min_element(AFFdistance.begin(), AFFdistance.end());
		int minIAF = std::distance(AFFdistance.begin(), itAFF);



		Point creAPoint = PointEllip[minIAF];
		Point diffcenter = creAPoint - patcenter;

		/*std::cout << "check elipse w/h " << minEllipse[minIAF].size.width << " / " << minEllipse[minIAF].size.height << " , " << minEllipse[minIAF].angle << endl;
		std::cout << "center is elipse:: " << creAPoint << endl;
		std::cout << "center is diff:: " << diffcenter << endl;*/


		double lenratio = 0.73;  //0.75 
		int offsetval;
		double stdlen, stdboundlen;
		if (target.TDheight < target.TDwidth)
		{
			stdlen = target.TDheight;
			stdboundlen = target.TDwidth;
			if (diffcenter.x > 0)
			{
				/*left:*/
				offsetval = (creAPoint.x - (int(creAPoint.x - target.TDwidth * 0.7 * 0.5) + int(target.TDwidth * (lenratio) * 0.5)));
				elipRect = RotatedRect(Point(int(creAPoint.x - target.TDwidth * 0.7 * 0.5 + 0.5 * offsetval), creAPoint.y), Size(int(target.TDwidth * (lenratio)), minEllipse[minIAF].size.height), 0);
			}
			else
			{
				/*right:*/
				offsetval = (creAPoint.x - (int(creAPoint.x + target.TDwidth * 0.7 * 0.5) - int(target.TDwidth * (lenratio) * 0.5)));
				elipRect = RotatedRect(Point(int(creAPoint.x + target.TDwidth * 0.7 * 0.5 + 0.5 * offsetval), creAPoint.y), Size(int(target.TDwidth * (lenratio)), minEllipse[minIAF].size.height), 0);
			}
		}
		else
		{
			stdlen = target.TDwidth;
			stdboundlen = target.TDheight;
			if (diffcenter.y > 0)
			{
				/*down:*/
				//std::cout << "check dy>0 is " << diffcenter.y << endl;
				offsetval = (creAPoint.y - (int(creAPoint.y - target.TDheight * 0.7 * 0.5) + int(target.TDheight * (lenratio) * 0.5)));
				elipRect = RotatedRect(Point(creAPoint.x, int(creAPoint.y - target.TDheight * 0.7 * 0.5 + 0.5 * offsetval)), Size(minEllipse[minIAF].size.height, int(target.TDheight * (lenratio))), 0);
			}
			else
			{
				/*up:*/
				offsetval = (creAPoint.y - (int(creAPoint.y + target.TDheight * 0.7 * 0.5) - int(target.TDheight * (lenratio) * 0.5)));
				elipRect = RotatedRect(Point(creAPoint.x, int(creAPoint.y + target.TDheight * 0.7 * 0.5 + 0.5 * offsetval)),
					Size(minEllipse[minIAF].size.height, int(target.TDheight * (lenratio))), 0);
			}
		}



		/*find elipse start2:::*/
		//std::cout << "phase2 : " << minEllipse[minIAF].size.height << " , " << stdlen * 1.1 << endl;
		if (minEllipse[minIAF].size.height < stdlen * 1.1) /*需確認blur*/
		{
			boxPoints(elipRect, elipPointsF);
			elipPointsF.assignTo(elipPointsInt, CV_32S);
			cv::polylines(markIMG, elipPointsInt, true, Scalar(255, 255, 255), 2);
			cv::fillPoly(elipsepatch, elipPointsInt, Scalar(255, 255, 255));
			cv::circle(markIMG, creAPoint, 2, Scalar(255, 255, 0), 2);

			/*find elipse end2:::*/

			cv::findContours(elipsepatch, elipcnt, eliphier,
				cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
		}
		else
		{
			//this condition is when elipse length is larger than target height, while the elipcnt will be zero,the flag will return error code=5
			ellipse(markIMG, minEllipse[minIAF], Scalar(110, 180, 130), 2);

		}


	}
	else
	{
		std::cout << "without ellipse......" << endl;
	}

	std::cout << "finish elipcnt " << elipcnt.size() << endl;

	std::cout << "finish " << endl;
	return elipcnt;
}

#pragma endregion FocusCheck


#pragma region ShiftCheck

bool checkinframe(Point2f framepoint, Point2f centerpoint, double AceptDist)
{
	return{ abs((norm(centerpoint - framepoint))) < AceptDist };
}

#pragma endregion ShiftCheck




#pragma endregion GeneralFunction


#pragma region ChipAlgorithm

std::tuple<int, Mat, Point2f, Mat>STPchip_singlephase(float flag, Mat stIMG, thresP thresParm, sizeTD target, Point2f creteriaPoint, Point IMGoffset, SettingP chipsetting, double creteriaDist, ImgP imageParm)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	/////Declare Parm.0-output parm:::::
	Mat Reqcomthres = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat Grayinspect = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat marksize;
	Point2f centercoord;

	/////Declare Parm.1-image pre-processing::: 
	Mat Gimg, gauGray, adwIMG, maskout;
	int px, py;
	Mat Bitnd = Mat::zeros(stIMG.size(), CV_8UC1);
	Mat AFFmsk = Mat::zeros(stIMG.size(), CV_8UC1);
	Mat AFFmskfi = Mat::zeros(stIMG.size(), CV_8UC1);
	Mat AFFkernel = Mat::ones(Size(10, 10), CV_8UC1);
	vector<vector<Point>>  cntAFF, cntAFFfi; // Vector for storing contour
	vector<Vec4i> hierAFF;
	const int lodiff = 15; //15 //5
	const int updiff = 15;
	const int newVal = 0;
	int coororder = 0;
	int fmode = 1; // 0:oeder mode; 1:coordinate mode
	Point cooradd = find_piccenter(stIMG);
	Point croppiccenter = cooradd;
	Mat AFFimg;



	/////Declare Parm.2-thresVal filtering & remove tiny dust cnt &  huge bulk interference cnt:::
	int adaptWsize = 289;
	int adaptKsize = 4;
	Mat acheckinner, acheckinner2;
	vector<vector<Point>>  Innercnt_ini, Innercnt_fin; // cnt for thresVal-filtering
	vector<Vec4i> Innerhier;
	vector<double> Arealist;
	vector<vector<Point>> interfereScnt; //dust cnt
	Size S_Kcomclose;
	int dimensioncheck = 0;
	int areacntcheck = 0;



	/////Declare Parm.3-dimension filtering & define the target chip with the shortest distance to the pic center:::	
	vector<double> distance;
	int minIndex;
	RotatedRect box;
	double areacomthres;
	vector<vector<Point>> reqConH;


	/////Declare Parm.4-Determine if the chip fits in these two conditions: 1.chip is in the frame region / 2. chip is rotated:::
	Mat ReqImg = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat boxPointsF, boxPointsInt;

	//////Declare Parm.5-Determine if the chip is at well focus length or the optical power is suitable.
	Rect bdbox;
	vector<vector<Point>> elippatchCNT;

	//////Declare Parm.6 Determine if the chip is broken or not, if not, calculate the coordinate of chip center.
	bool brokeninspect = false;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//Step1. image pre-processing & return color image [input:stIMG / output:maskout,marksize]
	/*
	There will be two pics as output in this step:
		1.maskout is the result after loopAFF operation.
		2.marksize is a picture in RGB mode for monitoring the final inspect result.
	Herein, a procedure is depicted as below:
		A.image preprocessing
		B.loopAFF: floodfill operation + target filter via contour
		C.bitwise operation with a customized mask
	*/
	cv::cvtColor(stIMG, marksize, COLOR_GRAY2RGB);
	stIMG.convertTo(Gimg, -1, 1.7, 0);//-1 / 1.2
	cv::GaussianBlur(stIMG, gauGray, Size(0, 0), 3);
	cv::addWeighted(Gimg, 1.5, gauGray, -0.5, 0.0, adwIMG);
	AFFimg = loopAFF(gauGray, croppiccenter, coororder, newVal, lodiff, updiff, cooradd, fmode);

	//create a mask of AFF result
	cv::medianBlur(AFFimg, AFFimg, 15);
	cv::findNonZero(AFFimg, Bitnd);

	for (int i = 0; i < Bitnd.total(); i++)
	{
		px = Bitnd.at<Point>(i).x;
		py = Bitnd.at<Point>(i).y;
		AFFmsk.at<uchar>(py, px) = 255;
	}
	cv::morphologyEx(AFFmsk, AFFmsk, cv::MORPH_CLOSE, AFFkernel, Point(-1, -1), 4);

	//filter target region:
	cv::findContours(AFFmsk, cntAFF, hierAFF,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
	/************************************/
	try
	{

		for (int i = 0; i < cntAFF.size(); i++)
		{
			double AFFarea = cv::contourArea(cntAFF[i]);
			/*std::cout << "check AFFarea: " << AFFarea << endl;
			std::cout << "check AFFarea2: " << target.TDwidth * target.TDheight * 0.05*0.5 << endl;
			std::cout << "check AFFarea3: " << target.TDwidth * target.TDheight * 20 << endl;*/

			if (AFFarea > double(target.TDwidth * target.TDheight * 0.6 * 0.6) && AFFarea < double(target.TDwidth * target.TDheight * 20))
			{

				Rect AFFbox = cv::boundingRect(cntAFF[i]);
				cv::rectangle(AFFmskfi, AFFbox, Scalar(255, 255, 255), -1);
			}

		}

		//cntAFF.clear();

		cv::findContours(AFFmskfi, cntAFFfi, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

		if (cntAFFfi.size() == 0)
		{
			flag = 2.0;
			AFFmsk.copyTo(Grayinspect);
			throw"something wrong::dimension , threshold, issue,abnormal target(1/3)";
		}


		cv::morphologyEx(AFFmskfi, AFFmskfi, cv::MORPH_DILATE, AFFkernel, Point(-1, -1), 4);
		bitwise_and(adwIMG, adwIMG, maskout, AFFmskfi);
		Bitnd.release();




		//Step2. thresVal filtering & remove tiny dust cnt &  huge bulk interference cnt
		//[input:maskout / output : acheckinner2]

		if (thresParm.thresmode == 0)
		{
			std::cout << "gray mode-*-*-" << endl;
			Scalar maxthres = Scalar(thresParm.fgmax[imageParm.PICmode], thresParm.fgmax[imageParm.PICmode], thresParm.fgmax[imageParm.PICmode]);
			Scalar minthres = Scalar(thresParm.fgmin[imageParm.PICmode], thresParm.fgmin[imageParm.PICmode], thresParm.fgmin[imageParm.PICmode]);
			cv::inRange(maskout, minthres, maxthres, acheckinner); //input=Gimg
			cv::medianBlur(acheckinner, acheckinner, 9);
		}
		else if (thresParm.thresmode == 3)
		{
			Mat thresadp;
			adaptWsize = thresParm.bgmax[imageParm.PICmode];
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
			/*此處應新增防護機制: adaptWsize須強制為奇數*/
			adaptiveThreshold(maskout, thresadp, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, adaptWsize, adaptKsize);
			bitwise_and(thresadp, thresadp, acheckinner, AFFmskfi);
			cv::medianBlur(acheckinner, acheckinner, 9);

		}
		else if (thresParm.thresmode == 4)
		{
			Mat thresadp;
			adaptWsize = thresParm.bgmax[imageParm.PICmode];
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
			/*此處應新增防護機制: adaptWsize須強制為奇數*/
			adaptiveThreshold(maskout, thresadp, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, adaptWsize, adaptKsize);
			bitwise_and(thresadp, thresadp, acheckinner, AFFmskfi);
			cv::medianBlur(acheckinner, acheckinner, 9);

		}


		cv::findContours(acheckinner, Innercnt_ini, Innerhier,
			cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());


		//try
		//{
		if (Innercnt_ini.size() == 0)
		{
			flag = 1.1;
			acheckinner.copyTo(Grayinspect);
			throw"something wrong::threshold issue";

		}
		else
		{
			//remove tint dust:::
			for (int i = 0; i < Innercnt_ini.size(); i++)
			{
				double interferearea = cv::contourArea(Innercnt_ini[i]);
				Arealist.push_back(interferearea);
				//std::cout << "check  area:: " << cv::contourArea(Innercnt_ini[i]) << endl;
				if (interferearea < 1000) //dust area
				{
					interfereScnt.push_back(Innercnt_ini[i]); //dust countour
				}
			}

			double areamax = *std::max_element(Arealist.begin(), Arealist.end());
			bool maxcover = false;

			//remove:Large interference
			if (areamax > target.TDheight * target.TDwidth * 5 && interfereScnt.size() + 1 != Innercnt_ini.size())//5 //Large interference contour
			{
				auto it = std::find(Arealist.begin(), Arealist.end(), areamax);
				int interfereIndex = std::distance(Arealist.begin(), it);


				cv::drawContours(acheckinner, Innercnt_ini, interfereIndex, Scalar(0, 0, 0), -1); //remove:Large interference contour
			}



			cv::drawContours(acheckinner, interfereScnt, -1, Scalar(0, 0, 0), -1);//remove: dust countour



			if (thresParm.thresmode == 0)
			{
				if (target.TDheight > target.TDwidth)
				{
					S_Kcomclose = Size(round(3 * target.TDwidth / 126), round(20 * target.TDheight / 240));
				}
				else
				{
					S_Kcomclose = Size(round(20 * target.TDwidth / 240), round(3 * target.TDheight / 126));//10,1
				}

				Mat Kcomclose = Mat::ones(S_Kcomclose, CV_8UC1);
				cv::morphologyEx(acheckinner, acheckinner, cv::MORPH_OPEN, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1), 1);
				cv::morphologyEx(acheckinner, acheckinner, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 1);
				Kcomclose.release();

				cv::findContours(acheckinner, Innercnt_ini,
					cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());

				for (int i = 0; i < Innercnt_ini.size(); i++)
				{
					Rect bdch = boundingRect(Innercnt_ini[i]);
					double Areacheck = contourArea(Innercnt_ini[i]);

					if (bdch.width > target.TDminW * target.TDwidth
						&& bdch.width < target.TDmaxW * target.TDwidth
						&& bdch.height < target.TDmaxH * target.TDheight
						&& bdch.height > target.TDminH * target.TDheight)
					{
						drawContours(acheckinner2, Innercnt_ini, i, Scalar(255, 255, 255), -1);
					}
					else
					{
						dimensioncheck++;
					}
				}



			}
			else //mode=3, mode=4
			{
				Innercnt_ini.clear();
				cv::findContours(acheckinner, Innercnt_ini,
					cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());

				acheckinner2 = Mat::zeros(acheckinner.size(), CV_8UC1);

				for (int i = 0; i < Innercnt_ini.size(); i++)
				{
					Rect bdch = boundingRect(Innercnt_ini[i]);
					double Areacheck = contourArea(Innercnt_ini[i]);

					/*std::cout << "calculate area is : " << Areacheck  <<" / "<< target.TDminH * target.TDheight * target.TDminW * target.TDwidth << endl;
					std::cout << "calculate bdch is : " << bdch << endl;
					std::cout << "calculate target.TDheight is : " << target.TDheight<< " / "<< target.TDwidth << endl;*/

					if (bdch.width > target.TDminW * target.TDwidth
						&& bdch.width < target.TDmaxW * target.TDwidth
						&& bdch.height < target.TDmaxH * target.TDheight
						&& bdch.height > target.TDminH * target.TDheight
						&& Areacheck>target.TDminH * target.TDheight * target.TDminW * target.TDwidth)
					{
						drawContours(acheckinner2, Innercnt_ini, i, Scalar(255, 255, 255), -1);
						//std::cout << "current area is : " << Areacheck <<" / "<< target.TDminH * target.TDheight * target.TDminW * target.TDwidth << endl;

					}
					else
					{

						dimensioncheck++;

						if (Areacheck < target.TDminH * target.TDheight * target.TDminW * target.TDwidth)
						{
							areacntcheck++;
						}
					}
				}

			}







			//Step3. dimension filtering & define the target chip with the shortest distance to the pic center
			//[input:acheckinner2 / output : center, distance,reqConH,box]
			cv::findContours(acheckinner2, Innercnt_fin, Innerhier,
				cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());


			if (Innercnt_fin.size() == 0)
			{

				Innercnt_ini.clear();
				cv::findContours(acheckinner, Innercnt_ini, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

				//std::cout << "check size of innercnt_ini is : " << Innercnt_ini.size() << endl;

				if (Innercnt_ini.size() == 1)
				{

					box = cv::minAreaRect(Innercnt_ini[0]);



					if (4.4 < abs(box.angle) && abs(box.angle) < 85.6)
					{
						flag = 4.0;
						acheckinner.copyTo(Grayinspect);
						boxPoints(box, boxPointsF);
						boxPointsF.assignTo(boxPointsInt, CV_32S);
						cv::polylines(marksize, boxPointsInt, true, Scalar(0, 250, 250), 2);
						throw "Angle rotation issue(1/2)";
					}//if 4.4 < abs(box.angle) && abs(box.angle) < 85.6

					else
					{
						bdbox = cv::boundingRect(Innercnt_ini[0]);
						brokeninspect = BrokenChipInspect(stIMG, bdbox, 3);

						if (dimensioncheck > 0)
						{


							if (brokeninspect == false && contourArea(Innercnt_ini[0]) > 0.5 * target.TDminH * target.TDminW * target.TDwidth * target.TDheight)
							{
								flag = 4.2;
								stIMG.copyTo(Grayinspect);
								throw "broken chip(1/3)";
							}
							else //good chip +not rotation
							{

								flag = 2.0;
								acheckinner.copyTo(Grayinspect);
								throw "something wrong::dimension , threshold, issue,abnormal target(2/3)";

							}

						}//if loop: dimensioncheck > 0
						else
						{

							if (brokeninspect == false)
							{
								flag = 4.2;
								stIMG.copyTo(Grayinspect);
								throw "broken chip(2/3)";
							}
							else //good chip
							{
								flag = 2.1;
								throw "something wrong::dimension issue(1/3)";
							}
						} //if-else loop: dimensioncheck > 0
					}//if-else 4.4 < abs(box.angle) && abs(box.angle) < 85.6									
				} //if (Innercnt_ini.size() == 1)
				else
				{
					if (areacntcheck > 0)
					{

						flag = 2.1;
						acheckinner.copyTo(Grayinspect);
						throw "something wrong:: something wrong::dimension issue(2/3)";
					}
					else
					{
						flag = 2.0;
						acheckinner.copyTo(Grayinspect);
						throw "something wrong::dimension , threshold, issue,abnormal target(3/3)";
					}


				}	//if-else (Innercnt_ini.size() == 1)			
			}
			else
			{
				acheckinner2.copyTo(Grayinspect);

				for (int i = 0; i < Innercnt_fin.size(); i++)
				{

					areacomthres = cv::contourArea(Innercnt_fin[i]);
					//std::cout << "check area is "<< areacomthres <<", "<< target.TDheight * target.TDwidth * 0.8 << endl;

					if (areacomthres > target.TDheight * target.TDwidth * target.TDminH * target.TDminW && //0.5
						areacomthres < target.TDheight * target.TDwidth * target.TDmaxH * target.TDmaxW) //1.1
					{

						//Rect sizeRect = boundingRect(Innercnt[i]);
						//cv::rectangle(marksize, sizeRect, Scalar(255, 255, 255), 2);
						Moments Mcont = (moments(Innercnt_fin[i], false));
						Point2f center = Point2i((Mcont.m10 / Mcont.m00), (Mcont.m01 / Mcont.m00));
						distance.push_back(norm(creteriaPoint - center));
						reqConH.push_back(Innercnt_fin[i]);

					}
				}

				if (distance.size() == 0)
				{
					flag = 2.1;
					throw "something wrong::dimension issue(3/3)";
				}
				else
				{
					/*Find the only one answer:::*/
					auto it = std::min_element(distance.begin(), distance.end());
					minIndex = std::distance(distance.begin(), it); //get only one answer
					cv::drawContours(ReqImg, reqConH, minIndex, Scalar(255, 255, 255), -1);
					box = cv::minAreaRect(reqConH[minIndex]);


					//Step4. Determine if the chip fits in these two conditions: 1.chip is in the frame region / 2. chip is rotated .
					//[input:[distance,minIndex,box,marksize] / output:[Reqcomthres,marksize]
					if (distance[minIndex] > creteriaDist)
					{
						flag = 3.0;
						Rect drawbox = boundingRect(reqConH[minIndex]);
						cv::rectangle(marksize, drawbox, Scalar(255, 0, 0), 3);
						//cv::drawContours(marksize, reqConH, minIndex, Scalar(255, 0, 0), 3);
						cv::line(marksize, Point(0, int(marksize.rows * 0.5)), Point(marksize.cols, int(marksize.rows * 0.5)), Scalar(0, 0, 255), 2);
						cv::line(marksize, Point(int(marksize.cols * 0.5), 0), Point(int(marksize.cols * 0.5), marksize.rows), Scalar(0, 0, 255), 2);

						throw "The chip is not in the frame...";
					}
					else
					{
						//std::cout << "check angle roataion:: " << abs(box.angle) << endl;

						if (4.4 < abs(box.angle) && abs(box.angle) < 85.6)
						{
							flag = 4.0;
							boxPoints(box, boxPointsF);
							boxPointsF.assignTo(boxPointsInt, CV_32S);
							cv::polylines(marksize, boxPointsInt, true, Scalar(0, 250, 250), 2);
							cv::fillPoly(Reqcomthres, boxPointsInt, Scalar(255, 255, 255));
							throw "Angle rotation issue(2/2)";
						}

						else
						{

							//Step5. Determine if the chip is at well focus length or the optical power is suitable.
							//[input:[ReqImg, stIMG, target, creteriaPoint] / output:[box,boxPointsInt,bdbox]
							elippatchCNT = elipsePatch(ReqImg, stIMG, target, creteriaPoint);

							//std::cout << "check cons: size: " << elippatchCNT.size() << endl;

							cv::drawContours(Reqcomthres, elippatchCNT, -1, Scalar(255, 255, 255), -1);


							if (elippatchCNT.size() == 0)
							{
								flag = 4.1;
								throw "focus issue & optical power issue";
							}

							else if (elippatchCNT.size() == 1)
							{
								//calculate rotation::

								bdbox = cv::boundingRect(elippatchCNT[0]); //bounding rect
								boxPoints(box, boxPointsF);
								boxPointsF.assignTo(boxPointsInt, CV_32S);
							}

							else
							{	//when elipse size is larger than 1 --> generally will not happen.
								box = cv::minAreaRect(reqConH[minIndex]); //rotated rect
								bdbox = cv::boundingRect(reqConH[minIndex]); //bounding rect
								boxPoints(box, boxPointsF);
								boxPointsF.assignTo(boxPointsInt, CV_32S);
							}


							//Step6. Determine if the chip is broken or not, if not, calculate the coordinate of chip center.
							//[input:[stIMG, bdbox] / output:[marksize,centercoord]								
							brokeninspect = BrokenChipInspect(stIMG, bdbox, 3);

							//std::cout << "check bool broken chip inspection:  " << brokeninspect << endl;

							if (brokeninspect == true)
							{
								cv::rectangle(marksize, bdbox, Scalar(180, 0, 120), 2);
								bdbox.x = bdbox.x + chipsetting.carx;
								bdbox.y = bdbox.y + chipsetting.cary;
								cv::rectangle(marksize, bdbox, Scalar(0, 0, 255), 3);
								cv::rectangle(maskout, bdbox, Scalar(180, 180, 180), 2);
								/* find center:: */
								centercoord = Point(bdbox.x + 0.5 * bdbox.width + IMGoffset.x, bdbox.y + 0.5 * bdbox.height + IMGoffset.y) + Point(chipsetting.carx, chipsetting.cary);
								//cv::circle(marksize, Point(bdbox.x + 0.5 * bdbox.width , bdbox.y + 0.5 * bdbox.height ), 2, Scalar(255, 0, 255), 5);
								//std::cout << "center coordinate is :: " << centercoord << endl;
								flag = 9;
							}
							else
							{
								flag = 4.2;
								stIMG.copyTo(Grayinspect);
								throw "broken chip(3/3)";
							}//if-loop: broken chip inspection
						}//if-loop: chip rotation
					}//if-loop: chip is not in the frame						
				} //if-loop:distance.size() != 0

			} // if-loop:Innercnt.size()!=0 : remove dust

		}// if-loop:Innercnt.size()!=0 :thresholdVal
	}
	catch (const char* message)
	{
		std::cout << message << std::endl;

	}

	boxPointsF.release();
	boxPointsInt.release();
	gauGray.release();
	adwIMG.release();
	AFFkernel.release();

	//////////////////////////////////////////////////////////output//////////////////////////////////
	auto t_end = std::chrono::high_resolution_clock::now();
	double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "check chip center is: [ " << centercoord << " ]" << endl;
	std::cout << "result flag is :: " << flag << endl;
	//std::cout << "check distance:: " << distance[minIndex] << endl;
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "calculate color-filter time is:: " << elapsed_time_ms << endl;



	std::cout << "end" << endl;

	return { flag, Grayinspect, centercoord, marksize };
}

#pragma endregion ChipAlgorithm





