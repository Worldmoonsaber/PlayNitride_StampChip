#include "MBstp_libV1.h"


std::tuple < vector<float>, vector<int>> dict_rectregion(int picorder);
std::tuple<int, Mat> Inputfunction();

int main()
{
	//declare parameters::
	SettingP_ chipsetting;
	thresP_ thresParm;
	ImgP_ imageParm;
	sizeTD_ target;


	/////I/O parameters setting:::
	
	imageParm.imgcols = 1300; //4000
	imageParm.imgrows = imageParm.imgcols;


	imageParm.Outputmode = 0; 
	imageParm.PICmode = 0;
	chipsetting.interval[0] = 1; //2
	chipsetting.xpitch[0] = 400; //400
	chipsetting.carx = 0;
	chipsetting.cary = 0;
	



	/////////////////////////////////////////
	Point2f creteriaPoint;
	Mat rawimg, cropedRImg, gauBGR;
	int picorder;
	Point IMGoffset;




	//output parameters::
	Mat ReqIMG, marksize;
	double creteriaDist = double(chipsetting.xpitch[0]);
	Point2f crossCenter;
	float boolflag = 0;


	//rawimg = imread("C:\\Image\\StampChip\\Cplus\\Stp0718_abnormal\\7180110.bmp");
		// Image source input: IMG format:RGB
		try
		{
			std::tie(picorder, rawimg) = Inputfunction();
			if (rawimg.empty())
			{
				boolflag = 8.0;
				throw "something wrong::input image failure";
			} //check if image is empty

		} //try loop
		catch (const char* message)
		{

			std::cout << "check catch state:: " << boolflag << endl;


		}//catch loop


		/////
		vector<float> sizelist;
		vector<int>threslist;


		
		std::tie(sizelist, threslist) = dict_rectregion(picorder);

		threslist.clear();

		target.TDwidth = sizelist[0];
		target.TDmaxW = sizelist[1];
		target.TDminW = sizelist[2];

		target.TDheight = sizelist[3];
		target.TDmaxH = sizelist[4];
		target.TDminH = sizelist[5];

		
		
		thresParm = { 4,{555,99999,99999},{99999,99999,99999} ,{6,99999,99999}, {99999,99999,99999} }; 
		//{mode,bgmax,bgmin,fgmax,fgmin}


		


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
			if (thresParm.bgmax[imageParm.PICmode]< thresParm.fgmax[imageParm.PICmode])
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



		if (boolflag == 0)
		{
			/*image with CROP  process :::*/
			Point piccenter;
			piccenter = find_piccenter(rawimg);
			//piccenter = Point(2489, 3769); Lchip
			
			IMGoffset.x = piccenter.x - int(imageParm.imgcols * 0.5);  //2736-600*0.5=2476
			IMGoffset.y = piccenter.y - int(imageParm.imgrows * 0.5);  //1824-600*0.5=1564
			Rect Cregion(IMGoffset.x, IMGoffset.y, imageParm.imgcols, imageParm.imgrows);
			cropedRImg = CropIMG(rawimg, Cregion);

			///*///*image without CROP  process :::*/
			/*IMGoffset.x=0;
			IMGoffset.y=0;
			rawimg.copyTo(cropedRImg);*/




			
			creteriaPoint = find_piccenter(cropedRImg);


			

			//start to ISP//////////////////////////
			std::tie(boolflag, ReqIMG, crossCenter, marksize) = STPchip_singlephase(boolflag, cropedRImg, thresParm, target, creteriaPoint, IMGoffset, chipsetting, creteriaDist, imageParm);
		}

		std::cout << "check img state:: " << boolflag << endl;
		std::cout << "check center is ::" << crossCenter << endl;
	
	
	return 0;
}


