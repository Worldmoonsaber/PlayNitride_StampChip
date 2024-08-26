
#include "MBstp_libV1.h"

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
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7040300 && picorder < 7040325)
	{
		W = 260;
		H = 126;
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 7180000 && picorder < 7189999)
	{
		W = 240;
		H = 126;
		maxw = 1.1;
		minw = 0.9;
		maxh =  1.1;
		minh =  0.7;
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
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
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

		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}

	else if (picorder > 7310200 && picorder < 7310222)
	{
		//horizontal
		W = 240; //240
		H = 126; //126
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder == 7310222)
	{
		//horizontal
		W = 126; //240
		H = 240; //126
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
		sizelist = vector<float>{ W, maxw, minw, H, maxh, minh };
	}
	else if (picorder > 8010200 && picorder < 8010299)
	{
		W = 240; //240
		H = 126; //126
		maxw =  1.1;
		minw =  0.9;
		maxh =  1.1;
		minh =  0.7;
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
			std::cout << "picture sequence: 0(C2G) / 1(Stp0704) / 2(S0718) / 3(Stp0718_bnormal) / 4(S0725) / 5(Stp0725_bnormal) / 6(Stp0731): "<<endl;
			std::cout << "picture sequence: 7(PN127_0412) / 8(PN96_0412) " << endl;
			std::cin >> picseq;

			if (picseq>-1 && picseq<10 )
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