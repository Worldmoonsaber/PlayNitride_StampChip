#include "pch.h"
#include "AOILib_Chip_DLL.h"
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation

using namespace cv;
using namespace std;


Point_<int> find_piccenter(Mat src);
Mat floodfill(Mat src, int px, int py, int newVal, int lodiff, int updiff);
Mat CropIMG(Mat img, Rect size);
std::tuple<Point_<int>, bool>ISP(Mat bg, Mat fg, int LEDh, int LEDw, int carx, int cary, Point piccenter);
void MyFilledCircle(Mat img, Point center);
std::tuple<int, Point_<int>> FindMF_pixel(Mat histImg);
std::tuple<Mat, double, int, float> HistFeaturization(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff);
Mat loopAFF(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff);



void calcenter(int PICmode, char Rimg[], float outputLED[],bool flag)
{ //int LEDcenter[]

	int carx, cary, piccenterx, piccentery, picorder;
	Mat original, gray, fImg;
	Mat cropedFImg, cropedRImg, croppedAFFimg;
	Point bgcoor, piccenter, croppiccenter;
	int coororder = 0;


	const int lodiff = 15;
	const int updiff = 15;
	const int newVal = 0;



	const int LEDw = int(36 * 3.45); //124
	const int LEDh = int(15 * 3.45);//51
	const int Ratio = 6;
	

	Point_<int> LEDcoordinate;

	if (PICmode == 0)
	{
		original = imread(Rimg);

		cvtColor(original, gray, COLOR_BGR2GRAY);
	}
	else if (PICmode == 1)
	{
		Mat inputIMG;
		original = inputIMG;
		cvtColor(original, gray, COLOR_BGRA2GRAY);
	}



	piccenter = find_piccenter(gray);
	//cout << "pic center is: (" << piccenterx << "," << piccentery <<")" << endl;
	piccenterx = piccenter.x;
	piccentery = piccenter.y;



	//flood fill picture:
	//gray.copyTo(fImg); //copy img
	//fImg = floodfill(fImg, bgx, bgy, newVal, lodiff, updiff);
	//MyFilledCircle (fImg,piccenter); //check pic center
	//namedWindow("floodfill");
	//imshow("floodfill", fImg);

	//Crop image ratio
	carx = piccenterx - Ratio * LEDw;
	cary = piccentery - Ratio * LEDw;
	croppiccenter = Point(piccenter.x - carx, piccenter.y - cary);



	Rect crop_region(carx, cary, 2 * Ratio * LEDw, 2 * Ratio * LEDw);// specifies the region of interest in Rectangle form

	cropedRImg = CropIMG(gray, crop_region);

	/// start to Magic Wand function(AFF  operation)
	croppedAFFimg = loopAFF(cropedRImg, croppiccenter, coororder, newVal, lodiff, updiff);

	///customized Magic wand function
	//cropedFImg = CropIMG(fImg, crop_region);



	tie(LEDcoordinate,flag) = ISP(cropedRImg, croppedAFFimg, LEDh, LEDw, carx, cary, piccenter);

	outputLED[0] = (LEDcoordinate.x);
	outputLED[1] = (LEDcoordinate.y);


}

Point_<int> find_piccenter(Mat src) {
	int piccenterx = src.size().width * 0.5;
	int piccentery = src.size().height * 0.5;
	Point piccenter = Point(piccenterx, piccentery);
	return piccenter;
}

Mat floodfill(Mat src, int px, int py, int newVal, int lodiff, int updiff) {


	Mat fImg = src;
	Mat mask = Mat::zeros(fImg.rows + 2, fImg.cols + 2, CV_8UC1);

	Point seed = Point(px, py);
	Rect ccomp;
	floodFill(fImg, mask, seed, newVal, &ccomp, lodiff, updiff, FLOODFILL_FIXED_RANGE);

	mask.release();
	return fImg;

}

Mat CropIMG(Mat img, Rect size)
{
	Mat croppedIMG;
	img(size).copyTo(croppedIMG);
	return croppedIMG;

}

std::tuple<int, Point_<int>> FindMF_pixel(Mat histImg)
{
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(histImg, &minVal, &maxVal, &minLoc, &maxLoc);
	return { maxVal,maxLoc };
}

std::tuple<Mat, double, int, float> HistFeaturization(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff)
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


Mat loopAFF(Mat crop, Point_<int> piccenter, int coororder, int newVal, int lodiff, int updiff)
{
	Mat AFFoutput;
	double maxhistValue, standVal;
	int maxhistpixel;
	float frequentRatio;


	standVal = crop.cols * crop.rows * 0.5 * 0.5;
	tie(AFFoutput, maxhistValue, maxhistpixel, frequentRatio) = HistFeaturization(crop, piccenter, coororder, newVal, lodiff, updiff);
	//cout << "current coororder: " << coororder << endl;
	if (maxhistpixel != 0 || maxhistValue <= standVal)
	{
		coororder = coororder + 35;
		AFFoutput = loopAFF(crop, piccenter, coororder, newVal, lodiff, updiff);

	}
	//cout << "final coororder: " << coororder << endl;
	return AFFoutput;
}

