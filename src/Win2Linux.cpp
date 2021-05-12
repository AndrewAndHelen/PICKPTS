#include "Win2Linux.h"

void _splitpath(std::string path, char *drive, char *dir, char *fname, char *ext)
{
	std::string str;

	char pre_char = path[0];
	str.push_back(path[0]);

	int len = path.size();

	for (int i = 1; i < len; ++i)
	{

		if (path[i] == '/')
		{
			if (pre_char == '/' || pre_char == '\\')
				continue;
			else
				str.push_back(path[i]);
		}
		else if (path[i] == '\\')
		{
			if (pre_char == '/' || pre_char == '\\')
				continue;
			else
				str.push_back('/');
		}
		else
		{
			str.push_back(path[i]);
		}

		pre_char = str.back();
	}
	_splitpath(str.c_str(), drive, dir, fname, ext);
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	char *p_whole_name;

	drive[0] = '\0';
	if (NULL == path)
	{
		dir[0] = '\0';
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	if ('/' == path[strlen(path)])
	{
		strcpy(dir, path);
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	p_whole_name = const_cast<char*>(strrchr(path, '/'));
	if (NULL != p_whole_name)
	{
		p_whole_name++;
		_split_whole_name(p_whole_name, fname, ext);

		snprintf(dir, p_whole_name - path, "%s", path);
	}
	else
	{
		_split_whole_name(path, fname, ext);
		dir[0] = '\0';
	}
	char tmp[256];
	strcpy(tmp, dir);
	sprintf(dir, "%s/", tmp);   // add "/" at the last
}

static void _split_whole_name(const char *whole_name, char *fname, char *ext)
{
	char *p_ext;

	p_ext = const_cast<char*>(strrchr(whole_name, '.'));
	if (NULL != p_ext)
	{
		strcpy(ext, p_ext);
		snprintf(fname, p_ext - whole_name + 1, "%s", whole_name);
	}
	else
	{
		ext[0] = '\0';
		strcpy(fname, whole_name);
	}
}

static bool copyFile(const char* src, const char* des)
{
	FILE* pSrc = NULL, *pDes = NULL;
	pSrc = fopen(src, "r");
	pDes = fopen(des, "w+");


	if (pSrc && pDes)
	{
		int nLen = 0;
		char szBuf[1024] = { 0 };
		while ((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0)
		{
			fwrite(szBuf, 1, nLen, pDes);
		}
	}
	else
		return false;


	if (pSrc)
		fclose(pSrc), pSrc = NULL;


	if (pDes)
		fclose(pDes), pDes = NULL;
	return true;
}

