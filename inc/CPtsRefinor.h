// CPtsRefinor.h: interface for the CPtsRefinor class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "CBasePara.h"

struct SORTDATA
{
    char strName[10]; //点名，排序的依据
    int nIndex; //点的序号
    SORTDATA()
    {
        nIndex = 0;
        memset(strName, 0, sizeof(strName));
    }
};

class CPtsRefinor
{
public:
    CPtsRefinor();
    virtual ~CPtsRefinor();

//operations
public:
    //从内定向数据文件中读取相片的大小，并得到格网坐标
    int ReadIOFile(string strIOFile);

    //从影像信息文件中读取数据，获得影像的相关信息，包括：影像ID，影像所在的航带号
    int ReadImageInforFile(string strImageInforFile);

    //从影像信息文件中读取数据，获得影像的相关信息，包括：影像ID，影像所在的航带号
    int ReadImageInforFile(string strImageInforFile, VEIMAGEINFOR& veImageInfor);

    //从点数据文件中读取点数据
    int ReadPtsFile(string strPtsFile);

    //对2度点按照一定规则进行删除
    /*
    1. 位于航线边缘上的2度点不能删除
    2. 位于不同航线上的2度点删除
    3. 格网内没有3度以上点的2度点不能删除
    */
    int RefinePts();

    //对2度点按照一定规则进行删除
    /*
    1. 位于航线边缘上的2度点不能删除
    2. 位于不同航线上的2度点删除
    3. 格网内没有3度以上点的2度点不能删除
    */
    int RefineAndDeletePts();


    //根据点所在相片以及坐标，确定它所在相片的格网是否有3度以上点
    int LocatePtPos(POINT2D& pt2D);

    //根据影像ID获得影像m_veImageInfor的索引号
    int GetImageIndexByImageID(int nImageID);

    //根据影像xy坐标获得点所在3*3格网的位置
    int GetGridIndexByxy(double x, double y, int& nx, int& ny);

    //将剔除后的点输出到文件中
    int WriteRefinedPts(string strOutFile);

    //生成分块数据目录
    int CreateDivDir(string strDir);

    //将数据进行分割
    int DivideData();

    //写影像信息文件
    int WriteImageInforFile(string strImageInforFile, VEIMAGEINFOR& veImageInfor);

    //将点数据输出到文件中
    int WritePtsInforFile(string strPtsInforFile, VEPTSINFOR& vePtsInfor);

    //确定一个点是单航带的点，还是交叉航带的点，返回0代表为单航带点，1代表交叉航带点
    int GetPtPos(PTSINFOR& pts);

    //按航带将数据分块
    int BlockData();

    //生成BA程序的工程文件，根据内定向路径、外定向路径、影像点文件数据，输出路径，生成在指定目录下的Proj文件
    //nPceMode：为处理的模式，0为前方交会，1为自由网
    //dThrRMS:为Threshold of RMS Value
    //dIgPtPc：为Image Point Precision X_1
    int CreateProjFile(string strIOFile, string strEOFile, string strImageFile, string strOutPath,
                       int nPcsMode, double dThrRMS, double dIgPtPc, string strProjPath);

    //根据log文件，判断是否处理成功, strLogFile文件路径
    //返回1，说明成功，0：失败
    int JudgeIsSuccess(string strLogFile);

    //读取像点残差文件
    //nStripID为航带编号，IsSgl是否为单航带的数据，1为是，0为航带间
    int ReadPtsResiduesFile(string strRsdFile, VEPTSINFOR& vePts);

    //排序比较函数
    static int compare(const SORTDATA& p1, const SORTDATA& p2);

    //读取log文件，并获得相应的结果信息
    //veSpan
    int GetRstInforFromLog(string strLogFile, VESPAN& veSpan);

    void ExtractID(string strRead,vector<int> &vecID);

    static int cmp(const int& a,const int& b)
    {
        return a < b;
    }

    static int cmp_pts(const PTS_ID_OP& a,const PTS_ID_OP& b)
    {
        //以增序列进行排序
        return a.nOP > b.nOP;
    }


    //根据PROJ_OPT写.proj文件
    //strBAProjFile为生成的文件路径
    //ProjOpt为平差配置文件
    int WriteBAProjFile(string strBAProjFile, PROJ_OPT& ProjOpt);

