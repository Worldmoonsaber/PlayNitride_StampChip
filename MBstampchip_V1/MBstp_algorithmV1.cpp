#include "MBstp_libV1.h"




std::tuple<int, Mat, Point2f, Mat>STPchip_singlephase(float flag, Mat stIMG, thresP_ thresParm, sizeTD_ target, Point2f creteriaPoint, Point IMGoffset, SettingP_ chipsetting, double creteriaDist, ImgP_ imageParm)
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
	vector<vector<Point>>  Innercnt_ini,Innercnt_fin; // cnt for thresVal-filtering
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

			if (AFFarea > double(target.TDwidth * target.TDheight * 0.6*0.6) && AFFarea < double(target.TDwidth * target.TDheight * 20))
			{

				Rect AFFbox = cv::boundingRect(cntAFF[i]);
				cv::rectangle(AFFmskfi, AFFbox, Scalar(255, 255, 255), -1);
			}

		}

		//cntAFF.clear();

		cv::findContours(AFFmskfi, cntAFFfi,cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

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
			if (areamax > target.TDheight * target.TDwidth * 5 && interfereScnt.size()+1!= Innercnt_ini.size())//5 //Large interference contour
			{
				auto it = std::find(Arealist.begin(), Arealist.end(), areamax);
				int interfereIndex = std::distance(Arealist.begin(), it);
				
				
				cv::drawContours(acheckinner, Innercnt_ini, interfereIndex, Scalar(0, 0, 0), -1); //remove:Large interference contour
			}
			
			
			//--將 要濾除的Region 塗黑
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
						&& Areacheck>target.TDminH * target.TDheight* target.TDminW * target.TDwidth)
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
			

			if (Innercnt_fin.size() == 0 )
			{
				
				Innercnt_ini.clear();
				cv::findContours(acheckinner, Innercnt_ini,cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

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
							

							if (brokeninspect == false && contourArea(Innercnt_ini[0])>0.5*target.TDminH * target.TDminW * target.TDwidth * target.TDheight)
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

					if (areacomthres > target.TDheight * target.TDwidth * target.TDminH* target.TDminW && //0.5
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
								bdbox.y= bdbox.y + chipsetting.cary;
								cv::rectangle(marksize, bdbox, Scalar(0,0,255), 3);
								cv::rectangle(maskout, bdbox, Scalar(180, 180, 180), 2);
								/* find center:: */
								centercoord = Point(bdbox.x + 0.5 * bdbox.width + IMGoffset.x, bdbox.y + 0.5 * bdbox.height + IMGoffset.y)+ Point(chipsetting.carx, chipsetting.cary);
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