void MyFilledCircle(Mat img, Point center)
{
	circle(img,
		center, //coordinate
		5, //radius
		Scalar(255, 255, 255), //color
		FILLED,
		LINE_AA //line type
	);
}

std::tuple<Point_<int>, bool>ISP(Mat bg, Mat fg, int LEDh, int LEDw, int carx, int cary, Point piccenter)
{
	//---------BG part---------------
	Mat ThresLow, ThresHigh, bgcom; //Img
	Mat kernelTLD, kernelTLE, kernelTHD; //Kernel
	Mat Threslow2, Threslow3, Threslow4; //check interPIC-ThresLow
	Mat Threshigh2, Threshigh3;			//check interPIC-ThresHigh
	Point anchor = Point(-1, -1);

	cv::medianBlur(bg, bg, 19);
	cv::GaussianBlur(bg, bg, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cv::equalizeHist(bg, bg);
	bg.copyTo(ThresHigh);

	bg.copyTo(ThresLow);
	//###	ThresLow ###
	//cv::adaptiveThreshold(src, dst,maxvalue, adaptiveMethod,thresholdType, blocksize, torr);
	cv::adaptiveThreshold(bg, ThresLow, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 2);
	cv::medianBlur(ThresLow, Threslow2, 35);
	kernelTLD = Mat::ones(Size(8, 16), CV_8UC1);
	cv::dilate(Threslow2, Threslow3, kernelTLD, anchor, 1, BORDER_CONSTANT, morphologyDefaultBorderValue());
	kernelTLE = cv::getStructuringElement(cv::MORPH_RECT, Size(1, 11));
	cv::erode(Threslow3, Threslow4, kernelTLE, anchor, 1, BORDER_CONSTANT, morphologyDefaultBorderValue());


	//imshow("ThresLow", ThresLow);

	//###	ThresHigh ###	
	cv::threshold(ThresHigh, Threshigh2, 230, 255, THRESH_BINARY);
	kernelTHD = Mat::ones(Size(8, 16), CV_8UC1);
	cv::dilate(Threshigh2, Threshigh3, kernelTHD, anchor, 1, BORDER_CONSTANT, morphologyDefaultBorderValue());
	bgcom = Threshigh3 + Threslow4;
	cv::medianBlur(bgcom, bgcom, 5);
	//imshow("Combine", bgcom);
	kernelTLD.release();
	kernelTLE.release();
	kernelTHD.release();




	//---------FG part---------------
	Mat FGtoBG, FGtoBG0; //Img
	Mat kernelFBC, kernelFBC2; //Kernel
	bgcom.copyTo(FGtoBG);
	//bgcom.copyTo(FGtoBG0);

	Mat newFGtoBG, FB01, FB02, newfg, FB03;
	fg.copyTo(newfg);
	cv::medianBlur(newfg, newfg, 5);
	cv::threshold(newfg, newfg, 1, 255, THRESH_BINARY);

	bgcom.copyTo(FGtoBG);

	for (int i = 0; i < newfg.rows; i++)
	{
		for (int j = 0; j < newfg.cols; j++)
		{
			if (newfg.at<uchar>(i, j) == 0) //picture1.at<uchar>(i,j) 就表示在第i行第j列的像素值
			{
				FGtoBG.at<uchar>(i, j) = newfg.at<uchar>(i, j);
				/*picture2.at<Vec3b>(i,j)[c]來表示RGB圖片某個通道中在(i,j)位置的像素值
				c取0就是B分量；c取1就是G分量；c取2就是R分量
				uchar、Vec3b表示图像元素的类型*/
			}
		}
	}


	int FGmode = 1;
	if (FGmode == 0) //fixed FF operation
	{
		cv::medianBlur(FGtoBG, FGtoBG, 17);
		kernelFBC = Mat::ones(Size(5, 5), CV_8UC1);
		cv::morphologyEx(FGtoBG, FGtoBG, cv::MORPH_DILATE, kernelFBC, anchor, 3, BORDER_CONSTANT, morphologyDefaultBorderValue());
		kernelFBC2 = cv::getStructuringElement(cv::MORPH_RECT, Size(5, 1));
		cv::erode(FGtoBG, FGtoBG, kernelFBC2, anchor, 2, BORDER_CONSTANT, morphologyDefaultBorderValue());
	}
	else if (FGmode == 1)
	{

		kernelFBC = getStructuringElement(MORPH_RECT, Size(9, 5), Point(2, 2));
		morphologyEx(FGtoBG, FGtoBG, MORPH_CLOSE, kernelFBC, Point(-1, -1), 2);
		kernelFBC2 = Mat::ones(Size(21, 1), CV_8UC1);
		cv::morphologyEx(FGtoBG, FGtoBG, cv::MORPH_DILATE, kernelFBC2, anchor, 1, BORDER_CONSTANT, morphologyDefaultBorderValue());






	}


	//imshow("FGtoBG", FGtoBG);
	kernelFBC.release();
	kernelFBC2.release();



	//---------Find contour---------------
	vector<vector<Point>> contours, reqContour, ERRcontour; // Vector for storing contour
	vector<Vec4i> hierarchy;
	Moments M;

	//vector<Point2i> centerRS, center; //LED center coordinate

	vector<Point2i> centerRS, center; //LED center coordinate

	Rect ret; //x,y,w,h
	vector<double> distance; //calculate the distance between piccenter and LED center
	Mat markIMG, ReqIMG, ISPimg;
	bool flag;
	int minIndex;
	float minvalue;
	Point_<float> Chipcenter = Point(0, 0);

	// Find the contours in the image
	findContours(FGtoBG, contours, hierarchy,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
	markIMG = Mat::zeros(FGtoBG.rows, FGtoBG.cols, CV_8UC1);
	cv::drawContours(markIMG, contours, -1, (255, 255, 255), 5, cv::LINE_AA);

	std::cout << "#######################33" << endl;
	std::cout << contours.size() << endl;
	std::cout << "#######################33" << endl;

	try
	{
		if (contours.size() == 0)
		{
			throw "something wrong...";
		}
		else
		{
			for (int i = 0; i < contours.size(); i++)
			{

				ret = cv::boundingRect(contours[i]);
				try
				{

					if (
						(ret.height / ret.width < 0.8) &&
						((ret.width - ret.height) / ret.width < 0.8) &&
						ret.height >= 0.7 * LEDh &&
						ret.width >= 0.7 * LEDw &&
						ret.width <= 3 * LEDw &&
						ret.height <= 3 * LEDh
						)
					{

						reqContour.push_back(contours[i]);
						M = (moments(contours[i], false));
						center.push_back(Point2i(M.m10 / M.m00, M.m01 / M.m00));
						centerRS.push_back(Point2i((M.m10 / M.m00) + carx, (M.m01 / M.m00) + cary));

						distance.push_back(norm(Point(piccenter) - centerRS[centerRS.size() - 1])); // get Euclidian distance

						/*cout << "coordinate-RS::" << centerRS[centerRS.size() - 1] << endl;
						cout << "distance ::: " << distance[distance.size() - 1] << endl;
						cout << "---------------------------" << endl;*/

					}
					else { ERRcontour.push_back(contours[i]); }
				}
				catch (...)
				{
					std::cout << "error: " << i << endl;
				}
			}






			ReqIMG = Mat::zeros(FGtoBG.rows, FGtoBG.cols, CV_8UC1);
			cv::drawContours(ReqIMG, reqContour, -1, (255, 255, 255), 5, cv::LINE_AA);
			//imshow("reqcontour", ReqIMG);

			//Find a LED coordinate with the shortest distance to the pic center
			auto it = std::min_element(distance.begin(), distance.end());
			minIndex = std::distance(distance.begin(), it);
			minvalue = *it;

			std::cout << "#########################" << endl;
			std::cout << minIndex << endl;
			std::cout << minvalue << endl;
			std::cout << "#########################" << endl;

			/*cout << "value of smallest element: " << minvalue << endl;
			cout << "index of smallest element: " << minIndex << endl;
			cout << "distance min" << distance[minIndex] << endl;
			cout << "++++++++++++++++++++++++++++++++++++++++++" << endl;*/
			/*cout << "LED coordinate is:" << centerRS[minIndex] << endl;
			cout << typeid(centerRS[minIndex]).name() << endl;
			cout << "---------------------------" << endl;*/

			ReqIMG.copyTo(ISPimg);



			MyFilledCircle(ISPimg, (piccenter - Point(carx, cary)));
			MyFilledCircle(ISPimg, ((centerRS[minIndex] - Point(carx, cary))));


			std::cout << "LED coordinate is:" << centerRS[minIndex] << endl;
			flag = true;
			Chipcenter = Point_<float>(centerRS[minIndex]);
		}
	}
	catch (const char* message)
	{
		std::cout << message << std::endl;
		flag = false;
	}


	ThresLow.release();
	ThresHigh.release();
	Threslow2.release();
	Threslow3.release();
	Threslow4.release();
	Threshigh2.release();
	Threshigh3.release();
	bgcom.release();
	FGtoBG.release();
	markIMG.release();
	ReqIMG.release();
	ISPimg.release();

	return { Chipcenter,flag };
}