std::tuple < vector<float>, vector<int>> dict_rectregion(int picorder)
{
	vector<float> sizelist;
	vector<int> threslist;



	if (picorder > 7040101 && picorder < 7040116)
	{
		threslist = vector<int>{ 35, 99999 };
	}
	else if (picorder > 7040300 && picorder < 7040325)
	{
		threslist = vector<int>{ 90, 99999 };//65 //50
	}
	else if (picorder > 7180000 && picorder < 7189999)
	{
		threslist = vector<int>{ 60, 99999 };//65 //40 //50
	}
	else if (picorder > 7250000 && picorder < 7259999)
	{
		threslist = vector<int>{ 60, 99999 };//60 //60
	}
	else if (picorder > 7310000 && picorder < 7319999)
	{
		threslist = vector<int>{ 60, 99999 };//60
	}
	else if (picorder > 8010200 && picorder < 8010299)
	{
		threslist = vector<int>{ 60, 99999 };//60
	}
	else if (picorder > 8010300 && picorder < 8010399)
	{
		threslist = vector<int>{ 70, 99999 };//60
	}
	else
	{
		threslist = vector<int>{ 60, 99999 }; //60
	}

	float maxw, minw, maxh, minh, W, H;

	if (picorder > 7040101 && picorder < 7040116)
	{
		W = 237;
		H = 116;
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7040300 && picorder < 7040325)
	{
		W = 260;
		H = 126;
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7180000 && picorder < 7189999)
	{
		W = 240;
		H = 126;
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7250000 && picorder < 7250299)
	{
		//horizontal
		W = 240; //240
		H = 126;
		/*maxw =  1.1;
		minw =  0.9;
		maxh = 1.1;
		minh =0.7;*/
		maxw = 1.3;
		minw = 0.7;
		maxh = 1.3;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7250300 && picorder < 7250317)
	{
		//horizontal
		W = 254;
		H = 126;
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7250319 && picorder < 7250322)
	{
		//horizontal
		/*W = 240;
		H = 126;*/

		//vertical
		W = 126;
		H = 240;

		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}

	else if (picorder > 7310200 && picorder < 7310222)
	{
		//horizontal
		W = 240; //240
		H = 126; //126
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder == 7310222)
	{
		//horizontal
		W = 126; //240
		H = 240; //126
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 8010200 && picorder < 8010299)
	{
		W = 240; //240
		H = 126; //126
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 8010300 && picorder < 8010399)
	{
		W = 126; //240
		H = 254; //126
		maxw = 1.1;
		minw = 0.9;
		maxh = 1.1;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 41199 && picorder < 41250)
	{
		/*P096*/
		//W = 246; //240
		//H = 122; //126

		/*P127*/
		W = 236;
		H = 114;

		maxw = 1.3;
		minw = 0.7;
		maxh = 1.3;
		minh = 0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}

	else
	{
		sizelist = vector<float>{ 260, 1.1, 0.9, 134, 1.1, 0.7 };
	}


	return{ sizelist,threslist };
}
std::tuple<int, Mat> Inputfunction()
{
	int picorder, picmode, picseq;

	bool valid = false;
	Mat Rawpic;

	string picseqNAME = "";

	do
	{

		do
		{
			std::cout << "picture sequence: 0(C2G) / 1(Stp0704) / 2(S0718) / 3(Stp0718_bnormal) / 4(S0725) / 5(Stp0725_bnormal) / 6(Stp0731): " << endl;
			std::cout << "picture sequence: 7(PN127_0412) / 8(PN96_0412) " << endl;
			std::cin >> picseq;

			if (picseq > -1 && picseq < 10)
			{
				valid = true;

				picseqNAME = "/";



			}
			else
			{
				std::cout << "picture sequence: 0(C2G) / 1(Stp0704) / 2(S0718) / 3(Stp0718_bnormal) / 4(S0725) / 5(Stp0725_bnormal) / 6(Stp0731): " << endl;
				std::cout << "picture sequence: 7(PN127_0412) / 8(PN96_0412) " << endl;
				std::cin >> picseq;
			}
		} while (!valid);


		if (picseqNAME != "" && picseq == 0)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/TestpicC2G" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image
		}
		else if (picseqNAME != "" && picseq == 1)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0704" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 2)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0718" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 3)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0718_abnormal" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 4)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0725" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 5)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0725_abnormal" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 6)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread("C:/Image/StampChip/Cplus/Stp0731" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 7)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread(R"x(C:/Image/StampChip/Cplus\PN127_240412)x" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else if (picseqNAME != "" && picseq == 8)
		{
			std::cout << "Enter the number of the picture:";
			std::cin >> picorder;
			Rawpic = imread(R"x(C:/Image/StampChip/Cplus\PN96_240412)x" + picseqNAME + string(std::to_string(picorder)) + ".bmp", IMREAD_GRAYSCALE);//loading the image

		}
		else
		{
			std::cout << "Error picture sequence" << endl;
		}




	} while (!valid);

	return { picorder ,Rawpic };

}











