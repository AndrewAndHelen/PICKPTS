// CPtsRefinor.cpp: implementation of the CPtsRefinor class.
//
//////////////////////////////////////////////////////////////////////


#include "CPtsRefinor.h"
#include "Win2Linux.h"
#include <algorithm>
#include <search.h>
#include <iostream>
#include <cstdio>
#include "math.h"
using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPtsRefinor::CPtsRefinor()
{

}

CPtsRefinor::~CPtsRefinor()
{
    int i = 0;
    for (i=0; i<m_maJntPtsInfor.size(); i++)
    {
        m_maJntPtsInfor[i].clear();
        m_maSglPtsInfor[i].clear();
    }

    m_maSglPtsInfor.clear();
    m_maJntPtsInfor.clear();
    m_veImageInfor.clear();
    m_ve2PtsIndex.clear();
    m_vePtsInfor.clear();
}

//从内定向数据文件中读取相片的大小，并得到格网坐标
int CPtsRefinor::ReadIOFile(string strIOFile)
{
    m_strIOFile = strIOFile;

    FILE* fp = fopen (m_strIOFile.c_str(), "r");
    if ( !fp ) return 0;

    char buf[512];
    fgets (buf, 512, fp);
    fgets (buf, 512, fp);
    fgets (buf, 512, fp);
    fgets (buf, 512, fp);
    int ncmr;
    sscanf (buf, "%d", &ncmr);

    double x0, y0, f, formatx, formaty, pixelsize, k0, k1, k2, k3, p1, p2, b1, b2;
    int cmridx, attrib;
    int tmp;
    for ( int i = 0; i < ncmr; i++ )
    {
        fgets(buf, 512, fp);
        tmp = sscanf(buf, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d", &cmridx,
                     &x0, &y0, &f, &formatx, &formaty, &pixelsize,
                     &k0, &k1, &k2, &k3, &p1, &p2, &b1, &b2, &attrib); //新版本
        if (tmp == 16) {//新版本
            CamInfo Cam;
            Cam.CamID = cmridx;
            Cam.PixelSize = pixelsize;
            Cam.FormatX = formatx;
            Cam.FormatY = formaty;
            m_vCam.push_back(Cam);
        }
    }

    fclose(fp);
//    int id;
//    double x0, y0, f, format_X, format_Y, ps,k1, k2, p1, p2, Attrib;
//    for (int j = 0; j < n; j++)
//    {
//        STD.ReadString(strRead);
//        sscanf(strRead, "%d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
//               &id, &x0, &y0, &f, &format_X, &format_Y, &ps, &k1, &k2, &p1, &p2, &Attrib);
//
//        CamInfo Cam;
//        Cam.CamID = id;
//        Cam.PixelSize = ps;
//        Cam.FormatX = format_X;
//        Cam.FormatY = format_Y;
//
//        m_vCam.push_back(Cam);
//    }
//    STD.Close();

    return 1;
}

//从影像信息文件中读取数据，获得影像的相关信息，包括：影像ID，影像所在的航带号
int CPtsRefinor::ReadImageInforFile(string strImageInforFile)
{
    m_strEOPath = strImageInforFile;

    char Infor[512];
    char Infor_[512];
    {
        char szdrive_[_MAX_DRIVE];
        char szdir_[_MAX_DIR];
        char szfname_[_MAX_FNAME];
        char szext_[_MAX_EXT];
        _splitpath(m_strEOPath.c_str(), szdrive_, szdir_, szfname_, szext_);
        sprintf(Infor, "%s%s", szfname_, szext_);
        sprintf(Infor_, "%s.pts", szfname_);
    }
    strPHTFile = Infor;
    strPTSFile = Infor_;

    FILE *f = fopen (m_strEOPath.c_str(), "r");
    if ( !f )
    {
        return 0;
    }

    char buf[1024], tmp[256];
    string POSNAME;

    fgets (buf, 1024, f);
    fgets (buf, 1024, f);
    fgets (buf, 1024, f);
    fgets (buf, 1024, f);
    int nImNum; //相片个数
    sscanf(buf, "%d", &nImNum);
    m_veImageInfor.clear();
    m_veImageInfor.resize(nImNum);
    int nImageID, nStripID, nAttrib, nCameraID, bFlag;
    double Xs, Ys, Zs, Phi, Omega, Kappa;
    int nLastStripID = -1; //上一个记录的StripID
    m_nStripNum = 0;

    for (int i=0; i<nImNum; i++)
    {
        fgets (buf, 1024, f);
        sscanf(buf, "%d %lf %lf %lf %lf %lf %lf %d %d %d %d",
               &nImageID, &Xs, &Ys, &Zs, &Phi, &Omega, &Kappa, &nStripID, &nAttrib, &nCameraID, &bFlag);

        if(isPTS == 0) nImageID = i;

        m_veImageInfor[i].nImageID = nImageID;
        m_veImageInfor[i].nStripID = nStripID;
        m_veImageInfor[i].Xs = Xs;
        m_veImageInfor[i].Ys = Ys;
        m_veImageInfor[i].Zs = Zs;
        m_veImageInfor[i].phi = Phi;
        m_veImageInfor[i].omega = Omega;
        m_veImageInfor[i].kappa = Kappa;
        m_veImageInfor[i].nAttrib = nAttrib;
        m_veImageInfor[i].nCameraID = nCameraID;
        m_veImageInfor[i].bFlag = bFlag;

        if (nLastStripID != nStripID) //当前StripID不等于上一个记录的StripID，则说明为航带边缘相片
        {
            m_nStripNum ++;
            m_veImageInfor[i].isStripEdge = true;
            m_veImageInfor[i].nPos = 0; //航带第一张影像
            if (nLastStripID != -1) //当前相片不是第一个，则前一个相片为上一条航带的最后一张相片，也为边缘相片
            {
                m_veImageInfor[i-1].isStripEdge = true;
                m_veImageInfor[i-1].nPos = 2; //航带末尾影像
            }

        }
        else
        {
            m_veImageInfor[i].nPos = 1; //航带中间的影像
        }
        nLastStripID = nStripID;

        //3*3格网内是否只有2度点，默认为真
        for (int j1=0; j1<3; j1++)
            for (int j2=0; j2<3; j2++)
                m_veImageInfor[i].isOnly2[j1][j2] = true;

    }
    m_veImageInfor[nImNum-1].isStripEdge = true;
    m_veImageInfor[nImNum-1].nPos = 2;
    fclose(f);

    return 1;
}


//从点数据文件中读取点数据
int CPtsRefinor::ReadPtsFile(string strPtsFile)
{

    FILE* fp = fopen (strPtsFile.c_str(), "r");
    if ( !fp ) return 0;
    char buf[512];

    if(isPTS == 1)
    {
        fgets (buf, 512, fp);
        fgets (buf, 512, fp);
        fgets (buf, 512, fp);
        fgets (buf, 512, fp);
        fgets (buf, 512, fp);
    }
    int nPtsNum; //像点个数
    fgets (buf, 512, fp);
    sscanf(buf, "%d %d", &nPtsNum, &m_nLineNumber);
    m_vePtsInfor.clear();
    m_vePtsInfor.resize(nPtsNum);
    m_ve2PtsIndex.clear();

    for (int i=0; i<nPtsNum; i++)
    {
        PTSINFOR& tmp = m_vePtsInfor[i];
        fgets (buf, 512, fp);
        sscanf(buf, "%s %lf %lf %lf %d",
               tmp.strName, &tmp.X, &tmp.Y, &tmp.Z, &tmp.nAttrib);
        fgets (buf, 512, fp);

        sscanf(buf, "%d", &tmp.nPtNum); //像点个数
        tmp.vePoints.resize(tmp.nPtNum);
        if (tmp.nPtNum == 2) //2度点
            m_ve2PtsIndex.push_back(i);

        for (int j = 0; j < tmp.nPtNum; j++) //读取每个像点坐标和航带数
        {
            POINT2D& tmp1 = tmp.vePoints[j];
            fgets (buf, 512, fp);
            if(isPTS==1)
                sscanf(buf, "%d %lf %lf %lf %d",
                       &tmp1.nImageID, &tmp1.x, &tmp1.y, &tmp1.nGroupID);
            else
                sscanf(buf, "%d %lf %lf %lf %d",
                       &tmp1.nImageID, &tmp1.x, &tmp1.y);
            if (tmp.nPtNum != 2) //非2度点
            {
                //确定所在影像格网有3度点
                LocatePtPos(tmp1);
            }
        }
    }
    fclose(fp);
    return 1;
}

//根据点所在相片以及坐标，确定它所在相片的格网是否有3度以上点
int CPtsRefinor::LocatePtPos(POINT2D& pt2D)
{
    int nx, ny;// 格网编号，nx：x方向，ny：y方向
    nx = ny = 0;
    int nImIndex;
    nImIndex = GetImageIndexByImageID(pt2D.nImageID);
    GetGridIndexByxy(pt2D.x, pt2D.y, nx, ny);
    m_veImageInfor[nImIndex].isOnly2[ny][nx] = false;

    return 1;
}

