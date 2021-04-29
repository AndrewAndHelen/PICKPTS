#if !defined(BASE_PARA)
#define BASE_PARA 1000

#include <memory.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

/////////////////////////////////数据结构定义/////////////////////////////////////

struct POINT2D
{
    int nImageID; //点所在的航带ID
    double x, y; //xy坐标
    double rx, ry; //xy的残差
    int nGroupID;

    bool IsCoarse; //是否为粗差
    POINT2D()
    {
        nImageID = -1;
        x = y = 0; rx = ry = 0;
        nGroupID = 0;
        IsCoarse = false;//默认不是粗差
    }
};
typedef vector<POINT2D> VEPOINT2D;


struct CamInfo
{
    int CamID;
    double PixelSize; //像素大小
    double FormatX;
    double FormatY;

    CamInfo()
    {
        CamID = -1;
        PixelSize=0;
        FormatX=0;
        FormatY=0;
    }
};
typedef vector<CamInfo> vCamInfo;

struct PTSINFOR
{
    char strName[10];
    double X, Y, Z; //大地坐标
    int nAttrib;
    int nPtNum; //点数
    VEPOINT2D vePoints;
    bool isDelete; //是否应该删除
    int nIndex; //在所有点中的索引号，用于最后合并文件所用
    PTSINFOR()
    {
        memset(strName, 0, sizeof(strName));
        X = Y = Z = 0;
        nAttrib = 0;
        nPtNum = 0;
        vePoints.clear();
        isDelete = true;
    }
};
typedef vector<PTSINFOR> VEPTSINFOR;

typedef vector<VEPTSINFOR> MAPTSINFOR;

struct SPAN //对航带进行子航带进行平差的区间
{
    //其中-1代表该航带的一个影像，-2代表最后一张影像
    int nBIID; //起始影像ID
    int nEIID; //终止影像ID
    SPAN()
    {
        nBIID = nEIID = 0;
    }
};
typedef vector<SPAN> VESPAN;

struct PROJ_OPT
{
    string strTitle; //$Bundle Adjustment Project File
    string strProjectName; //$Project Name:
    string strUserName; //$User    Name:
    string strLsUdTime; //$Last Updated:

    string strIOFile; //Interior Orientation File   :
    string strEOFile; //Exterior Orientation File   :
    string strIgPsFile; //Image & Pass Point   File   :
    string strCtCkFile; //Control & Check Point File  :
    string strGPSPOSDataFile; //GPS or POS Data File        :
    string strBAOutPath; //Bundle Adjustment OutPath   :

    int nMinIter; //Minimum Iterations          :
    int nMaxIter; //Maximum Iterations
    int nRmGPSGross; //Remove GPS Gross_Error
    int nRmIMUGross; //Remove IMU Gross Error
    int nIgDataType; //Image Data Type for Adjust
    int nMinOlPP; //Min Overlaps of Pass Point
    int nWeightStragegy; //Weight Strategy
    int nRmIvCtPt; //Remove Invalid Control Point
    int nCBAdj; //Combined Bundle Adjustment
    int nDtIvIgPt; //Detect Invalid Image Point
    int nDtISGrossPt; //Detect IntStrip Gross Point
    int nCtFreeNetwork; //Construct Free Network
    int nAtomsRfCrt; //Atmos. Refraction Correction
    int nEarthCurCorrect; //Earth Curvature Correction
    int nGndCtDataUnits; //Ground Control Data Units
    double dStableCoeff; //Stable Coefficient
    double dThresRMS; //Threshold of RMS Value

    int nOutputCTRLPointResidues; //Output CTRL Point Residues
    int nOutputIO; //Output Interior Orientation
    int nOutputEO; //Output Exterior Orientation
    int nOutputSpacePointRes; //Output Space Point Result
    int nOutputAdjRes; //Output Adjustment Result
    int nOutputSysErrorRes; //Output System Error Result
    int nOutputImagePtResi; //Output Image Point Residues
    int nOutputGPSObsResi; //Output GPS Observ. Residues
    int nOutputIMUObsResi; //Output IMU Observ. Residues
    int nOutputIntermediateRes; //Output Intermediate Result
    int nOutputUnknowPrecision; //Output Unknow Precision
    int nOutputInterSectionAngle; //Output InterSectionAngle

    double dIgPtPrcX[5]; //Image Point Precision X_1~Image Point Precision X_5
    double dIgPtPrcY[5]; //Image Point Precision Y_1~Image Point Precision Y_5

