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
        _splitpath(m_strEOPath, szdrive_, szdir_, szfname_, szext_);
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