//对2度点按照一定规则进行删除
/*
1. 位于航线边缘上的2度点不能删除
2. 位于不同航线上的2度点删除
3. 格网内没有3度以上点的2度点不能删除
*/
int CPtsRefinor::RefinePts()
{
    int i;
    int nDeleteNum = 0;
    for (i=0; i<m_ve2PtsIndex.size(); i++)
    {
        PTSINFOR& TmpPts = m_vePtsInfor[m_ve2PtsIndex[i]];
        TmpPts.isDelete = true;

        //2度点只有两个影像点
        POINT2D& pt2D1 = TmpPts.vePoints[0];
        POINT2D& pt2D2 = TmpPts.vePoints[1];
        int nImIndex1, nImIndex2;
        nImIndex1 = GetImageIndexByImageID(pt2D1.nImageID);
        nImIndex2 = GetImageIndexByImageID(pt2D2.nImageID);
        //该点在不同航线上，应删除
        if (m_veImageInfor[nImIndex1].nStripID != m_veImageInfor[nImIndex2].nStripID)
        {
            TmpPts.isDelete = true;
            nDeleteNum++;
            continue;
        }

        //该点在航线边缘影像上，不可删除
        if (m_veImageInfor[nImIndex1].isStripEdge == true ||
            m_veImageInfor[nImIndex2].isStripEdge == true )
        {
            TmpPts.isDelete = false;
            continue;
        }

        int nx1, ny1, nx2, ny2;
        GetGridIndexByxy(pt2D1.x, pt2D1.y, nx1, ny1);
        GetGridIndexByxy(pt2D2.x, pt2D2.y, nx2, ny2);
        //有一个点所在格网只有2度点，因此不能删除
        if (m_veImageInfor[nImIndex1].isOnly2[ny1][nx1] == true ||
            m_veImageInfor[nImIndex2].isOnly2[ny2][nx2] == true )
        {
            TmpPts.isDelete = false;
            continue;
        }
        nDeleteNum++;
    }
    m_nDeleteNum = nDeleteNum;
    return 1;
}

//对2度点按照一定规则进行删除
/*
1. 位于航线边缘上的2度点不能删除
2. 位于不同航线上的2度点删除
3. 格网内没有3度以上点的2度点不能删除
*/
int CPtsRefinor::RefineAndDeletePts()
{
    int i;
    int nDeleteNum = 0;
    VEPTSINFOR vePtsInfor = m_vePtsInfor;
    m_vePtsInfor.clear();

    for (i=0; i<m_ve2PtsIndex.size(); i++)
    {
        PTSINFOR& TmpPts = vePtsInfor[m_ve2PtsIndex[i]];
        TmpPts.isDelete = true;

        //2度点只有两个影像点
        POINT2D& pt2D1 = TmpPts.vePoints[0];
        POINT2D& pt2D2 = TmpPts.vePoints[1];
        int nImIndex1, nImIndex2;
        nImIndex1 = GetImageIndexByImageID(pt2D1.nImageID);
        nImIndex2 = GetImageIndexByImageID(pt2D2.nImageID);
        //该点在不同航线上，应删除
        if (m_veImageInfor[nImIndex1].nStripID != m_veImageInfor[nImIndex2].nStripID)
        {
            TmpPts.isDelete = true;
            nDeleteNum++;
            continue;
        }

        //该点在航线边缘影像上，不可删除
        if (m_veImageInfor[nImIndex1].isStripEdge == true ||
            m_veImageInfor[nImIndex2].isStripEdge == true )
        {
            TmpPts.isDelete = false;
            continue;
        }

        int nx1, ny1, nx2, ny2;
        GetGridIndexByxy(pt2D1.x, pt2D1.y, nx1, ny1);
        GetGridIndexByxy(pt2D2.x, pt2D2.y, nx2, ny2);
        //有一个点所在格网只有2度点，因此不能删除
        if (m_veImageInfor[nImIndex1].isOnly2[ny1][nx1] == true ||
            m_veImageInfor[nImIndex2].isOnly2[ny2][nx2] == true )
        {
            TmpPts.isDelete = false;
            continue;
        }
        nDeleteNum++;
    }
    m_nDeleteNum = nDeleteNum;

    for (i=0; i<vePtsInfor.size(); i++)
    {
        if (vePtsInfor[i].isDelete == false)
        {
            m_vePtsInfor.push_back(vePtsInfor[i]);
        }
    }


    return 1;

}



//根据影像ID获得影像m_veImageInfor的索引号
int CPtsRefinor::GetImageIndexByImageID(int nImageID)
{
    //利用二分查找，加快查找速度
    int i=0, j=0, k=0;
// 	i = 1; j = m_veImageInfor.size();
// 	while (i <= j)
// 	{
// 		k = (i+j)/2;
// 		if (m_veImageInfor[k-1].nImageID == nImageID)
// 		{
// 			return (k-1);
// 			//res = k-1; break;
// 		}
// 		if (m_veImageInfor[k-1].nImageID > nImageID)
// 			j = k-1;
// 		else
// 			i = k+1;
// 	}

    for (i=0; i<m_veImageInfor.size(); i++)
    {
        if (m_veImageInfor[i].nImageID == nImageID)
            return i;
    }
    return -1;

}

//根据影像xy坐标获得点所在3*3格网的位置
int CPtsRefinor::GetGridIndexByxy(double x, double y, int& nx, int& ny)
{
	/*nx = int((x+m_dOffsetx) / m_dGridLenX);
	ny = int((y+m_dOffsety) / m_dGridLenY);*/

	// 	int j;
	// 	//确定x方向格网编号
	// 	for (j=0; j<3; j++)
	// 	{
	// 		if (x >= m_dGridX[j] && x < m_dGridX[j+1])
	// 		{
	// 			nx = j;
	// 			break;
	// 		}
	// 	}
	// 	//确定y方向格网编号
	// 	for (j=0; j<3; j++)
	// 	{
	// 		if (y >= m_dGridY[j] && y < m_dGridY[j+1])
	// 		{
	// 			ny = j;
	// 			break;
	// 		}
	// 	}
	//
	// 	if (nx != nx1 || ny != ny1)
	// 	{
	// 		int b = 0;
	// 	}
	return 1;
}


//
////将剔除后的点输出到文件中
//int CPtsRefinor::WriteRefinedPts(string strOutFile)
//{
//    FILE *outFile = fopen (strOutFile.c_str (), "w+");
//    if ( !outFile ) return 0;
//
//    if(isPTS == 1)
//    {
//        fprintf (outFile, "$Coordinates of Image and Space Points\n");
//        fprintf (outFile, "$Point Number  LineNumber \n");
//        fprintf (outFile, "$Point Name     X          Y          Z       Attrib\n");
//        fprintf (outFile, "$ImagePointsNumber\n");
//        fprintf (outFile, "$ImageID    x          y    GroupID\n");
//    }
//    fprintf (outFile, "%d %6d\n", m_vePtsInfor.size()-m_nDeleteNum, m_nLineNumber);
//
//    int i, j; int nt = 0;
//    for (i = 0; i < m_vePtsInfor.size(); i++)
//    {
//        PTSINFOR& TmpPts = m_vePtsInfor[i];
//        if (TmpPts.isDelete == true)
//            continue;
//
//        fprintf (outFile, "%10s %13.4lf %12.4lf %12.4lf %3d\n",
//                        TmpPts.strName, TmpPts.X, TmpPts.Y, TmpPts.Z, TmpPts.nAttrib);
//        fprintf (outFile, "%5d\n", TmpPts.nPtNum);
//
//        for (j = 0; j < TmpPts.nPtNum; j++)
//        {
//            if(isPTS == 1)
//                fprintf (outFile, "%5d %10.4lf %10.4lf %3d\n", TmpPts.vePoints[j].nImageID,
//                                TmpPts.vePoints[j].x, TmpPts.vePoints[j].y, TmpPts.vePoints[j].nGroupID);
//            else
//                fprintf (outFile, "%5d %10.4lf %10.4lf\n", TmpPts.vePoints[j].nImageID,
//                                TmpPts.vePoints[j].x, TmpPts.vePoints[j].y);
//        }
//        nt++;
//    }
//    fclose(outFile);
//    return 1;
//}
//
////生成分块数据目录，并将相机参数文件拷贝到生成的路径中
//int CPtsRefinor::CreateDivDir(string strDir)
//{
//    string strPath;
//    string strFileName;
//    string strCmrFile;
//    string strCmrName;
//    strCmrName = m_strIOFile.Right(m_strIOFile.GetLength() - m_strIOFile.ReverseFind('\\'));
//    int i = 0;
//    for (i=0; i<m_nStripNum; i++)
//    {
//        strFileName.Format("Strip_%d", i+1);
//        strPath = strDir + "\\" + strFileName;
//        CreateDirectory(strPath, NULL);
//        strCmrFile = strPath + strCmrName;
//        CopyFile(m_strIOFile, strCmrFile, false); //将已有的文件覆盖
//    }
//
////只生成单航带，不生成航带间的
//// 	for (i=0; i<m_nStripNum-1; i++)
//// 	{
//// 		strFileName.Format("Strip_%d_%d", i+1, i+2);
//// 		strPath = strDir + "\\" + strFileName;
//// 		CreateDirectory(strPath, NULL);
//// 		strCmrFile = strPath + strCmrName;
//// 		CopyFile(m_strIOFile, strCmrFile, false); //将已有的文件覆盖
//// 	}
//    m_strSaveDir = strDir;
//    return 1;
//}
//
////将数据进行分割
//int CPtsRefinor::DivideData()
//{
//    //删除不必要的点
//    RefineAndDeletePts();
//
//    //*************************************************************************
//    //影像信息的划分数据，并保存之
//    MAIMAGEINFOR maSglImageInfor, maJntImageInfor;
//    maSglImageInfor.resize(m_nStripNum);
//    maJntImageInfor.resize(m_nStripNum-1);
//    int i, j;
//    //数据分类
//    for (i=0; i<m_veImageInfor.size(); i++)
//    {
//        IMAGEINFOR& ImageInfor = m_veImageInfor[i];
//        //将这个数据推入该航带中
//        maSglImageInfor[ImageInfor.nStripID].push_back(ImageInfor);
//        //将该数据推入相邻航带中
//        if (ImageInfor.nStripID != 0)
//            maJntImageInfor[ImageInfor.nStripID-1].push_back(ImageInfor);
//        if (ImageInfor.nStripID != m_nStripNum-1)
//            maJntImageInfor[ImageInfor.nStripID].push_back(ImageInfor);
//    }
//    //数据保存
//    string strPath;
//    string strFileName;
//    for (i=0; i<m_nStripNum; i++)
//    {
//        strFileName.Format("Strip_%d", i+1);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPHTFile;
//        WriteImageInforFile(strPath, maSglImageInfor[i]);
//    }
//    for (i=0; i<m_nStripNum-1; i++)
//    {
//        strFileName.Format("Strip_%d_%d", i+1, i+2);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPHTFile;
//        WriteImageInforFile(strPath, maJntImageInfor[i]);
//    }
//    //*************************************************************************
//    //点信息的划分数据，并保存之
//    MAPTSINFOR maSglPtsInfor, maJntPtsInfor;
//    maSglPtsInfor.resize(m_nStripNum);
//    maJntPtsInfor.resize(m_nStripNum-1);
//    for (i=0; i<m_vePtsInfor.size(); i++)
//    {
//        PTSINFOR& pts = m_vePtsInfor[i];
//        int nPtInStripNum = 1; //点所在航带的个数，至少有1个
//        int nStripIndex1, nStripIndex2, nImageIndex;
//        nImageIndex = GetImageIndexByImageID(pts.vePoints[0].nImageID);
//        nStripIndex1 = m_veImageInfor[nImageIndex].nStripID;
//
//        for (j=1; j<pts.vePoints.size(); j++)
//        {
//            nImageIndex = GetImageIndexByImageID(pts.vePoints[j].nImageID);
//            if (m_veImageInfor[nImageIndex].nStripID != nStripIndex1)
//            {
//                nStripIndex2 = m_veImageInfor[nImageIndex].nStripID;
//                nPtInStripNum++;
//                break; //最多在两个航带上有点
//            }
//        }
//
//        //单航带点
//        if (nPtInStripNum == 1)
//        {
//            VEPTSINFOR& vePtsInfor = maSglPtsInfor[nStripIndex1];
//            vePtsInfor.push_back(m_vePtsInfor[i]);
//        }
//        else //交叉航带点
//        {
//            VEPTSINFOR& vePtsInfor = maJntPtsInfor[(nStripIndex1+nStripIndex2)/2];
//            vePtsInfor.push_back(m_vePtsInfor[i]);
//        }
//
//    }
//    //数据保存
//    test = 0;
//    for (i=0; i<m_nStripNum; i++)//单航带数据保存
//    {
//        strFileName.Format("Strip_%d", i+1);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPTSFile;
//        WritePtsInforFile(strPath, maSglPtsInfor[i]);
//    }
//    for (i=0; i<m_nStripNum-1; i++)//交叉航带数据保存
//    {
//        strFileName.Format("Strip_%d_%d", i+1, i+2);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPTSFile;
//        WritePtsInforFile(strPath, maJntPtsInfor[i]);
//    }
//    //*************************************************************************
//
//    return 1;
//}
//
////确定一个点是单航带的点，还是交叉航带的点，返回0代表为单航带点，1代表交叉航带点
//int CPtsRefinor::GetPtPos(PTSINFOR& pts)
//{
//
//    return 1;
//}
//


