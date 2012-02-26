
typedef struct {
	int width;
	int height;
	int linesize;
	uint8_t *data;
	
} BMP_DATA_INFO;

void init_ffmpeg();

int ffmpegrun(int, char *openFileName, FILE *pSaveFile);