    double dCTRLPtPcsXY[5]; //CTRL Point Precision XY_1~CTRL Point Precision XY_5
    double dCTRLPtPcsZ[5]; //CTRL Point Precision Z_1~CTRL Point Precision Z_5

    int nGPSOffsetCali[5]; //GPS Offset Calibration_1~GPS Offset Calibration_5
    int nGPSConstDriftCalib[5]; //GPS Const. Drift Calib_1~GPS Const. Drift Calib_5
    int nGPSLinearDriftCalib[5]; //GPS Linear Drift Calib_1~GPS Linear Drift Calib_5

    double dGPSDataPcsXY[5]; //GPS Data Precision XY_1~GPS Data Precision XY_5
    double dGPSDataPcsZ[5]; //GPS Data Precision Z_1~GPS Data Precision Z_5

    int nIMUOffsetCali[5]; //IMU Offset Calibration_1~IMU Offset Calibration_5
    int nIMUConstDriftCalib[5]; //IMU Const. Drift Calib_1~IMU Const. Drift Calib_5
    int nIMULinearDriftCalib[5]; //IMU Linear Drift Calib_1~IMU Linear Drift Calib_5

    double dIMUDataPcsPhi[5]; //IMU Data Precision Phi_1~IMU Data Precision Phi_5
    double dIMUDataPcsOmega[5]; //IMU Data Precision Omega_1~IMU Data Precision Omega_5
    double dIMUDataPcsKappa[5]; //IMU Data Precision Kappa_1~IMU Data Precision Kappa_5

    int nLensDistCaliF[5]; //Lens Distortion Calib F_1~Lens Distortion Calib F_5
    int nLensDistCaliU[5]; //Lens Distortion Calib U_1~Lens Distortion Calib U_5
    int nLensDistCaliV[5]; //Lens Distortion Calib V_1~Lens Distortion Calib V_5
    int nLensDistCaliK1[5]; //Lens Distortion Calib K1_1~Lens Distortion Calib K1_5
    int nLensDistCaliK2[5]; //Lens Distortion Calib K2_1~Lens Distortion Calib K2_5
    int nLensDistCaliP1[5]; //Lens Distortion Calib P1_1~Lens Distortion Calib P1_1
    int nLensDistCaliP2[5]; //Lens Distortion Calib P2_1~Lens Distortion Calib P2_1
    int nSysErrorEsti[5]; //System Error Estimation_1~System Error Estimation_1

    string strCoda; //$Bundle Adjustment Project File

