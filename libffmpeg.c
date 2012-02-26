
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h> 

#include <stdio.h>

#include "libffmpeg.h"

typedef struct tagBITMAPFILEHEADER {
//  unsigned short bfType;
  unsigned long  bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long  bfOffBits;
} BMP_HEADER ;

typedef struct tagBITMAPINFOHEADER{
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImporant;
} BMP_INFO;

#define DEST_PIX_FMT PIX_FMT_BGR24


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[256];
  int  y;
  
  // Open file
  sprintf(szFilename, "E:\\MinGW\\msys\\1.0\\home\\koji\\work\\ezffmpeg\\img\\frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL) {
	printf("ファイルが開けません");
    return;
  }
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

void SaveFrameBmp(AVFrame *pFrame, int width, int height, FILE *pSaveFile) {
  FILE *pFile = pSaveFile;
//  char szFilename[256];
  int  y;
  unsigned short headerId = 0x4d42; //BM
//  uint8_t blue[3] = {0x000000, 0x000000, 0x0000FF};
  BMP_HEADER header;
  BMP_INFO info;
//  header.bfSize = (width*3*height) + sizeof(BMP_HEADER) + sizeof(BMP_INFO);
//  header.bfType = 0x4d42;	/* "BM" */
  header.bfSize = 0;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = sizeof(BMP_HEADER) + sizeof(BMP_INFO);
  
  info.biSize          = sizeof(BMP_INFO);
  info.biWidth         = width;
  info.biHeight        = height;
  info.biPlanes        = 1;
  info.biBitCount      = 24;
  info.biCompression   = 0;
  info.biSizeImage     = 0;
  info.biXPixPerMeter = 1;
  info.biYPixPerMeter = 1;
  info.biClrUsed       = 0;
  info.biClrImporant  = 0;  
  
  
  // Open file
//  sprintf(szFilename, "E:\\MinGW\\msys\\1.0\\home\\koji\\work\\ezffmpeg\\img\\frame%d.bmp", iFrame);
//  pFile=fopen(szFilename, "wb");
//  if(pFile==NULL) {
//	printf("ファイルが開けません");
//    return;
//  }
  
  // Write header
  fwrite(&headerId, sizeof(headerId), 1, pFile);
  if(fwrite(&header, sizeof(BMP_HEADER), 1, pFile) != 1) {
	printf("ファイルヘッダ書き込み失敗");
  }
  if(fwrite(&info, sizeof(BMP_INFO), 1, pFile) != 1) {
	printf("ファイルINFO書き込み失敗");
  }
  
  // Write pixel data
  //*が先
  for(y=height; y>=0; y--) {
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], width*3, 1, pFile);
  }
//    printf("%d_", y);
//	for(i = 0; i < width; i++) {
//	    fwrite(blue, sizeof(uint8_t[3]), 1, pFile);
//	}
  
  // Close file
//  fclose(pFile);
}

void init_ffmpeg()
{
  av_register_all();
}


