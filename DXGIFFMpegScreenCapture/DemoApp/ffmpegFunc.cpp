#include"ffmpegFunc.h"

void MyFFMpegFunc::ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb) {
	const int in_linesize[1] = { 3 * c->width };
	sws_context = sws_getCachedContext(sws_context,
		c->width, c->height, AV_PIX_FMT_RGB24,
		c->width, c->height, AV_PIX_FMT_YUV420P,
		0, 0, 0, 0);
	sws_scale(sws_context, (const uint8_t * const *)&rgb, in_linesize, 0,
		c->height, frame->data, frame->linesize);
}

void MyFFMpegFunc::ffmpeg_encoder_set_frame_yuv_from_bgra(uint8_t *bgra) {
	const int in_linesize[1] = { 4 * c->width };
	sws_context = sws_getCachedContext(sws_context,
		c->width, c->height, AV_PIX_FMT_BGRA,
		c->width, c->height, AV_PIX_FMT_NV12,
		0, 0, 0, 0);
	sws_scale(sws_context, (const uint8_t * const *)&bgra, in_linesize, 0,
		c->height, frame->data, frame->linesize);
}

/* Allocate resources and write header data to the output file. */
void MyFFMpegFunc::ffmpeg_encoder_start(const char *filename, int codec_id, int fps, int width, int height) {
	AVCodec *codec;
	int ret;

	codec = avcodec_find_encoder((AVCodecID)codec_id);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}
	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	c->bit_rate = 2000000;
	c->width = width;
	c->height = height;
	c->time_base.num = 1;
	c->time_base.den = fps;
	c->keyint_min = 600;
	c->pix_fmt = AV_PIX_FMT_YUV420P;
	if (codec_id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}
	file = fopen(filename, "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;
	ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}
}

void MyFFMpegFunc::ffmpeg_encoder_start(const char *filename, char *codec_name, int fps, int width, int height) {
	AVCodec *codec;
	int ret;

	codec = avcodec_find_encoder_by_name(codec_name);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}
	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	c->bit_rate = 2500000;
	c->width = width;
	c->height = height;
	c->time_base.num = 1;
	c->time_base.den = fps;
	c->keyint_min = 20;
	c->pix_fmt = AV_PIX_FMT_NV12;
	//c->pix_fmt = AV_PIX_FMT_YUV420P;
	
	av_opt_set(c->priv_data, "preset", "slow", 0);
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}
	file = fopen(filename, "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;
	ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}
}

/*
Write trailing data to the output file
and free resources allocated by ffmpeg_encoder_start.
*/
void MyFFMpegFunc::ffmpeg_encoder_finish(void) {
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	int got_output, ret;
	do {
		fflush(stdout);
		ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			exit(1);
		}
		if (got_output) {
			fwrite(pkt.data, 1, pkt.size, file);
			av_packet_unref(&pkt);
		}
	} while (got_output);
	fwrite(endcode, 1, sizeof(endcode), file);
	fclose(file);
	avcodec_close(c);
	av_free(c);
	av_freep(&frame->data[0]);
	av_frame_free(&frame);
}

/*
Encode one frame from an RGB24 input and save it to the output file.
Must be called after ffmpeg_encoder_start, and ffmpeg_encoder_finish
must be called after the last call to this function.
*/
void MyFFMpegFunc::ffmpeg_encoder_encode_frame_rgb(uint8_t *rgb) {
	int ret, got_output;
	ffmpeg_encoder_set_frame_yuv_from_rgb(rgb);
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	if (frame->pts == 1) {
		frame->key_frame = 1;
		frame->pict_type = AV_PICTURE_TYPE_I;
	}
	else {
		frame->key_frame = 0;
		frame->pict_type = AV_PICTURE_TYPE_P;
	}
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
	if (ret < 0) {
		fprintf(stderr, "Error encoding frame\n");
		exit(1);
	}
	if (got_output) {
		fwrite(pkt.data, 1, pkt.size, file);
		av_packet_unref(&pkt);
	}
}

void MyFFMpegFunc::ffmpeg_encoder_encode_frame_bgra(uint8_t *bgra) {
	int ret, got_output;
	ffmpeg_encoder_set_frame_yuv_from_bgra(bgra);
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	if (frame->pts == 1) {
		frame->key_frame = 1;
		frame->pict_type = AV_PICTURE_TYPE_I;
	}
	else {
		frame->key_frame = 0;
		frame->pict_type = AV_PICTURE_TYPE_P;
	}
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
	if (ret < 0) {
		fprintf(stderr, "Error encoding frame\n");
		exit(1);
	}
	if (got_output) {
		fwrite(pkt.data, 1, pkt.size, file);
		av_packet_unref(&pkt);
	}
}