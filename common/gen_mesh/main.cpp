
#include "genMesh.h"
#include "genMeshUtils.h"
#include "Utils.h"
#include "LdchApp.h"

#include <iostream>

#ifdef WIN32
#include <vld.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "rkisp_fisheye_c.h"

#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>

/* 测试FEC分级校正效果 */
void test1()
{
	int srcWidth, srcHeight;
	int dstWidth, dstHeight;

	/* 镜头参数 */
	CameraCoeff camCoeff;

	/* 4000x3000_DJI*/
	//srcWidth = 4000;
	//srcHeight = 3000;
	//dstWidth = 4000;
	//dstHeight = 3000;
	//camCoeff.a0 = -1717.3398540219218375568743795156478881835937500000000000000000000000000000000000000000000000000000000000;
	//camCoeff.a2 = 0.0000979924653478899483540626169641996057180222123861312866210937500000000000000000000000000000000000;
	//camCoeff.a3 = -0.0000000365259026624921297930806668007724935520741382788401097059249877929687500000000000000000000000;
	//camCoeff.a4 = 0.0000000000221078478569051180847769663747939545943299588515174036729149520397186279296875000000000000;
	//camCoeff.cx = 2035.6544000000001233274815604090690612792968750000000000000000000000000000000000000000000000000000000000;
	//camCoeff.cy = 1515.2591999999999643478076905012130737304687500000000000000000000000000000000000000000000000000000000000;

	/* 3840x2160_imx415_3.6mm */
	srcWidth = 3840;
	srcHeight = 2160;
	dstWidth = 3840;
	dstHeight = 2160;
	camCoeff.a0 = -2701.5553345375383287318982183933258056640625000000000000000000000000000000000000000000000000000000000000;
	camCoeff.a2 = 0.0000268969613364317018525725261346437378051632549613714218139648437500000000000000000000000000000000;
	camCoeff.a3 = 0.0000000691847147285347339282183675053528038034755809349007904529571533203125000000000000000000000000;
	camCoeff.a4 = -0.0000000000130627571989119925053445836944665276596611036552531004417687654495239257812500000000000000;
	camCoeff.cx = 1957.1773439999999482097337022423744201660156250000000000000000000000000000000000000000000000000000000000;
	camCoeff.cy = 1135.7418239999999514111550524830818176269531250000000000000000000000000000000000000000000000000000000000;

	/* 2688x1520_imx347_4mm */
	//srcWidth = 2688;
	//srcHeight = 1520;
	//dstWidth = 2688;
	//dstHeight = 1520;
	//camCoeff.a0 = -1863.7984212819360436697024852037429809570312500000000000000000000000000000000000000000000000000000000000;
	//camCoeff.a2 = 0.0004710038222292998403434305387804670317564159631729125976562500000000000000000000000000000000000000;
	//camCoeff.a3 = -0.0000002932338849422447377658846635883893583240933367051184177398681640625000000000000000000000000000;
	//camCoeff.a4 = 0.0000000001512588833120702854166806930133210712385860574613616336137056350708007812500000000000000000;
	//camCoeff.cx = 1363.0740479999999479332473129034042358398437500000000000000000000000000000000000000000000000000000000000;
	//camCoeff.cy = 787.0316799999999375359038822352886199951171875000000000000000000000000000000000000000000000000000000000;

	/* 2688x1520_OS04A10_4IR */
	//camCoeff.a0 = -1830.2636074851754983683349564671516418457031250000000000000000000000000000000000000000000000000000000000;
	//camCoeff.a2 = 0.0004237951284061040315917501253295540664112195372581481933593750000000000000000000000000000000000000;
	//camCoeff.a3 = -0.0000002507674855122118848025746917079770170744268398266285657882690429687500000000000000000000000000;
	//camCoeff.a4 = 0.0000000001272470035935948196069050985539572624916093701585850794799625873565673828125000000000000000;
	//camCoeff.cx = 1351.1178239999999277642928063869476318359375000000000000000000000000000000000000000000000000000000000000;
	//camCoeff.cy = 739.4860800000000153886503539979457855224609375000000000000000000000000000000000000000000000000000000000;

	/* 2688x1520_OS04A10_6IR */
	//camCoeff.a0 = -2653.9299617503911576932296156883239746093750000000000000000000000000000000000000000000000000000000000000;
	//camCoeff.a2 = 0.0001171174803799108549888854935439042037614854052662849426269531250000000000000000000000000000000000;
	//camCoeff.a3 = -0.0000000150166870224219489918825648283248752612450971355428919196128845214843750000000000000000000000;
	//camCoeff.a4 = 0.0000000000087465891629053077103521832658175594581856371334538380324374884366989135742187500000000000;
	//camCoeff.cx = 1333.2695039999998698476701974868774414062500000000000000000000000000000000000000000000000000000000000000;
	//camCoeff.cy = 728.1043200000000297222868539392948150634765625000000000000000000000000000000000000000000000000000000000;

	int srcSize = (srcWidth * srcHeight) > (dstWidth * dstHeight) ? (srcWidth * srcHeight) : (dstWidth * dstHeight);
	unsigned char *pImgY = new unsigned char[srcSize];
	unsigned char *pImgUV = new unsigned char[srcSize];
	unsigned char *pImgOut = new unsigned char[srcSize * 2];
	/* 读yuv文件 */
	char *srcYuvPath = "../data_in/imx415_3.6mm_3840x2160_001.nv12";
	readNV12(srcYuvPath, srcWidth, srcHeight, pImgY, pImgUV);

	/* 生成FEC映射表相关的参数 */
	FecParams fecParams;
	/* 初始化：根据图像输出分辨率，计算FEC映射表的相关参数，申请需要的buffer */
	genFecMeshInit(srcWidth, srcHeight, dstWidth, dstHeight, fecParams, camCoeff);

	unsigned short	*pMeshXI;		/* X整数部分 */
	unsigned char	*pMeshXF;		/* X小数部分 */
	unsigned short	*pMeshYI;		/* Y整数部分 */
	unsigned char	*pMeshYF;		/* Y小数部分 */
	/* FEC 4个mesh 内存申请 */
	mallocFecMesh(fecParams.meshSize4bin, &pMeshXI, &pMeshXF, &pMeshYI, &pMeshYF);

	Rkclock rkclock;
	bool success;
	int level = 0;// level范围: 0-255
	char dstYuvPath[256];
	for (level = 0; level <= 10; level++)
	{
		printf("level = %d\n", level);

		/* 生成不同校正程度的FEC模块的mesh表 */
		rkclock.clockStart();
		success = genFECMeshNLevel(fecParams, camCoeff, level, pMeshXI, pMeshXF, pMeshYI, pMeshYF);
		rkclock.clockEnd("genFECMeshNLevel");

		/* 调用FEC_Cmodel_4bin */
		FEC_Cmodel_4bin(srcWidth, srcHeight, dstWidth, dstHeight, pImgY, pImgUV, pMeshXI, pMeshXF, pMeshYI, pMeshYF, pImgOut);

		/* 保存 */
		sprintf(dstYuvPath, "../data_out/fec_out_%dx%d_%03d.nv12", dstWidth, dstHeight, level);
		saveNV12(dstYuvPath, dstWidth, dstHeight, pImgOut);
	}
	delete[] pImgY;
	delete[] pImgUV;
	delete[] pImgOut;
	/* FEC mesh 内存释放 */
	freeFecMesh(pMeshXI, pMeshXF, pMeshYI, pMeshYF);
	/* 反初始化 */
	genFecMeshDeInit(fecParams);

}

