
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libffmpeg.h"

int main( int argc, char *argv[] )
{
	printf("start...%s\n%s\n", argv[0], argv[1]);
	init_ffmpeg();
	
	char pathname[MAX_PATH];
	memset(pathname, '\0', MAX_PATH);
	getcwd(pathname, MAX_PATH);
	fprintf(stdout,"���݂̃t�@�C���p�X:%s\n", pathname);
	FILE *pFile;
    char szFilename[MAX_PATH + 256];
    sprintf(szFilename, "%s\\img\\%s", pathname, argv[2]);
    pFile=fopen(szFilename, "wb");
	if(pFile==NULL) {
	printf("�t�@�C�����J���܂��� %s\n", szFilename);
	  return -1;
	}
	printf("�������݃t�@�C�� %s\n", szFilename);
	ffmpegrun(argc, argv, pFile);
	
	fclose(pFile);
	
	return 0;
}