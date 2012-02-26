
#include <stdio.h>

#include "libffmpeg.h"

int main( int argc, char *argv[] )
{
	printf("start...%s\n%s\n", argv[0], argv[1]);
	init_ffmpeg();
	ffmpegrun(argc, argv);
	return 0;
}