    PROJ_OPT()
    {
        strTitle = "$Bundle Adjustment Project File"; //$Bundle Adjustment Project File
        strProjectName = "DEFAULT PROJECT"; //$Project Name:
        strUserName = "DEFAULT USER"; //$User    Name:
        strLsUdTime = "2006-11-13  22:13:14"; //$Last Updated:

        strIOFile; //Interior Orientation File   :
        strEOFile; //Exterior Orientation File   :
        strIgPsFile; //Image & Pass Point   File   :
        strCtCkFile; //Control & Check Point File  :
        strGPSPOSDataFile; //GPS or POS Data File        :
        strBAOutPath; //Bundle Adjustment OutPath   :

        nMinIter = 3; //Minimum Iterations          :
        nMaxIter = 10; //Maximum Iterations
        nRmGPSGross = 1; //Remove GPS Gross_Error
        nRmIMUGross = 1; //Remove IMU Gross Error
        nIgDataType = 1; //Image Data Type for Adjust
        nMinOlPP = 2; //Min Overlaps of Pass Point
        nWeightStragegy = 1; //Weight Strategy
        nRmIvCtPt = 1; //Remove Invalid Control Point
        nCBAdj = 1; //Combined Bundle Adjustment
        nDtIvIgPt = 0; //Detect Invalid Image Point
        nDtISGrossPt = 0; //Detect IntStrip Gross Point
        nCtFreeNetwork = 0; //Construct Free Network
        nAtomsRfCrt = 0; //Atmos. Refraction Correction
        nEarthCurCorrect = 0; //Earth Curvature Correction
        nGndCtDataUnits = 0; //Ground Control Data Units
        dStableCoeff = 0; //Stable Coefficient
        dThresRMS = 0.0052; //Threshold of RMS Value

        nOutputCTRLPointResidues = 1; //Output CTRL Point Residues
        nOutputIO = 1; //Output Interior Orientation
        nOutputEO = 1; //Output Exterior Orientation
        nOutputSpacePointRes = 1; //Output Space Point Result
        nOutputAdjRes = 1; //Output Adjustment Result
        nOutputSysErrorRes = 1; //Output System Error Result
        nOutputImagePtResi = 1; //Output Image Point Residues
        nOutputGPSObsResi = 1; //Output GPS Observ. Residues
        nOutputIMUObsResi = 1; //Output IMU Observ. Residues
        nOutputIntermediateRes = 1; //Output Intermediate Result
        nOutputUnknowPrecision = 0; //Output Unknow Precision
        nOutputInterSectionAngle = 0; //Output InterSectionAngle

        for (int i=0; i<5; i++)
        {
            dIgPtPrcX[i] = 0;
            dIgPtPrcY[i] = 0;

            dCTRLPtPcsXY[i] = 0;
            dCTRLPtPcsZ[i] = 0;

            nGPSOffsetCali[i] = 0;
            nGPSConstDriftCalib[i] = 0;
            nGPSLinearDriftCalib[i] = 0;

            dGPSDataPcsXY[i] = 0;
            dGPSDataPcsZ[i] = 0;

            nIMUOffsetCali[i] = 0;
            nIMUConstDriftCalib[i] = 0;
            nIMULinearDriftCalib[i] = 0;

            dIMUDataPcsPhi[i] = 0; //IMU Data Precision Phi_1~IMU Data Precision Phi_5
            dIMUDataPcsOmega[i] = 0; //IMU Data Precision Omega_1~IMU Data Precision Omega_5
            dIMUDataPcsKappa[i] = 0; //IMU Data Precision Kappa_1~IMU Data Precision Kappa_5

            nLensDistCaliF[i] = 0; //Lens Distortion Calib F_1~Lens Distortion Calib F_5
            nLensDistCaliU[i] = 0; //Lens Distortion Calib U_1~Lens Distortion Calib U_5
            nLensDistCaliV[i] = 0; //Lens Distortion Calib V_1~Lens Distortion Calib V_5
            nLensDistCaliK1[i] = 0; //Lens Distortion Calib K1_1~Lens Distortion Calib K1_5
            nLensDistCaliK2[i] = 0; //Lens Distortion Calib K2_1~Lens Distortion Calib K2_5
            nLensDistCaliP1[i] = 0; //Lens Distortion Calib P1_1~Lens Distortion Calib P1_1
            nLensDistCaliP2[i] = 0; //Lens Distortion Calib P2_1~Lens Distortion Calib P2_1
            nSysErrorEsti[i] = 0; //System Error Estimation_1~System Error Estimation_1
        }
        strCoda = "$Bundle Adjustment Project File\n"; //$Bundle Adjustment Project File
    }

};

struct PTS_ID_OP //点的index和overlop重叠度的信息结构体
{
    int nIndex; //点的索引号
    int nOP; //点的重叠度
    PTS_ID_OP()
    {
        nIndex = nOP = 0;
    }
};

typedef vector<PTS_ID_OP> VEPTS_ID_OP;

struct IMAGEINFOR
{
    int nImageID; //相片ID
    int nStripID; //航带ID
    bool isOnly2[3][3]; //3*3格网内是否只有2度点，默认为真
    bool isStripEdge; //是否位于航带的边缘，即位于航带头和尾的相片
    int nPos; //记录影像的位置信息，0为航带第一张影像，1为航带中间影像，2为航带末尾影像
    double Xs, Ys, Zs, phi, omega, kappa; //外方位元素
    int nAttrib;
    int nCameraID;
    int bFlag;

//	int* pGridDist; //点在指点格网上的点位分布
    int nGridCol; //列数
    int nGridRow; //行数

    VEPTS_ID_OP* pGridPtsInfor; //m*n格网点的信息，每个vector记录该格网拥有的点在VEPTSINFOR中的地址

    IMAGEINFOR()
    {
        nImageID = -1;
        nStripID = -1;
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                isOnly2[i][j] = true;

        isStripEdge = false;
        nAttrib = 0;
        nCameraID = 0;
        bFlag= 0;

//		pGridDist = NULL;
        nGridCol = nGridRow = 0;
        pGridPtsInfor = NULL;
        nPos = -1;

    }
};

typedef vector<IMAGEINFOR> VEIMAGEINFOR;
typedef vector<VEIMAGEINFOR> MAIMAGEINFOR;




#endif