////写影像信息文件
//int CPtsRefinor::WriteImageInforFile(string strImageInforFile, VEIMAGEINFOR& veImageInfor)
//{
//    CStdioFile STD;
//    STD.Open(strImageInforFile, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
//    string strWrite;
//    strWrite.Format("$Exterior Orientation Parameters of Images\n");
//    STD.WriteString(strWrite);
//    strWrite.Format("$ImageNumber\n");
//    STD.WriteString(strWrite);
//    strWrite.Format("$ImageID     Xs           Ys           Zs            Phi         Omega        Kappa   StripID Attrib  CameraID  bFlag\n");
//    STD.WriteString(strWrite);
//
//    strWrite.Format("%5d\n", veImageInfor.size());
//    STD.WriteString(strWrite);
//
//    for (int i=0; i<veImageInfor.size(); i++)
//    {
//        strWrite.Format("%6d%15.4lf%13.4lf%13.4lf%13.6lf%13.6lf%13.6lf%6d%7d%8d%9d\n",
//                        veImageInfor[i].nImageID, veImageInfor[i].Xs, veImageInfor[i].Ys,
//                        veImageInfor[i].Zs, veImageInfor[i].phi, veImageInfor[i].omega,
//                        veImageInfor[i].kappa, veImageInfor[i].nStripID, veImageInfor[i].nAttrib,
//                        veImageInfor[i].nCameraID, veImageInfor[i].bFlag);
//        STD.WriteString(strWrite);
//    }
//
//    STD.Close();
//    return 1;
//}

//将点数据输出到文件中
int CPtsRefinor::WritePtsInforFile(string strPtsInforFile, VEPTSINFOR& vePtsInfor)
{
    FILE *outFile = fopen (strPtsInforFile.c_str (), "w+");
    if ( !outFile ) return 0;

    if(isPTS == 1)
    {
        fprintf (outFile, "$Coordinates of Image and Space Points\n");
        fprintf (outFile, "$Point Number  LineNumber \n");
        fprintf (outFile, "$Point Name     X          Y          Z       Attrib\n");
        fprintf (outFile, "$ImagePointsNumber\n");
        fprintf (outFile, "$ImageID    x          y    GroupID\n");
    }
    fprintf (outFile, "%d %6d\n", vePtsInfor.size(), m_nLineNumber);


    int i, j; int nt = 0;
    for (i=0; i<vePtsInfor.size(); i++)
    {
        PTSINFOR& TmpPts = vePtsInfor[i];
// 		if (TmpPts.isDelete == true)
// 			continue;
        fprintf (outFile, "%10s %13.4lf %12.4lf %12.4lf %3d\n",
                        TmpPts.strName, TmpPts.X, TmpPts.Y, TmpPts.Z, TmpPts.nAttrib);
        fprintf (outFile, "%5d\n", TmpPts.nPtNum);

        for (j = 0; j < TmpPts.vePoints.size(); j++)
        {
			if (TmpPts.vePoints[j].nImageID < 0 || TmpPts.vePoints[j].nImageID > 100000)
				cout << "Data false!"<< endl;


            if(isPTS == 1)
                fprintf (outFile, "%5d %10.4lf %10.4lf %3d\n", TmpPts.vePoints[j].nImageID, TmpPts.vePoints[j].x, TmpPts.vePoints[j].y, TmpPts.vePoints[j].nGroupID);
            else
                fprintf (outFile, "%5d %10.4lf %10.4lf\n", TmpPts.vePoints[j].nImageID,TmpPts.vePoints[j].x, TmpPts.vePoints[j].y);
        }
        nt ++;
        test ++;
    }
    fclose(outFile);
    return 1;
}

