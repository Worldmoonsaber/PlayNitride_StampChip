#pragma once

#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include <chrono>
#include <fstream> //save as csv

using namespace cv;
using namespace std;

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

	/*double TDwidth;
	double TDmaxW;
	double TDminW;
	double TDheight;
	double TDmaxH;
	double TDminH;
	double WRmax;
	double WRmin;
	double HRmax;
	double HRmin;*/

	double TDwidth;
	double TDmaxW;
	double TDminW;
	double TDheight;
	double TDmaxH;
	double TDminH;

}sizeTD;

typedef struct
{
	int thresmode; //0:gray ; 1:RGB ; 2:HSV
	int bgmax[3];
	int bgmin[3];
	int fgmax[3];
	int fgmin[3];

}thresP;

typedef struct
{
	int PICmode; //0: read pic from path ; 1: read pic from pointer address
	int Outputmode; //0:center chip 
	int imgcols; //
	int imgrows; //
	double correctTheta;

}ImgP;

/*Input function*/
std::tuple<int, Mat> Inputfunction();
std::tuple < vector<float>, vector<int>> dict_rectregion(int picorder);


/*unused function*/
//void gammaCorrection(const Mat& src, Mat& dst, const float gamma);
//int findBoundary(Mat creteriaIMG, Rect inirect, char direction);
//Rect FindMaxInnerRect(Mat src, Mat colorSRC, sizeTD target);

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