/*unused function:::*/
/*

	
Mat gammimg;
float alpha = 4; //1.5
float beta = -20;
Mat dstimg = Mat::zeros(Gimg.size(), Gimg.type());

for (int row = 0; row < stIMG.rows; row++)
{
	for (int col = 0; col < stIMG.cols; col++)
	{
		float pxl = stIMG.at<uchar>(row, col);
		dstimg.at<uchar>(row, col) = saturate_cast<uchar>(pxl * alpha + beta);
	}
}
gammaCorrection(stIMG, gammimg, 1.1);
	



void gammaCorrection(const Mat& src, Mat& dst, const float gamma)
{
	float invGamma = 1 / gamma;

	Mat table(1, 256, CV_8U);
	uchar* p = table.ptr();
	for (int i = 0; i < 256; ++i) {
		p[i] = (uchar)(pow(i / 255.0, invGamma) * 255);
	}

	LUT(src, table, dst);
}


int findBoundary(Mat creteriaIMG, Rect inirect, char direction)
{
	int step = 1;
	auto findRecr = inirect;
	int BoundaryVal;

	switch (direction)
	{
	case 'L':
		while (true)
		{
			//const auto count = cv::countNonZero(inverted_mask(inside_rect));
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.x;
				break;
			}
			findRecr.x -= step;
		}
		break;
	case 'T':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.y;
				break;
			}
			findRecr.y -= step;
		}
		break;
	case 'R':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.x;
				break;
			}
			findRecr.x += step;
		}
		break;

		break;
	case 'B':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.y;
				break;
			}
			findRecr.y += step;
		}
		break;

	default:
		std::cout << "****** Error case mode ******" << endl;
		break;

	}


	std::cout << "finish findboundary~" << endl;
	std::cout << "fi";
	return BoundaryVal;
}

Rect FindMaxInnerRect(Mat src,Mat colorSRC, sizeTD target)
{
	//output:::
	Rect innerboundary;
	//
	//find inner rect:
	Size ksize;
	Mat src2;
	if (target.TDheight > target.TDwidth)
	{
		ksize = Size(int(15 * target.TDwidth / 126), int(15 * target.TDheight / 240));
	}
	else
	{
		ksize = Size(int(15 * target.TDwidth / 240), int(15 * target.TDheight / 126));
	}
	Mat Kcomclose = Mat::ones(ksize, CV_8UC1);
	cv::morphologyEx(src, src2, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 4);





	//Step.NEW7-find inner rect (via tiny scanning mechanism) ::
	cv::Mat inverted_mask;
	cv::bitwise_not(src2, inverted_mask);
	cv::Mat pointsmsk = Mat::zeros(src.size(), CV_8UC1);;
	cv::findNonZero(src2, pointsmsk);
	const cv::Rect outside_rect = cv::boundingRect(pointsmsk);

	int step_w, step_h;
	if (outside_rect.width > outside_rect.height)
	{
		step_w = 1;//2
		step_h = 1;//1
	}
	else
	{
		step_w = 1;//1
		step_h = 1;//2
	}


	auto inside_rect = outside_rect;


	while (true)
	{
		//const auto count = cv::countNonZero(inverted_mask(inside_rect));
		const auto count = cv::countNonZero(inverted_mask(inside_rect));


		if (count == 0)
		{
			// we found a rectangle we can use!
			break;
		}

		inside_rect.x += step_w;
		inside_rect.y += step_h;
		inside_rect.width -= (step_w * 2);
		inside_rect.height -= (step_h * 2);
	}

	Mat TDrect = Mat::zeros(src.size(), CV_8UC1);
	src.copyTo(TDrect);
	cv::rectangle(TDrect, outside_rect, Scalar(180, 180, 180), 1); //Scalar(0, 0, 0)
	cv::rectangle(TDrect, inside_rect, Scalar(100, 100, 100), 1); //Scalar(0, 0, 0)
	
	std::cout << "check inside rect:: " << inside_rect << endl;
	std::cout << "check outside_rect:: " << outside_rect << endl;

	//Step.NEW8-find inner rect boundary ::
	Rect line = Rect(inside_rect.x, inside_rect.y, inside_rect.width, 1);
	//cv::rectangle(gamimg, line, Scalar(0, 0, 0), 1); //Scalar(0, 0, 0)
	const auto count = cv::countNonZero(inverted_mask(line));
	std::cout << "999999999999999999999999999999999: " << count << endl;
	int leftBound;
	Rect Leftline = Rect(inside_rect.x, inside_rect.y, 1, inside_rect.height); //360,355 
	leftBound = findBoundary(inverted_mask, Leftline, 'L');
	std::cout << "check left boundary " << leftBound << endl;


	int topBound;
	Rect Topline = Rect(inside_rect.x, inside_rect.y, inside_rect.width, 1);
	topBound = findBoundary(inverted_mask, Topline, 'T');
	std::cout << "check Top boundary " << topBound << endl;

	int RightBound;
	Rect Rightline = Rect(inside_rect.x + inside_rect.width, inside_rect.y, 1, inside_rect.height);
	RightBound = findBoundary(inverted_mask, Rightline, 'R');
	std::cout << "check right boundary " << RightBound << endl;

	int BottomBound;
	Rect bottomline = Rect(inside_rect.x, inside_rect.y + inside_rect.height, inside_rect.width, 1);
	BottomBound = findBoundary(inverted_mask, bottomline, 'B');
	std::cout << "check bottom boundary " << BottomBound << endl;

	innerboundary = Rect(leftBound, topBound, (RightBound - leftBound), (BottomBound - topBound));

	//Step.NEW9-Mark inner rect::
	cv::rectangle(colorSRC, innerboundary, Scalar(0, 0, 255), 1);
	cv::rectangle(TDrect, innerboundary, Scalar(50, 50, 50), 2);



	std::cout << "fini" << endl;

	//
	return innerboundary;

}
*/