////按航带将数据分块
//int CPtsRefinor::BlockData()
//{
//    //删除不必要的点
//    if (m_bWeed2DegPts == true)
//    {
//        RefineAndDeletePts();
//    }
//
//    //*************************************************************************
//    //影像信息的划分数据，并保存之
//    MAIMAGEINFOR maSglImageInfor;
//    maSglImageInfor.resize(m_nStripNum);
//    int i=0, j=0, k=0;
//    //数据分类
//    for (i=0; i<m_veImageInfor.size(); i++)
//    {
//        IMAGEINFOR& ImageInfor = m_veImageInfor[i];
//        //将这个数据推入该航带中
//        maSglImageInfor[ImageInfor.nStripID].push_back(ImageInfor);
//// 		//将该数据推入相邻航带中
//// 		if (ImageInfor.nStripID != 0)
//// 			maJntImageInfor[ImageInfor.nStripID-1].push_back(ImageInfor);
//// 		if (ImageInfor.nStripID != m_nStripNum-1)
//// 			maJntImageInfor[ImageInfor.nStripID].push_back(ImageInfor);
//    }
//    //数据保存
//    string strPath;
//    string strFileName;
//    for (i=0; i<m_nStripNum; i++)
//    {
//        strFileName.Format("Strip_%d", i+1);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPHTFile;
//        WriteImageInforFile(strPath, maSglImageInfor[i]);
//    }
//// 	for (i=0; i<m_nStripNum-1; i++)
//// 	{
//// 		strFileName.Format("Strip_%d_%d", i+1, i+2);
//// 		strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPHTFile;
//// 		WriteImageInforFile(strPath, maJntImageInfor[i]);
//// 	}
//    //*************************************************************************
//    //点信息的划分数据，并保存之
////	MAPTSINFOR maSglPtsInfor, maJntPtsInfor;
//    m_maSglPtsInfor.clear();
//    m_maJntPtsInfor.clear();
//    m_maSglPtsInfor.resize(m_nStripNum);
//    m_maJntPtsInfor.resize(m_nStripNum);
//    for (i=0; i<m_vePtsInfor.size(); i++)
//    {
//
//        PTSINFOR& pts = m_vePtsInfor[i];
//        if (strcmp("10010081", pts.strName) == 0)
//        {
//            int b = 0;
//        }
//
//        pts.nIndex = i;//用于合并的时候进行索引
//        //统计该点出现的所有航带编号
//        PTSINFOR TmpPts = pts;
//        TmpPts.vePoints.clear(); //将影像点清空
//        int nImageIndex, nCurStpIndex, nLtStpIndex, nPtInStripNum;
//        nPtInStripNum = 1;
//        nImageIndex = GetImageIndexByImageID(pts.vePoints[0].nImageID);
//        nLtStpIndex = m_veImageInfor[nImageIndex].nStripID; //记录上一个航带号
//        TmpPts.vePoints.push_back(pts.vePoints[0]); //将第一个点推入栈中
//
//        for (j=1; j<pts.vePoints.size(); j++)
//        {
//            //获取影像的ID
//            nImageIndex = GetImageIndexByImageID(pts.vePoints[j].nImageID);
//            //获取当前影像的航带号
//            nCurStpIndex = m_veImageInfor[nImageIndex].nStripID;
//            if (nCurStpIndex != nLtStpIndex) //航带发生改变，一定属于交叉航带
//            {
//                nPtInStripNum ++;
//                TmpPts.nPtNum = TmpPts.vePoints.size();
//                //m_maJntPtsInfor[nLtStpIndex].push_back(TmpPts); //推入邻航带中
//                if (TmpPts.vePoints.size() >= 2) //只有等于或多于两个点才进行计算
//                {
//                    m_maJntPtsInfor[nLtStpIndex].push_back(TmpPts); //推入邻航带中
//                }
//                TmpPts.vePoints.clear();
//            }
//            TmpPts.vePoints.push_back(pts.vePoints[j]);
//            nLtStpIndex = nCurStpIndex;
//        }
//
//        if (nPtInStripNum == 1) //单航带
//        {
//            VEPTSINFOR& vePtsInfor = m_maSglPtsInfor[nLtStpIndex];
//            vePtsInfor.push_back(m_vePtsInfor[i]); //直接将该点存到单航带点中
//        }
//        else //交叉航带
//        {
//            TmpPts.nPtNum = TmpPts.vePoints.size();
//            if (TmpPts.vePoints.size() >= 2) //只有等于或多于两个点才进行计算
//            {
//                m_maJntPtsInfor[nCurStpIndex].push_back(TmpPts); //推入邻航带中
//            }
//        }
//
//    }
//
//    //数据保存
//    test = 0;
//    for (i=0; i<m_nStripNum; i++)
//    {
//        //单航带数据保存
//        strFileName.Format("Strip_%d", i+1);
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+strPTSFile;
//        WritePtsInforFile(strPath, m_maSglPtsInfor[i]);
//
//        //交叉航线数据保存
//        strPath = m_strSaveDir + "\\" + strFileName + "\\"+ "Jnt" + strPTSFile;
//        WritePtsInforFile(strPath, m_maJntPtsInfor[i]);
//    }
//    //***************************数据分块完毕**********************************************
//    return 1;
//}
//
//
//
//int CPtsRefinor::compare(const SORTDATA& p1, const SORTDATA& p2)
//{
//    return strcmp(p1.strName, p2.strName)>0 ? 1:0;
//}
//
//
////生成BA程序的工程文件，根据内定向路径、外定向路径、影像点文件数据，输出路径，生成在指定目录下的Proj文件
////nPcsMode：为处理的模式，0为前方交会，1为自由网
////dThrRMS:为Threshold of RMS Value
////dIgPtPc：为Image Point Precision X_1
//int CPtsRefinor::CreateProjFile(string strIOFile, string strEOFile, string strImageFile, string strOutPath,
//                                int nPcsMode, double dThrRMS, double dIgPtPc, string strProjPath)
//{
//    CStdioFile File(strProjPath,CFile::modeCreate|CFile::modeWrite);
//    string strWrite;
//    strWrite.Format("$Bundle Adjustment Project File\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("$Project Name:     DEFAULT PROJECT\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("$User    Name:     DEFAULT USER\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("$Last Updated:     2009-04-26  13:38:50\n\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("Interior Orientation File   :   ");
//    strWrite += strIOFile; strWrite += '\n';
//    File.WriteString(strWrite);
//
//    strWrite.Format("Exterior Orientation File   :   ");
//    strWrite += strEOFile;strWrite += '\n';
//    File.WriteString(strWrite);
//
//    strWrite.Format("Image & Pass Point   File   :   ");
//    strWrite += strImageFile; strWrite += '\n';
//    File.WriteString(strWrite);
//
//    strWrite.Format("Control & Check Point File  :\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("GPS or POS Data File        :\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("Bundle Adjustment OutPath   :   ");
//    strWrite += strOutPath; strWrite += "\n\n";
//    File.WriteString(strWrite);
//
//
//    strWrite.Format("Minimum Iterations          :   3\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Maximum Iterations          :   10\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Remove GPS Gross_Error      :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Remove IMU Gross Error      :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Image Data Type for Adjust  :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Min Overlaps of Pass Point  :   2\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Weight Strategy             :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Remove Invalid Control Point:   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Combined Bundle Adjustment  :   %d\n",nPcsMode);
//    File.WriteString(strWrite);
//    strWrite.Format("Detect Invalid Image Point  :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Detect IntStrip Gross Point :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Construct Free Network      :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Atmos. Refraction Correction:   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Earth Curvature Correction  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Ground Control Data Units   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Stable Coefficient          :   0.00000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Threshold of RMS Value      :   %.5f\n\n",dThrRMS);
//    File.WriteString(strWrite);
//
//    strWrite.Format("Output CTRL Point Residues  :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Interior Orientation :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Exterior Orientation :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Space Point Result   :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Adjustment Result    :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output System Error Result  :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Image Point Residues :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output GPS Observ. Residues :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output IMU Observ. Residues :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Intermediate Result  :   1\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output Unknow Precision     :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Output InterSectionAngle    :   0\n\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("Image Point Precision X_1   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision X_2   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision X_3   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision X_4   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision X_5   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision Y_1   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision Y_2   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision Y_3   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision Y_4   :   %.5f\n",dIgPtPc);
//    File.WriteString(strWrite);
//    strWrite.Format("Image Point Precision Y_5   :   %.5f\n\n",dIgPtPc);
//    File.WriteString(strWrite);
//
//    strWrite.Format("CTRL Point Precision XY_1   :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision XY_2   :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision XY_3   :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision XY_4   :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision XY_5   :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision Z_1    :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision Z_2    :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision Z_3    :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision Z_4    :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("CTRL Point Precision Z_5    :   0.05000\n\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("GPS Offset Calibration_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Offset Calibration_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Offset Calibration_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Offset Calibration_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Offset Calibration_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Const. Drift Calib_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Const. Drift Calib_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Const. Drift Calib_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Const. Drift Calib_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Const. Drift Calib_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Linear Drift Calib_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Linear Drift Calib_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Linear Drift Calib_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Linear Drift Calib_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Linear Drift Calib_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision XY_1     :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision XY_2     :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision XY_3     :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision XY_4     :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision XY_5     :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision Z_1      :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision Z_2      :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision Z_3      :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision Z_4      :   0.05000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("GPS Data Precision Z_5      :   0.05000\n\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("IMU Offset Calibration_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Offset Calibration_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Offset Calibration_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Offset Calibration_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Offset Calibration_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Const. Drift Calib_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Const. Drift Calib_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Const. Drift Calib_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Const. Drift Calib_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Const. Drift Calib_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Linear Drift Calib_1    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Linear Drift Calib_2    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Linear Drift Calib_3    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Linear Drift Calib_4    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Linear Drift Calib_5    :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Phi_1    :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Phi_2    :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Phi_3    :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Phi_4    :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Phi_5    :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Omega_1  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Omega_2  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Omega_3  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Omega_4  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Omega_5  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Kappa_1  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Kappa_2  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Kappa_3  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Kappa_4  :   0.00010000\n");
//    File.WriteString(strWrite);
//    strWrite.Format("IMU Data Precision Kappa_5  :   0.00010000\n\n");
//    File.WriteString(strWrite);
//
//
//    strWrite.Format("Lens Distortion Calib F_1   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib F_2   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib F_3   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib F_4   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib F_5   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib U_1   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib U_2   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib U_3   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib U_4   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib U_5   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib V_1   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib V_2   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib V_3   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib V_4   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib V_5   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K1_1  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K1_2  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K1_3  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K1_4  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K1_5  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K2_1  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K2_2  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K2_3  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K2_4  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib K2_5  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P1_1  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P1_2  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P1_3  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P1_4  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P1_5  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P2_1  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P2_2  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P2_3  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P2_4  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("Lens Distortion Calib P2_5  :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("System Error Estimation_1   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("System Error Estimation_2   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("System Error Estimation_3   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("System Error Estimation_4   :   0\n");
//    File.WriteString(strWrite);
//    strWrite.Format("System Error Estimation_5   :   0\n\n");
//    File.WriteString(strWrite);
//
//    strWrite.Format("$Bundle Adjustment Project File\n");
//    File.WriteString(strWrite);
//    return 1;
//}
//
////根据log文件，判断是否处理成功, strLogFile文件路径
////返回1，说明成功，0：失败
//int CPtsRefinor::JudgeIsSuccess(string strLogFile)
//{
//    CStdioFile File;
//    File.Open(strLogFile,CFile::modeRead|CFile::typeText);
//    string strRead;
//    for(;;)
//    {
//        File.ReadString(strRead);
//        if(strRead.Find("Exit Code") != -1)
//        {
//            if(strRead.Find("1") != -1)
//            {
//                File.Close();
//                return 1;
//            }
//            else
//            {
//                File.Close();
//                return 0;
//            }
//        }
//    }
//    File.Close();
//    return 0;
//}
//
////读取像点残差文件
////nStripID为航带编号，IsSgl是否为单航带的数据，1为是，0为航带间
//int CPtsRefinor::ReadPtsResiduesFile(string strRsdFile, VEPTSINFOR& vePts)
//{
//    CStdioFile STD;
//    STD.Open(strRsdFile, CFile::modeRead|CFile::typeText);
//    string strRead;
//    STD.ReadString(strRead);
//    sscanf(strRead, "$Root Mean Square Error: %lf", &m_dRMS);
//
//    int nPts;
//    STD.ReadString(strRead);
//    STD.ReadString(strRead);
//    sscanf(strRead, "%d", &nPts);
//    int i, j, nIgPts;
//    double X, Y, Z;
//    char strName[10];
//    if (nPts != vePts.size())
//    {
//        AfxMessageBox("数据不准");
//    }
//
//
//    for (i=0; i<nPts; i++)
//    {
//        if (i==1677)
//        {
//            int b = 0;
//        }
//        STD.ReadString(strRead);
//        sscanf(strRead, "%s %lf %lf %lf", strName, &X, &Y, &Z);
//
//
//        STD.ReadString(strRead);
//        sscanf(strRead, "%d", &nIgPts);
//        PTSINFOR& Pts = vePts[i];
//
//        if (strcmp(strName, Pts.strName) != 0 || nIgPts != Pts.vePoints.size())
//        {
//            AfxMessageBox("数据不准");
//        }
//
//
//        for (j=0; j<nIgPts; j++)
//        {
//            double x, y, rx, ry;
//            int nImageID;
//            STD.ReadString(strRead);
//            sscanf(strRead, "%d %lf %lf %lf %lf", &nImageID, &x, &y, &rx, &ry);
//            Pts.vePoints[j].rx = rx;
//            Pts.vePoints[j].ry = ry;
//        }
//    }
//
//    STD.Close();
//    return 1;
//}
//
//
////读取log文件，并获得相应的结果信息
////veSpan
//int CPtsRefinor::GetRstInforFromLog(string strLogFile, VESPAN& veSpan)
//{
//    CStdioFile File(strLogFile,CFile::modeRead);
//    string strRead;
//    vector<int> m_vecID;
//    for(;;)
//    {
//        if(File.ReadString(strRead) == NULL)
//            break;
//        if(strRead.Find("Model") != -1 || strRead.Find("Pair") != -1)
//        {
//            ExtractID(strRead,m_vecID);
//        }
//    }
//
//    sort(m_vecID.begin(),m_vecID.end(),cmp);
//    SPAN m_span;
//    for(int i = 0; i < m_vecID.size(); )
//    {
//        if(i == 0)
//        {
//            m_span.nBIID = -1;
//            m_span.nEIID = m_vecID.at(i);
//            veSpan.push_back(m_span);
//            i++;
//        }
//        else if(i == (m_vecID.size() - 1))
//        {
//            m_span.nBIID = m_vecID.at(i);
//            m_span.nEIID = -2;
//            veSpan.push_back(m_span);
//            i++;
//        }
//        else
//        {
//            m_span.nBIID = m_vecID.at(i);
//            m_span.nEIID = m_vecID.at(i+1);
//            i+=2;
//            veSpan.push_back(m_span);
//        }
//    }
//
//    return 1;
//}
//

