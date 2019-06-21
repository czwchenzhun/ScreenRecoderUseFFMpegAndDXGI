#pragma once
#include"CommonTypes.h"

/* Represents the main loop of an application which generates one frame per loop. */
//static void encode_example(const char *filename, int codec_id) {
//	int pts;
//	int width = 320;
//	int height = 240;
//	uint8_t *rgb = NULL;
//	ffmpeg_encoder_start(filename, codec_id, 25, width, height);
//	for (pts = 0; pts < 100; pts++) {
//		frame->pts = pts;
//		//rgb = generate_rgb(width, height, pts, rgb);
//		ffmpeg_encoder_encode_frame(rgb);
//	}
//	ffmpeg_encoder_finish();
//}

class MyFFMpegFunc
{
public:
	AVCodecContext *c = NULL;
	AVFrame *frame;
	AVPacket pkt;
	FILE *file;
	SwsContext *sws_context = NULL;
public:
	MyFFMpegFunc() {}
	void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb);
	void ffmpeg_encoder_set_frame_yuv_from_bgra(uint8_t *bgra);
	void ffmpeg_encoder_start(const char *filename, int codec_id, int width, int height, int fps, int bitrate);
	void ffmpeg_encoder_start(const char *filename, const char *codec_name, int width, int height, int fps, int bitrate);
	void ffmpeg_encoder_finish(void);
	void ffmpeg_encoder_encode_frame_rgb(uint8_t *rgb);
	void ffmpeg_encoder_encode_frame_bgra(uint8_t *bgra);
};