    //根据PROJ_OPT写.proj文件
    //strBAProjFile为生成的文件路径
    //ProjOpt为平差配置文件
    int ReadBAProjFile(string strBAProjFile, PROJ_OPT& ProjOpt);

    //根据给定的格网，分配空间
    int AllocateGridDist(VEIMAGEINFOR& veImageInfor, int nRow, int nCol);

    //释放格网数据
    int ReleaseGridDist(VEIMAGEINFOR& veImageInfor);

    //根据给定的格网（5*5），统计点的分布
    int StatisGridDist(VEIMAGEINFOR& veImageInfor, VEPTSINFOR& vePtsInfor, int nRow, int nCol);

    //根据影像ID获得影像veImageInfor的索引号
    int GetImageIndexByImageID(VEIMAGEINFOR& veImageInfor, int nImageID);

    //获取点所在的格网
    /*
    x, y为点的坐标
    XFormat, YFormat为影像的长度
    nx, ny为所在的格网
    nGridRow, nGridCol：统计格网的行列
    */
    int GetGridIndexByxy(double x, double y, double XFormat,
                         double YFormat, int nGridRow, int nGridCol, int& nx, int& ny);

    //根据点在格网的分布，取重叠度在前位的点，剔除其余点
    /*
    strRefPtsFile:为剔除不需要点的点文件路径
    veImageInfor：影像信息
    vePtsInfor: 点信息
    nRow, nCol: 格网的行列
    nSaveNum: 每个格网保留的个数,即重叠度在前nSaveNum的点要保留
    */
    int RefinePtsByGridInfor(string strRefPtsFile, VEIMAGEINFOR& veImageInfor,
                             VEPTSINFOR& vePtsInfor, int nRow, int nCol, int nSaveNum);

    //根据影像ID获取CamID
    int GetCamIndexByImgID(VEIMAGEINFOR& veImageInfor,int ImgID,int& nCamID);

    //点剔除主函数
    int PtsRefineMainFunc();

    //输出格网内缺点的信息
    int WriteGridInfor(string strGridInforFile,VEIMAGEINFOR& veImageInfor);


//attributes
public:
    VEPTSINFOR m_vePtsInfor; //所有点的信息
    VEIMAGEINFOR m_veImageInfor; //相片的信息

    //double m_dGridX[4]; //对相片划分格网的X坐标边界值
    //double m_dGridY[4]; //对相片划分格网的Y坐标边界值

    vector<int> m_ve2PtsIndex; //2度点的索引
    int m_nDeleteNum; //删除的点数
    int m_nLineNumber;
    int m_nStripNum;

    double m_dRMS; //单位权中误差

    vCamInfo m_vCam; //相机参数

    /*double m_dPixelSize; //像素大小
    double m_dXFormat; //x长度
    double m_dYFormat; //y长度*/

    /*double m_dOffsetx;
    double m_dOffsety;
    double m_dGridLenX;
    double m_dGridLenY;*/

    string m_strIOFile; //内方位元素文件路径
    string strPTSFile;	//像点文件名称
    string strPHTFile;	//方位元素文件名称
    string m_strEOPath; //方位元素文件路径，含文件名

    PROJ_OPT m_ProjOpt; //平差配置
    string m_strProjFile; //平差配置文件

    string m_strSaveDir; //分块保存文件路径
    string m_strFinalResPath; //最后整块数据的文件
    string m_strBAPath; //Bundle Adjustment 的路径

    //按航带划分的点信息数据
    MAPTSINFOR m_maSglPtsInfor;//单航带数据
    MAPTSINFOR m_maJntPtsInfor;//交叉航带数据

    bool m_bWeed2DegPts; //是否剔除2度点
    bool m_bFinalWeed; //最后是否剔除大于5倍中误差和5倍像素大小的点
    int test;


    int m_nGridRow; //统计格网的行数
    int m_nGridCol; //统计格网的列数
    int m_nSaveNum; //每个格网保留的点数
    string m_strRefPtsFile; //点剔除后的文件路径
    string m_strGridInforFile; //点剔除后的缺点的格网信息

    int isPTS;

};