int ffmpegrun(int argc, char *argv[], FILE *pSaveFile) {
  AVFormatContext *pFormatCtx  = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx = NULL;
  AVCodec         *pCodec = NULL;
  AVFrame         *pFrame = NULL; 
  AVFrame         *pFrameRGB = NULL;
  AVPacket        packet;
  int             frameFinished;
  int             numBytes;
  int             numBytesRGB;
  uint8_t         *buffer;
  uint8_t         *bufferRGB;
  struct SwsContext *img_convert_ctx = NULL;
  
  
  int frameNum = 0;
  frameFinished = 1;
  
  if(argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }
  // Register all formats and codecs
  
  printf("ab %d\n", 11);
	printf("finame...%s\n", argv[0]);
	printf("finame...%s\n", argv[1]);
  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0) {
	printf("ファイルが開けません");
    return -1; // Couldn't open file
  }
  printf("abc\n");
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information
  printf("stream_infoが存在します\n");
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1; // Could not open codec
    printf("コーデック開きました\n");
  
  // Allocate video frame
  pFrame=avcodec_alloc_frame();
  
  // Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if(pFrame == NULL || pFrameRGB==NULL)
    return -1;
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width,
			      pCodecCtx->height);
  numBytesRGB=avpicture_get_size(DEST_PIX_FMT, pCodecCtx->width,
			      pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  memset ( buffer, 0, numBytes*sizeof(uint8_t) ); 
  bufferRGB=(uint8_t *)av_malloc(numBytesRGB*sizeof(uint8_t));
  memset ( bufferRGB, 0, numBytesRGB*sizeof(uint8_t) ); 
  
  printf("codecのフォーマット:%d\n", pCodecCtx->pix_fmt);
  
  img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
  									pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height,
  									DEST_PIX_FMT, SWS_BICUBIC, NULL,NULL,NULL);
  									
  if (img_convert_ctx == NULL) {
	printf("sws_contextの初期化に失敗しました\n");
	return -1;
  }
  

  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrame, buffer, pCodecCtx->pix_fmt,
		 pCodecCtx->width, pCodecCtx->height);
  avpicture_fill((AVPicture *)pFrameRGB, bufferRGB, DEST_PIX_FMT,
		 pCodecCtx->width, pCodecCtx->height);
		 
	printf("rgb data[0] :%p\n", pFrameRGB->data[0]);
 	printf("rgb data[1] :%p\n", pFrameRGB->data[1]);
	printf("rgb data[2] :%p\n", pFrameRGB->data[2]);
	printf("rgb linesize :%d\n", pFrameRGB->linesize[0]);
 
  av_init_packet(&packet);
  // Read frames and save first five frames to disk
  printf("frame読み込み開始\n");
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
	if (packet.stream_index!=videoStream) {
		printf("video streamではないのでスキップ %d", i);
		i++;
		continue;
	}
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
		printf("-----------\n");
		printf("start dec\n");
		printf("packet duration:%d\n", packet.duration);
		printf("packet data:%p\n", packet.data);
		printf("packet size:%d\n", packet.size);
		printf("packet pts:%I64d\n", packet.pts);
		printf("packet dts:%I64d\n", packet.dts);
		printf("stream index:%d\n", packet.stream_index);
		int get_pic_ptr;
      // Decode video frame
      int decoded = 0;
      decoded = avcodec_decode_video2(pCodecCtx, pFrame, 
			   &get_pic_ptr, &packet);
	if (get_pic_ptr == 0) {
		printf("skip no frame: %d\n", i);
		i++;
		continue;
	} else {
		frameNum++;

	}
/*
	printf("start dec ok decoded:%d i:%d gotPic:%d\n", decoded, i, get_pic_ptr);
	printf("yuv data[0] :%d\n", pFrame->data[0]);
	printf("yuv data[1] :%d\n", pFrame->data[1]);
	printf("yuv data[2] :%d\n", pFrame->data[2]);
	printf("yuv linesize :%d\n", pFrame->linesize[0]);
*/
      
      // Did we get a video frame?
      if(frameFinished) {
	// Convert the image from its native format to RGB
//	img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
//                    (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
//                    pCodecCtx->height);
	int heightOfSlice = 0;
	heightOfSlice = sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
	          pFrameRGB->data, pFrameRGB->linesize);
/*
	printf("heightOfSlice:%d\n", heightOfSlice);
	printf("data[0] :%d size:%d num:%d\n", pFrameRGB->data[0], sizeof(pFrameRGB->data), (sizeof(pFrameRGB->data)/sizeof(pFrameRGB->data[0])));
	printf("data[1] :%d\n", pFrameRGB->data[1]);
	printf("data[2] :%d\n", pFrameRGB->data[2]);
	printf("rgb linesize :%d\n", pFrameRGB->linesize[0]);
	printf("codec width:%d\n", pCodecCtx->width);
	
	// Save the frame to disk
	printf("save frame... %d\n ", i);
*/
	  SaveFrameBmp(pFrameRGB, pCodecCtx->width, pCodecCtx->height, 
		    pSaveFile);
      }
    }
	++i;
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
    
    if(frameNum >= 1) {
		break;
	}
  }
  printf("frameNum:%d", frameNum);
  
  // Free the RGB image
  av_free(buffer);
  av_free(bufferRGB);
  av_free(pFrameRGB);
  
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  sws_freeContext(img_convert_ctx);
  
  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}