//
//void CPtsRefinor::ExtractID(string strRead,vector<int> &vecID)
//{
//    if(strRead.Find("Model") != -1)  //对于Model类
//    {
//        int first,last;
//        string str;
//        first = strRead.Find('-');
//        last = strRead.ReverseFind('-');
//        str = strRead.Mid(first+1,last-first-1);
//        int ID;
//        ID = atoi(str);
//        vecID.push_back(ID);
//        vecID.push_back(ID);
//    }
//    else                                   //对于Pair类
//    {
//        int pos;
//        pos=strRead.Find("Pair",0);
//        strRead.Delete(pos,4);
//
//        int pos1;
//        pos1 = strRead.Find('-');
//
//        string str1,str2;
//        str1 = strRead.Left(pos1-1);//记录第一个数
//
//        int flag = 0;
//        for(int i = pos1+1; i < strRead.GetLength(); i++)
//        {
//            char ch = strRead.GetAt(i);
//            if(ch != ' ' )
//            {
//                str2 += ch;  //记录第二个数
//                flag = 1;
//            }
//            else if(ch == ' ' && flag == 1)
//                break;
//        }
//
//        int ID1,ID2;
//        ID1 = atoi(str1); ID2 = atoi(str2);
//        vecID.push_back(ID1);
//        vecID.push_back(ID2);
//
//    }
//
//// 	int d1,d2,d3;
//// 	string str1,str2;
//// 	if(strRead.Find("Model") != -1)
//// 	{
//// 		sscanf(strRead, "%s %d - %d - %d %s", str1, &d1, &d2, &d3, str2);
//// 		vecID.push_back(d2);
//// 		vecID.push_back(d2);
//// 	}
//// 	else
//// 	{
//// 		sscanf(strRead, "%s %d - %d %s",str1, &d1, &d2, str2);
//// 		vecID.push_back(d1);
//// 		vecID.push_back(d2);
//// 	}
//
//// 	string strTemp;
//// 	for (int i = 5; i < strRead.GetLength(); i++)
//// 	{
//// 		char tp = strRead.GetAt(i);
//// 		if(tp != ' ' && tp != '-')
//// 			strTemp += tp;
//// 		else if(tp == '-')
//// 		{
//// 			int ID;
//// 			ID = atoi(strTemp);
//// 			if(strRead.Find("Model") != -1)
//// 			{
//// 				ID++;
//// 				vecID.push_back(ID);
//// 				vecID.push_back(ID);
//// 			}
//// 			else
//// 			{
//// 				vecID.push_back(ID);
//// 				vecID.push_back(++ID);
//// 			}
//// 			break;
//// 		}
//// 	}
//
//}
//
////根据PROJ_OPT写.proj文件
////strBAProjFile为生成的文件路径
////ProjOpt为平差配置文件
//int CPtsRefinor::WriteBAProjFile(string strBAProjFile, PROJ_OPT& ProjOpt)
//{
//    CStdioFile stdfile;
//    stdfile.Open(strBAProjFile,CFile::modeCreate|CFile::modeWrite|CFile::typeText);
//    string strTemp;
//
//    strTemp = ProjOpt.strTitle + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "$Project Name:     " + ProjOpt.strProjectName + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "$User    Name:     " + ProjOpt.strUserName + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "$Last Updated:     " + ProjOpt.strLsUdTime + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    strTemp = "Interior Orientation File   :   " + ProjOpt.strIOFile + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "Exterior Orientation File   :   " + ProjOpt.strEOFile + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "Image & Pass Point   File   :   " + ProjOpt.strIgPsFile + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "Control & Check Point File  :   " + ProjOpt.strCtCkFile + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "GPS or POS Data File        :   " + ProjOpt.strGPSPOSDataFile + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "Bundle Adjustment OutPath   :   " + ProjOpt.strBAOutPath + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    strTemp.Format("%d",ProjOpt.nMinIter);
//    strTemp = "Minimum Iterations          :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nMaxIter);
//    strTemp = "Maximum Iterations          :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nRmGPSGross);
//    strTemp = "Remove GPS Gross_Error      :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nRmIMUGross);
//    strTemp = "Remove IMU Gross Error      :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nIgDataType);
//    strTemp = "Image Data Type for Adjust  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nMinOlPP);
//    strTemp = "Min Overlaps of Pass Point  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nWeightStragegy);
//    strTemp = "Weight Strategy             :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nRmIvCtPt);
//    strTemp = "Remove Invalid Control Point:   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nCBAdj);
//    strTemp = "Combined Bundle Adjustment  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nDtIvIgPt);
//    strTemp = "Detect Invalid Image Point  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nDtISGrossPt);
//    strTemp = "Detect IntStrip Gross Point :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nCtFreeNetwork);
//    strTemp = "Construct Free Network      :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nAtomsRfCrt);
//    strTemp = "Atmos. Refraction Correction:   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nEarthCurCorrect);
//    strTemp = "Earth Curvature Correction  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nGndCtDataUnits);
//    strTemp = "Ground Control Data Units   :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//
//    strTemp.Format("%.5lf",ProjOpt.dStableCoeff);
//    strTemp = "Stable Coefficient          :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%.5lf",ProjOpt.dThresRMS);
//    strTemp = "Threshold of RMS Value      :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//
//    strTemp.Format("%d",ProjOpt.nOutputCTRLPointResidues);
//    strTemp = "Output CTRL Point Residues  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputIO);
//    strTemp = "Output Interior Orientation :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputEO);
//    strTemp = "Output Exterior Orientation :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputSpacePointRes);
//    strTemp = "Output Space Point Result   :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputAdjRes);
//    strTemp = "Output Adjustment Result    :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputSysErrorRes);
//    strTemp = "Output System Error Result  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputImagePtResi);
//    strTemp = "Output Image Point Residues :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputGPSObsResi);
//    strTemp = "Output GPS Observ. Residues :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputIMUObsResi);
//    strTemp = "Output IMU Observ. Residues :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputIntermediateRes);
//    strTemp = "Output Intermediate Result  :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputUnknowPrecision);
//    strTemp = "Output Unknow Precision     :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//    strTemp.Format("%d",ProjOpt.nOutputInterSectionAngle);
//    strTemp = "Output InterSectionAngle    :   " + strTemp + "\n";
//    stdfile.WriteString(strTemp);
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    int i = 0;
//    string strnum;
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dIgPtPrcX[i]);
//        strTemp = "Image Point Precision X_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dIgPtPrcY[i]);
//        strTemp = "Image Point Precision Y_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dCTRLPtPcsXY[i]);
//        strTemp = "CTRL Point Precision XY_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dCTRLPtPcsZ[i]);
//        strTemp = "CTRL Point Precision Z_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nGPSOffsetCali[i]);
//        strTemp = "GPS Offset Calibration_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nGPSConstDriftCalib[i]);
//        strTemp = "GPS Const. Drift Calib_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nGPSLinearDriftCalib[i]);
//        strTemp = "GPS Linear Drift Calib_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dGPSDataPcsXY[i]);
//        strTemp = "GPS Data Precision XY_" + strnum + "     :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dGPSDataPcsZ[i]);
//        strTemp = "GPS Data Precision Z_" + strnum + "      :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nIMUOffsetCali[i]);
//        strTemp = "IMU Offset Calibration_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nIMUConstDriftCalib[i]);
//        strTemp = "IMU Const. Drift Calib_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nIMULinearDriftCalib[i]);
//        strTemp = "IMU Linear Drift Calib_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dIMUDataPcsPhi[i]);
//        strTemp = "IMU Data Precision Phi_" + strnum + "    :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dIMUDataPcsOmega[i]);
//        strTemp = "IMU Data Precision Omega_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%.5lf",ProjOpt.dIMUDataPcsKappa[i]);
//        strTemp = "IMU Data Precision Kappa_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliF[i]);
//        strTemp = "Lens Distortion Calib F_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliU[i]);
//        strTemp = "Lens Distortion Calib U_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliV[i]);
//        strTemp = "Lens Distortion Calib V_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliK1[i]);
//        strTemp = "Lens Distortion Calib K1_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliK2[i]);
//        strTemp = "Lens Distortion Calib K2_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliP1[i]);
//        strTemp = "Lens Distortion Calib P1_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nLensDistCaliP2[i]);
//        strTemp = "Lens Distortion Calib P2_" + strnum + "  :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//    for(i = 0;i<5;i++)
//    {
//        strnum.Format("%d",i+1);
//        strTemp.Format("%d",ProjOpt.nSysErrorEsti[i]);
//        strTemp = "System Error Estimation_" + strnum + "   :   " + strTemp + "\n";
//        stdfile.WriteString(strTemp);
//    }
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//    strTemp = ProjOpt.strCoda;
//    stdfile.WriteString(strTemp);
//
//    strTemp = "\n";
//    stdfile.WriteString(strTemp);
//
//
//    stdfile.Close();
//    return 1;
//
//}
//
////根据PROJ_OPT写.proj文件
////strBAProjFile为生成的文件路径
////ProjOpt为平差配置文件
//int CPtsRefinor::ReadBAProjFile(string strBAProjFile, PROJ_OPT& ProjOpt)
//{
//    CStdioFile File(strBAProjFile,CFile::modeRead);
//    string strRead;
//    int i = 1;
//
//    for(;;)
//    {
//        if(File.ReadString(strRead) == NULL)
//            break;
//        if(strRead != "")
//        {
//            string temp;
//            int j = strRead.Find(':'); temp = strRead.Right(strRead.GetLength()-j-1);
//            temp.TrimLeft();
//            switch(i)
//            {
//                case 1:
//                    ProjOpt.strTitle = temp;
//                    break;
//                case 2:
//                    ProjOpt.strProjectName = temp;
//                    break;
//                case 3:
//                    ProjOpt.strUserName = temp;
//                    break;
//                case 4:
//                    ProjOpt.strLsUdTime = temp;
//                    break;
//                    //////////////////////////////////////
//                case 5:
//                    ProjOpt.strIOFile = temp;
//                    break;
//                case 6:
//                    ProjOpt.strEOFile = temp;
//                    break;
//                case 7:
//                    ProjOpt.strIgPsFile = temp;
//                    break;
//                case 8:
//                    ProjOpt.strCtCkFile = temp;
//                    break;
//                case 9:
//                    ProjOpt.strGPSPOSDataFile = temp;
//                    break;
//                case 10:
//                    ProjOpt.strBAOutPath = temp;
//                    break;
//                    //////////////////////////////////////////
//                case 11:
//                    ProjOpt.nMinIter = atoi(temp);
//                    break;
//                case 12:
//                    ProjOpt.nMaxIter = atoi(temp);
//                    break;
//                case 13:
//                    ProjOpt.nRmGPSGross = atoi(temp);
//                    break;
//                case 14:
//                    ProjOpt.nRmIMUGross = atoi(temp);
//                    break;
//                case 15:
//                    ProjOpt.nIgDataType = atoi(temp);
//                    break;
//                case 16:
//                    ProjOpt.nMinOlPP = atoi(temp);
//                    break;
//                case 17:
//                    ProjOpt.nWeightStragegy = atoi(temp);
//                    break;
//                case 18:
//                    ProjOpt.nRmIvCtPt = atoi(temp);
//                    break;
//                case 19:
//                    ProjOpt.nCBAdj = atoi(temp);
//                    break;
//                case 20:
//                    ProjOpt.nDtIvIgPt = atoi(temp);
//                    break;
//                case 21:
//                    ProjOpt.nDtISGrossPt =atoi(temp);
//                    break;
//                case 22:
//                    ProjOpt.nCtFreeNetwork = atoi(temp);
//                    break;
//                case 23:
//                    ProjOpt.nAtomsRfCrt = atoi(temp);
//                    break;
//                case 24:
//                    ProjOpt.nEarthCurCorrect = atoi(temp);
//                    break;
//                case 25:
//                    ProjOpt.nGndCtDataUnits = atoi(temp);
//                    break;
//                case 26:
//                    ProjOpt.dStableCoeff = atof(temp);
//                    break;
//                case 27:
//                    ProjOpt.dThresRMS = atof(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//
//                case 28:
//                    ProjOpt.nOutputCTRLPointResidues = atoi(temp);
//                    break;
//                case 29:
//                    ProjOpt.nOutputIO = atoi(temp);
//                    break;
//                case 30:
//                    ProjOpt.nOutputEO = atoi(temp);
//                    break;
//                case 31:
//                    ProjOpt.nOutputSpacePointRes = atoi(temp);
//                    break;
//                case 32:
//                    ProjOpt.nOutputAdjRes = atoi(temp);
//                    break;
//                case 33:
//                    ProjOpt.nOutputSysErrorRes = atoi(temp);
//                    break;
//                case 34:
//                    ProjOpt.nOutputImagePtResi = atoi(temp);
//                    break;
//                case 35:
//                    ProjOpt.nOutputGPSObsResi = atoi(temp);
//                    break;
//                case 36:
//                    ProjOpt.nOutputIMUObsResi = atoi(temp);
//                    break;
//                case 37:
//                    ProjOpt.nOutputIntermediateRes = atoi(temp);
//                    break;
//                case 38:
//                    ProjOpt.nOutputUnknowPrecision = atoi(temp);
//                    break;
//                case 39:
//                    ProjOpt.nOutputInterSectionAngle = atoi(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//                case 40:
//                    ProjOpt.dIgPtPrcX[0] = atof(temp);
//                    break;
//                case 41:
//                    ProjOpt.dIgPtPrcX[1] = atof(temp);
//                    break;
//                case 42:
//                    ProjOpt.dIgPtPrcX[2] = atof(temp);
//                    break;
//                case 43:
//                    ProjOpt.dIgPtPrcX[3] = atof(temp);
//                    break;
//                case 44:
//                    ProjOpt.dIgPtPrcX[4] = atof(temp);
//                    break;
//                case 45:
//                    ProjOpt.dIgPtPrcY[0] = atof(temp);
//                    break;
//                case 46:
//                    ProjOpt.dIgPtPrcY[1] = atof(temp);
//                    break;
//                case 47:
//                    ProjOpt.dIgPtPrcY[2] = atof(temp);
//                    break;
//                case 48:
//                    ProjOpt.dIgPtPrcY[3] = atof(temp);
//                    break;
//                case 49:
//                    ProjOpt.dIgPtPrcY[4] = atof(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//                case 50:
//                    ProjOpt.dCTRLPtPcsXY[0]= atof(temp);
//                    break;
//                case 51:
//                    ProjOpt.dCTRLPtPcsXY[1] = atof(temp);
//                    break;
//                case 52:
//                    ProjOpt.dCTRLPtPcsXY[2] = atof(temp);
//                    break;
//                case 53:
//                    ProjOpt.dCTRLPtPcsXY[3] = atof(temp);
//                    break;
//                case 54:
//                    ProjOpt.dCTRLPtPcsXY[4] = atof(temp);
//                    break;
//                case 55:
//                    ProjOpt.dCTRLPtPcsZ[0]= atof(temp);
//                    break;
//                case 56:
//                    ProjOpt.dCTRLPtPcsZ[1] = atof(temp);
//                    break;
//                case 57:
//                    ProjOpt.dCTRLPtPcsZ[2] = atof(temp);
//                    break;
//                case 58:
//                    ProjOpt.dCTRLPtPcsZ[3] = atof(temp);
//                    break;
//                case 59:
//                    ProjOpt.dCTRLPtPcsZ[4] = atof(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//                case 60:
//                    ProjOpt.nGPSOffsetCali[0] = atoi(temp);
//                    break;
//                case 61:
//                    ProjOpt.nGPSOffsetCali[1] = atoi(temp);
//                    break;
//                case 62:
//                    ProjOpt.nGPSOffsetCali[2] = atoi(temp);
//                    break;
//                case 63:
//                    ProjOpt.nGPSOffsetCali[3] = atoi(temp);
//                    break;
//                case 64:
//                    ProjOpt.nGPSOffsetCali[4] = atoi(temp);
//                    break;
//                case 65:
//                    ProjOpt.nGPSConstDriftCalib[0] = atoi(temp);
//                    break;
//                case 66:
//                    ProjOpt.nGPSConstDriftCalib[1] = atoi(temp);
//                    break;
//                case 67:
//                    ProjOpt.nGPSConstDriftCalib[2] = atoi(temp);
//                    break;
//                case 68:
//                    ProjOpt.nGPSConstDriftCalib[3] = atoi(temp);
//                    break;
//                case 69:
//                    ProjOpt.nGPSConstDriftCalib[4] = atoi(temp);
//                    break;
//                case 70:
//                    ProjOpt.nGPSLinearDriftCalib[0] = atoi(temp);
//                    break;
//                case 71:
//                    ProjOpt.nGPSLinearDriftCalib[1] = atoi(temp);
//                    break;
//                case 72:
//                    ProjOpt.nGPSLinearDriftCalib[2] = atoi(temp);
//                    break;
//                case 73:
//                    ProjOpt.nGPSLinearDriftCalib[3] = atoi(temp);
//                case 74:
//                    ProjOpt.nGPSLinearDriftCalib[4] = atoi(temp);
//                    break;
//                case 75:
//                    ProjOpt.dGPSDataPcsXY[0] = atof(temp);
//                    break;
//                case 76:
//                    ProjOpt.dGPSDataPcsXY[1] = atof(temp);
//                    break;
//                case 77:
//                    ProjOpt.dGPSDataPcsXY[2] = atof(temp);
//                    break;
//                case 78:
//                    ProjOpt.dGPSDataPcsXY[3] = atof(temp);
//                case 79:
//                    ProjOpt.dGPSDataPcsXY[4] = atof(temp);
//                    break;
//                case 80:
//                    ProjOpt.dGPSDataPcsZ[0] = atof(temp);
//                    break;
//                case 81:
//                    ProjOpt.dGPSDataPcsZ[1] = atof(temp);
//                    break;
//                case 82:
//                    ProjOpt.dGPSDataPcsZ[2] = atof(temp);
//                    break;
//                case 83:
//                    ProjOpt.dGPSDataPcsZ[3] = atof(temp);
//                case 84:
//                    ProjOpt.dGPSDataPcsZ[4] = atof(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//                case 85:
//                    ProjOpt.nIMUOffsetCali[0] = atoi(temp);
//                    break;
//                case 86:
//                    ProjOpt.nIMUOffsetCali[1] = atoi(temp);
//                    break;
//                case 87:
//                    ProjOpt.nIMUOffsetCali[2] = atoi(temp);
//                    break;
//                case 88:
//                    ProjOpt.nIMUOffsetCali[3] = atoi(temp);
//                    break;
//                case 89:
//                    ProjOpt.nIMUOffsetCali[4] = atoi(temp);
//                    break;
//                case 90:
//                    ProjOpt.nIMUConstDriftCalib[0] = atoi(temp);
//                    break;
//                case 91:
//                    ProjOpt.nIMUConstDriftCalib[1] = atoi(temp);
//                    break;
//                case 92:
//                    ProjOpt.nIMUConstDriftCalib[2] = atoi(temp);
//                    break;
//                case 93:
//                    ProjOpt.nIMUConstDriftCalib[3] = atoi(temp);
//                    break;
//                case 94:
//                    ProjOpt.nIMUConstDriftCalib[4] = atoi(temp);
//                    break;
//                case 95:
//                    ProjOpt.nIMULinearDriftCalib[0] = atoi(temp);
//                    break;
//                case 96:
//                    ProjOpt.nIMULinearDriftCalib[1] = atoi(temp);
//                    break;
//                case 97:
//                    ProjOpt.nIMULinearDriftCalib[2] = atoi(temp);
//                    break;
//                case 98:
//                    ProjOpt.nIMULinearDriftCalib[3] = atoi(temp);
//                case 99:
//                    ProjOpt.nIMULinearDriftCalib[4] = atoi(temp);
//                    break;
//                case 100:
//                    ProjOpt.dIMUDataPcsPhi[0] = atof(temp);
//                    break;
//                case 101:
//                    ProjOpt.dIMUDataPcsPhi[1] = atof(temp);
//                    break;
//                case 102:
//                    ProjOpt.dIMUDataPcsPhi[2] = atof(temp);
//                    break;
//                case 103:
//                    ProjOpt.dIMUDataPcsPhi[3] = atof(temp);
//                case 104:
//                    ProjOpt.dIMUDataPcsPhi[4] = atof(temp);
//                    break;
//                case 105:
//                    ProjOpt.dIMUDataPcsOmega[0] = atof(temp);
//                    break;
//                case 106:
//                    ProjOpt.dIMUDataPcsOmega[1] = atof(temp);
//                    break;
//                case 107:
//                    ProjOpt.dIMUDataPcsOmega[2] = atof(temp);
//                    break;
//                case 108:
//                    ProjOpt.dIMUDataPcsOmega[3] = atof(temp);
//                case 109:
//                    ProjOpt.dIMUDataPcsOmega[4] = atof(temp);
//                    break;
//                case 110:
//                    ProjOpt.dIMUDataPcsKappa[0] = atof(temp);
//                    break;
//                case 111:
//                    ProjOpt.dIMUDataPcsKappa[1] = atof(temp);
//                    break;
//                case 112:
//                    ProjOpt.dIMUDataPcsKappa[2] = atof(temp);
//                    break;
//                case 113:
//                    ProjOpt.dIMUDataPcsKappa[3] = atof(temp);
//                case 114:
//                    ProjOpt.dIMUDataPcsKappa[4] = atof(temp);
//                    break;
//                    //////////////////////////////////////////////////////////////////////////
//                case 115:
//                    ProjOpt.nLensDistCaliF[0] = atoi(temp);
//                    break;
//                case 116:
//                    ProjOpt.nLensDistCaliF[1] = atoi(temp);
//                    break;
//                case 117:
//                    ProjOpt.nLensDistCaliF[2] = atoi(temp);
//                    break;
//                case 118:
//                    ProjOpt.nLensDistCaliF[3] = atoi(temp);
//                    break;
//                case 119:
//                    ProjOpt.nLensDistCaliF[4] = atoi(temp);
//                    break;
//                case 120:
//                    ProjOpt.nLensDistCaliU[0] = atoi(temp);
//                    break;
//                case 121:
//                    ProjOpt.nLensDistCaliU[1] = atoi(temp);
//                    break;
//                case 122:
//                    ProjOpt.nLensDistCaliU[2] = atoi(temp);
//                    break;
//                case 123:
//                    ProjOpt.nLensDistCaliU[3] = atoi(temp);
//                    break;
//                case 124:
//                    ProjOpt.nLensDistCaliU[4] = atoi(temp);
//                    break;
//                case 125:
//                    ProjOpt.nLensDistCaliV[0] = atoi(temp);
//                    break;
//                case 126:
//                    ProjOpt.nLensDistCaliV[1] = atoi(temp);
//                    break;
//                case 127:
//                    ProjOpt.nLensDistCaliV[2] = atoi(temp);
//                    break;
//                case 128:
//                    ProjOpt.nLensDistCaliV[3] = atoi(temp);
//                    break;
//                case 129:
//                    ProjOpt.nLensDistCaliV[4] = atoi(temp);
//                    break;
//                case 130:
//                    ProjOpt.nLensDistCaliK1[0] = atoi(temp);
//                    break;
//                case 131:
//                    ProjOpt.nLensDistCaliK1[1] = atoi(temp);
//                    break;
//                case 132:
//                    ProjOpt.nLensDistCaliK1[2] = atoi(temp);
//                    break;
//                case 133:
//                    ProjOpt.nLensDistCaliK1[3] = atoi(temp);
//                    break;
//                case 134:
//                    ProjOpt.nLensDistCaliK1[4] = atoi(temp);
//                    break;
//                case 135:
//                    ProjOpt.nLensDistCaliK2[0] = atoi(temp);
//                    break;
//                case 136:
//                    ProjOpt.nLensDistCaliK2[1] = atoi(temp);
//                    break;
//                case 137:
//                    ProjOpt.nLensDistCaliK2[2] = atoi(temp);
//                    break;
//                case 138:
//                    ProjOpt.nLensDistCaliK2[3] = atoi(temp);
//                    break;
//                case 139:
//                    ProjOpt.nLensDistCaliK2[4] = atoi(temp);
//                    break;
//                case 140:
//                    ProjOpt.nLensDistCaliP1[0] = atoi(temp);
//                    break;
//                case 141:
//                    ProjOpt.nLensDistCaliP1[1] = atoi(temp);
//                    break;
//                case 142:
//                    ProjOpt.nLensDistCaliP1[2] = atoi(temp);
//                    break;
//                case 143:
//                    ProjOpt.nLensDistCaliP1[3] = atoi(temp);
//                    break;
//                case 144:
//                    ProjOpt.nLensDistCaliP1[4] = atoi(temp);
//                    break;
//                case 145:
//                    ProjOpt.nLensDistCaliP2[0] = atoi(temp);
//                    break;
//                case 146:
//                    ProjOpt.nLensDistCaliP2[1] = atoi(temp);
//                    break;
//                case 147:
//                    ProjOpt.nLensDistCaliP2[2] = atoi(temp);
//                    break;
//                case 148:
//                    ProjOpt.nLensDistCaliP2[3] = atoi(temp);
//                    break;
//                case 149:
//                    ProjOpt.nLensDistCaliP2[4] = atoi(temp);
//                    break;
//                case 150:
//                    ProjOpt.nSysErrorEsti[0] = atoi(temp);
//                    break;
//                case 151:
//                    ProjOpt.nSysErrorEsti[1] = atoi(temp);
//                    break;
//                case 152:
//                    ProjOpt.nSysErrorEsti[2] = atoi(temp);
//                    break;
//                case 153:
//                    ProjOpt.nSysErrorEsti[3] = atoi(temp);
//                    break;
//                case 154:
//                    ProjOpt.nSysErrorEsti[4] = atoi(temp);
//                    break;
//                case 155:
//                    ProjOpt.strCoda = temp;
//                    break;
//                default:
//                    break;
//            }
//            i++;
//        }
//
//    }
//
//    return 1;
//}
//


