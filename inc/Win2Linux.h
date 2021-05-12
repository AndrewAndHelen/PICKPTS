#ifndef WIN2LINUX_H
#define WIN2LINUX_H
#include <iostream>
#ifdef  _WIN32
#include<io.h>
#include<direct.h>
#else defined linux
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <string.h>

#define _MAX_FNAME 256
#define _MAX_PATH  256
#define _MAX_DIR   256
#define _MAX_DRIVE 3
#define _MAX_EXT   32

void _splitpath(std::string path, char *drive, char *dir, char *fname, char *ext);

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);

bool copyFile(const char* src, const char* des);

static void _split_whole_name(const char *whole_name, char *fname, char *ext);

#endif // WIN2LINUX_H


