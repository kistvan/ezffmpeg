INCLUDE = -I.
LIBS = -L./libavformat -L./libavcodec -L./libavutil -L./libswscale -lavformat -lavcodec -lavutil -lswscale

all: dll main

main:libfmpgdebug.c
	gcc -Wall -g -o libfmpegdebug libfmpgdebug.c ezffmpeg.dll $(INCLUDE) $(LIBS) 


dll:libffmpeg.c
	gcc -Wall -g -shared -o ezffmpeg.dll libffmpeg.c $(INCLUDE) $(LIBS) -Wl,--output-def,ezffmpeg.def,--out-implib,libezffmpeg.a 

