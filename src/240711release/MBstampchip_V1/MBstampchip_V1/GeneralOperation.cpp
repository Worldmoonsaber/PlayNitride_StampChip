#include "MBstp_libV1.h"


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


	minMaxLoc(Sechist,0, &secmaxVal, 0, &secmaxLoc);
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
		&& double(vechist[vechistnonzero[2]])/ double(vechist[vechistnonzero[1]])<0.3)
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


	
	cv::threshold(AFFimg, AFFthres, int((maxValAFF- histstdnum) * 0.7), 255, THRESH_BINARY); //0.7


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
		double stdlen,stdboundlen;
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
		if (minEllipse[minIAF].size.height < stdlen * 1.1 ) /*»Ý½T»{blur*/
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
