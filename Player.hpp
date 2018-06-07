#include <iostream>
#include <assert.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}
#include "SDL2/SDL.h"
#include "SDL2/SDL_thread.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_audio.h"

#define SDL_MAIN_HANDLED
#define ERROR_SIZE 128
#define FORMATO AV_PIX_FMT_RGB24
#define SDL_AUDIO_BUFFER_SIZE 1024;
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

typedef struct _AudioPacket
	{
		AVPacketList *first, *last;
		int nb_packets, size;
  		SDL_mutex *mutex;
  		SDL_cond *cond;
	} AudioPacket;

class Player {

public:
	
	//construtor
	Player(std::string filename) {

		audioStream = -1;

		//init ffmpeg
		av_register_all();

		//open video
		int res = avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL);

		//check video opened
		if (res!=0){
			ShowError(res);
			exit(-1);
		}

		//get video info
		res = avformat_find_stream_info(pFormatCtx, NULL);
		if (res < 0) {
			ShowError(res);
			exit(-1);
		}

		//get video stream
		videoStream = GetCodecParameters();
		if (videoStream == -1) {
			std::cout << "Error opening your video using AVCodecParameters, does not have codecpar_type type AVMEDIA_TYPE_VIDEO" << std::endl;
			exit(-1);
		}

		if (ReadideoCodec() < 0) exit(-1);

	}

	~Player(void) {

		av_free(buffer);
		av_free(pFrameRGB);

		// Free the YUV frame
		av_free(pFrame);

		// Close the codecs
		avcodec_close(pCodecCtx);

		// Close the video file
		avformat_close_input(&pFormatCtx);

	}

	
	//void exibirInformacaoArquivoVideo(void);
	int AllocateMemory(void);
	int lerFramesVideo(void);
	int CreateDisplay(void);
	
	static int GetAudioPacket(AudioPacket*, AVPacket*, int);

private:
	
	//void memsetAudioPacket(AudioPacket * pq);

	// Stores the index of the given stream to be transmitted
	int videoStream;

	// Audio Stream index
	int audioStream;

	// Contains information about the video file, including codecs, etc.
	AVFormatContext *pFormatCtx = NULL;

	// Contains information about the video codec, obtained through pFormatCtx->streams[i]->codecpar
	// looking at the codec_type and seeing if video transmission of type AVMEDIA_TYPE_VIDEO
	AVCodecParameters *pCodecParameters = NULL;

	//Audio COdec Parametrs
	AVCodecParameters *pCodecAudioParameters = NULL;


	// CodecParameters information, but copied. pCodecParameters serves as a backup of the video information
	AVCodecContext *pCodecCtx = NULL;

	AVCodecContext *pCodecAudioCtx = NULL;

	SDL_AudioSpec wantedSpec = { 0 }, audioSpec = { 0 };

	// Save Video Codec
	AVCodec *pCodec = NULL;

	// Save Audio Codec
	AVCodec *pAudioCodec = NULL;

	// Structure that holds the frame
	AVFrame *pFrame = NULL;

	// Structure that holds RGB frame
	AVFrame *pFrameRGB = NULL;

	// Buffer for reading video frames
	uint8_t *buffer = NULL;

	// Structure that stores the conversion to RGB
	struct SwsContext *sws_ctx = NULL;

	//surface window para exibir o video
	//pode ter m�ltiplas screens
	SDL_Window *screen;

	SDL_Renderer *renderer;

	SDL_Texture* bmp;
	
	// Displays the error message for the given error code
	void ShowError(int error);

	int GetCodecParameters(void);

	int ReadideoCodec(void);

	//int PacketQueuePut(AudioPacket *, const AVPacket *);

	void initAudioPacket(AudioPacket *); 

	int putAudioPacket(AudioPacket *, AVPacket *); 

};