/* 测试LDCH分级校正效果 */
void test2()
{
	int srcWidth, srcHeight;
	int dstWidth, dstHeight;

	/* 镜头参数 */
	CameraCoeff camCoeff;

	/* 3840x2160_imx415_3.6mm_new */
	srcWidth = 3840;
	srcHeight = 2160;
	dstWidth = 3840;
	dstHeight = 2160;
	camCoeff.a0 = -2701.5553345375383287318982183933258056640625000000000000000000000000000000000000000000000000000000000000;
	camCoeff.a2 = 0.0000268969613364317018525725261346437378051632549613714218139648437500000000000000000000000000000000;
	camCoeff.a3 = 0.0000000691847147285347339282183675053528038034755809349007904529571533203125000000000000000000000000;
	camCoeff.a4 = -0.0000000000130627571989119925053445836944665276596611036552531004417687654495239257812500000000000000;
	camCoeff.cx = 1957.1773439999999482097337022423744201660156250000000000000000000000000000000000000000000000000000000000;
	camCoeff.cy = 1135.7418239999999514111550524830818176269531250000000000000000000000000000000000000000000000000000000000;

	int srcSize = (srcWidth * srcHeight) > (dstWidth * dstHeight) ? (srcWidth * srcHeight) : (dstWidth * dstHeight);
	unsigned short *pImgIn = new unsigned short[srcSize * 3];
	unsigned short *pImgOut = new unsigned short[srcSize * 3];

	/* 读BGR图像 */
	char *srcBGRPath = "../data_in/imx415_3.6mm_3840x2160_001.nv12";
	cv::Mat imgBGR = cv::imread("../data_in/imx415_3.6mm_3840x2160_001.jpeg");
	unsigned char *pIn = imgBGR.data;
	int stride = srcWidth * 3;
	int index = 0;
	for (int y = 0; y < imgBGR.rows; y++)
	{
		for (int x = 0; x < imgBGR.cols; x++)
		{
			pImgIn[index + 0] = *(pIn + y * stride + 3 * x + 0);
			pImgIn[index + 1] = *(pIn + y * stride + 3 * x + 1);
			pImgIn[index + 2] = *(pIn + y * stride + 3 * x + 2);
			index = index + 3;
		}
	}

	/* 生成LDCH映射表相关的参数 */
	LdchParams ldchParams;
	/* 初始化：根据图像输出分辨率，计算LDCH映射表的相关参数，申请需要的buffer */
	genLdchMeshInit(srcWidth, srcHeight, dstWidth, dstHeight, ldchParams, camCoeff);
	unsigned short *pMeshX = new unsigned short[ldchParams.meshSize];

	Rkclock rkclock;
	bool success;
	int level = 0;// level范围: 0-255
	char dstYuvPath[256];
	for (level = 0; level <= 10; level++)
	{
		printf("level = %d\n", level);

		rkclock.clockStart();
		success = genLDCMeshNLevel(ldchParams, camCoeff, level, pMeshX);
		rkclock.clockEnd("genLDCMeshNLevel");

		/* 调用LDCH_Cmodel */
		LDCH_Cmodel(dstWidth, dstHeight, pImgIn, pImgOut, pMeshX);

		/* 保存 */
		sprintf(dstYuvPath, "../data_out/ldch_out_%dx%d_%03d.bmp", dstWidth, dstHeight, level);
		int imgBit = 8;
		SaveBmpFile2(dstYuvPath, dstWidth, dstHeight, imgBit, pImgOut);

	}

	delete[] pImgIn;
	delete[] pImgOut;
	delete[] pMeshX;
	genLdchMeshDeInit(ldchParams);
}

int main()
{
	test1();

#ifdef WIN32
	system("pause");
#endif
	return 0;
}