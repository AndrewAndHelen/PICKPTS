#include "CPtsRefinor.h"
#include <iostream>
#include "Win2Linux.h"

using namespace  std;
int main (int argc, char *argv[])
{
    cout << "挑点开始..." << endl;
    if(argc != 8)   { cout << "error argc!" << endl;   return 0; }
    CPtsRefinor PtsRef;
    PtsRef.isPTS = 1;
    // 内参 .cmr
    string DirIOFile = argv[1];
    // 外参 .pht
    string DirEOFile = argv[2];
    // 连接点 .pts
    string DirPTFile = argv[3];
    // 结果路径
    string DirResFile = argv[4];

    PtsRef.m_nGridRow = atoi(argv[5]);
    PtsRef.m_nGridCol = atoi(argv[6]);
    PtsRef.m_nSaveNum = atoi(argv[7]);

    char GridInfor[512];
    {
        char szdrive_[_MAX_DRIVE];
        char szdir_[_MAX_DIR];
        char szfname_[_MAX_FNAME];
        char szext_[_MAX_EXT];
        _splitpath(DirResFile, szdrive_, szdir_, szfname_, szext_);
        sprintf(GridInfor, "%s%s%s.grid", szdrive_, szdir_, szfname_);
    }


    PtsRef.ReadIOFile(DirIOFile);
    PtsRef.ReadImageInforFile(DirEOFile);
    PtsRef.ReadPtsFile(DirPTFile);

    PtsRef.m_strRefPtsFile = DirResFile;
    PtsRef.m_strGridInforFile = GridInfor;

    PtsRef.PtsRefineMainFunc();
    cout << "挑点成功" << endl;
    return 0;
}