//根据给定的格网，分配空间
int CPtsRefinor::AllocateGridDist(VEIMAGEINFOR& veImageInfor, int nRow, int nCol)
{

    for (int i=0; i<veImageInfor.size(); i++)
    {
        IMAGEINFOR& ImageInfor = veImageInfor[i];
        if (ImageInfor.pGridPtsInfor != NULL)
        {
            delete[] ImageInfor.pGridPtsInfor;
            ImageInfor.pGridPtsInfor = NULL;
        }
        ImageInfor.pGridPtsInfor = new VEPTS_ID_OP[nCol*nRow];
        ImageInfor.nGridCol = nCol;
        ImageInfor.nGridRow = nRow;
        //memset(ImageInfor.pGridDist, 0, sizeof(int));
    }

    return 1;
}

//释放格网数据
int CPtsRefinor::ReleaseGridDist(VEIMAGEINFOR& veImageInfor)
{
    for (int i=0; i<veImageInfor.size(); i++)
    {
        IMAGEINFOR& ImageInfor = veImageInfor[i];
        if (ImageInfor.pGridPtsInfor != NULL)
        {
            for (int j=0; j<ImageInfor.nGridCol*ImageInfor.nGridRow; j++)
            {
                ImageInfor.pGridPtsInfor[j].clear();
            }
            delete[] ImageInfor.pGridPtsInfor;
            ImageInfor.pGridPtsInfor = NULL;
        }
    }

    return 1;
}

//根据给定的格网（5*5），统计点的分布
int CPtsRefinor::StatisGridDist(VEIMAGEINFOR& veImageInfor, VEPTSINFOR& vePtsInfor, int nRow, int nCol)
{
    for (int i=0; i<vePtsInfor.size(); i++)
    {
        PTSINFOR& pts = vePtsInfor[i];

        for (int j=0; j<pts.vePoints.size(); j++) //读取每个像点坐标和航带数
        {
            POINT2D& pt2D = pts.vePoints[j];
            //获取点所在的影像索引号
            int nImageIndex = GetImageIndexByImageID(veImageInfor, pt2D.nImageID);
            int nCamID = veImageInfor[nImageIndex].nCameraID;
            double dFormatX, dFormatY;
            for (int k = 0; k < m_vCam.size(); k++)
            {
                if (m_vCam[k].CamID == nCamID)
                {
                    dFormatX = m_vCam[k].FormatX;
                    dFormatY = m_vCam[k].FormatY;
                    break;
                }
            }
            int nx, ny;
            //根据坐标获取点所在的格网
            //GetGridIndexByxy(pt2D.x, pt2D.y, m_dXFormat, m_dYFormat, nRow, nCol, nx, ny);
            GetGridIndexByxy(pt2D.x, pt2D.y, dFormatX, dFormatY, nRow, nCol, nx, ny);

            PTS_ID_OP idop;
            idop.nIndex = i;
            idop.nOP = pts.vePoints.size();

            //将该点的索引号推入影像对应格网中
            veImageInfor[nImageIndex].pGridPtsInfor[ny*nCol+nx].push_back(idop);

        }
    }
    return 1;
}

//根据影像ID获得影像veImageInfor的索引号
int CPtsRefinor::GetImageIndexByImageID(VEIMAGEINFOR& veImageInfor, int nImageID)
{
    for (int i=0; i<veImageInfor.size(); i++)
    {
        if (veImageInfor[i].nImageID == nImageID)
            return i;
    }
    return -1;

}

//获取点所在的格网
/*
x, y为点的坐标
XFormat, YFormat为影像的长度
nx, ny为所在的格网
nGridRow, nGridCol：统计格网的行列
*/
int CPtsRefinor::GetGridIndexByxy(double x, double y, double XFormat,
                                  double YFormat, int nGridRow, int nGridCol, int& nx, int& ny)
{
    double dOffsetx, dOffsety; //与左下角原点的偏移量
    dOffsetx = XFormat/2;
    dOffsety = YFormat/2;

    double dGridLenx, dGridLeny; //x、y方向上的格网长度
    dGridLenx = XFormat/nGridCol;
    dGridLeny = YFormat/nGridRow;

    nx = int( (x+dOffsetx)/dGridLenx );
    ny = int( (y+dOffsety)/dGridLeny );

    return 1;
}

//根据影像ID获取CamID
int CPtsRefinor::GetCamIndexByImgID(VEIMAGEINFOR& veImageInfor, int ImgID, int& nCamID)
{
    nCamID = -1;
    for (int i = 0; i < veImageInfor.size(); i++)
    {
        if (veImageInfor[i].nImageID == ImgID)
        {
            nCamID = veImageInfor[i].nCameraID;
            break;
        }
    }

    return 1;
}

//根据点在格网的分布，取重叠度在前位的点，剔除其余点
/*
strRefPtsFile:为剔除不需要点的点文件路径
veImageInfor：影像信息
vePtsInfor: 点信息
nRow, nCol: 格网的行列
nSaveNum: 每个格网保留的个数,即重叠度在前nSaveNum的点要保留
*/
int CPtsRefinor::RefinePtsByGridInfor(string strRefPtsFile, VEIMAGEINFOR& veImageInfor,  VEPTSINFOR& vePtsInfor, int nRow, int nCol, int nSaveNum)
{
    int i, j, k, t;

    for (i=0; i<veImageInfor.size(); i++)
    {
        IMAGEINFOR& ImageInfor = veImageInfor[i];
        for (j=0; j<nRow; j++)
        {
            for (k=0; k<nCol; k++)
            {
                VEPTS_ID_OP vePtsIDOP = ImageInfor.pGridPtsInfor[j*nCol+k];
                //根据重叠度进行排序
                sort(vePtsIDOP.begin(), vePtsIDOP.end(), cmp_pts);
                //nSaveNum与vePtsIDOP.size()取小值
                int te = (nSaveNum<vePtsIDOP.size() ? nSaveNum:vePtsIDOP.size());
                //这些点不该删除
                for (t=0; t<te; t++)
                {
                    int ID = vePtsIDOP[t].nIndex;
                    vePtsInfor[ID].isDelete = false;
                }
            }
        }
    }

    VEPTSINFOR veRefPtsInfor;
    for (i=0; i<vePtsInfor.size(); i++)
    {
        PTSINFOR& pts = vePtsInfor[i];
        if (pts.isDelete == false)
            veRefPtsInfor.push_back(pts);
    }
    WritePtsInforFile(strRefPtsFile, veRefPtsInfor);


    return 1;
}

//点剔除主函数
int CPtsRefinor::PtsRefineMainFunc()
{
    AllocateGridDist(m_veImageInfor, m_nGridRow, m_nGridCol);
    StatisGridDist(m_veImageInfor, m_vePtsInfor, m_nGridRow, m_nGridCol);
    RefinePtsByGridInfor(m_strRefPtsFile, m_veImageInfor, m_vePtsInfor, m_nGridRow, m_nGridCol, m_nSaveNum);
    WriteGridInfor(m_strGridInforFile, m_veImageInfor);
    ReleaseGridDist(m_veImageInfor);
    return 1;
}

//输出格网内缺点的信息
int CPtsRefinor::WriteGridInfor(string strGridInforFile,VEIMAGEINFOR& veImageInfor)
{
    FILE *outFile = fopen (strGridInforFile.c_str (), "w+");
    if ( !outFile ) return 0;

    fprintf (outFile,"$点数不足的格网信息\n");
    fprintf (outFile,"$Grid: Row Columun: %d  %d \n", m_nGridRow, m_nGridCol);
    fprintf (outFile,"$每个格网保留的点数: %d\n", m_nSaveNum);

    double dOPDeg = 0.6; //重叠度
    int nOPGridCol = ceil(dOPDeg*m_nGridCol); //在列上重叠的格网个数，向上取整，确保个数
    int i, j, k;
    int nCount = 0;
    for (i=0; i<veImageInfor.size(); i++)
    {
        IMAGEINFOR& ImageInfor = veImageInfor[i];
        for (j=0; j<m_nGridRow; j++)
        {
            for (k=0; k<m_nGridCol; k++)
            {
                if ( ImageInfor.nPos == 0 && k < (m_nGridCol-nOPGridCol) ||//航带首的影像，且格网处于非重叠区域
                     ImageInfor.nPos == 2 && k >= nOPGridCol )//航带首的影像，且格网处于非重叠区域
                    continue;

                VEPTS_ID_OP& vePtsIDOP = ImageInfor.pGridPtsInfor[j*m_nGridCol+k];
                //点数不足
                if (vePtsIDOP.size() < m_nSaveNum)
                    nCount ++;
            }
        }
    }

    fprintf (outFile,"$不满足要求的格网的个数: %d\n", nCount);
    fprintf (outFile,"$ImageID    GridRow    GridColumn    PointNum\n");


    for (i=0; i<veImageInfor.size(); i++)
    {
        IMAGEINFOR& ImageInfor = veImageInfor[i];
        for (j=0; j<m_nGridRow; j++)
        {
            for (k=0; k<m_nGridCol; k++)
            {
                if ( ImageInfor.nPos == 0 && k < (m_nGridCol-nOPGridCol) ||//航带首的影像，且格网处于非重叠区域
                     ImageInfor.nPos == 2 && k >= nOPGridCol )//航带首的影像，且格网处于非重叠区域
                    continue;

                VEPTS_ID_OP& vePtsIDOP = ImageInfor.pGridPtsInfor[j*m_nGridCol+k];
                //点数不足
                if (vePtsIDOP.size() < m_nSaveNum)
                {
                    fprintf (outFile,"%8d%10d%13d%10d\n", ImageInfor.nImageID, j, k, vePtsIDOP.size());
                }
            }
        }

    }
    fclose(outFile);
    return 1;
}
