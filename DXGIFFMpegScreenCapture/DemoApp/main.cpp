#include"CommonTypes.h"
#include"Duplication.h"
#include<ctime>
#include"ffmpegFunc.h"


//½«B8G8R8A8×ªÎªB8G8R8
inline void dataFormatTransform(BYTE** data,int width,int height) 
{
	BYTE* pt = *data;
	BYTE* newData = new BYTE[width*height * 3];
	BYTE* pt2 = newData;
	BYTE value;
	int numberOfPixel = width*height;
	for (int i = 0; i < numberOfPixel; i++)
	{
		memcpy_s(pt2, 3, pt, 3);
		pt += 4;
		pt2 += 3;
	}
	delete *data;
	*data = newData;
}

int main() 
{
	Duplication dup;
	if (!dup.InitDevice())
	{
		std::cout << "init device failed" << std::endl;
		return 0;
	}
	if (!dup.InitDupl(0))
	{
		std::cout << "init duplication failed" << std::endl;
		return 0;
	}
	av_register_all();

	int fps = 18;
	int bitrate = 2000000;
	int width = 1920;
	int height = 1080;
	const char* outputFileName = "h264qsv.h264";
	const char* encoderName = "h264_qsv";
	BYTE* buffer = nullptr;
	BYTE* backupBuffer = nullptr;

	clock_t start, end;
	float duration;
	start = clock();
	int counter = 0;
	int time = 60;

	MyFFMpegFunc ffmpegFunc;
	ffmpegFunc.ffmpeg_encoder_start(outputFileName, encoderName, width, height, fps, bitrate);

	while (true)
	{
		end = clock();
		duration = (float)(end - start) / (float)CLOCKS_PER_SEC;
		if (duration > time)
			break;

		if (dup.GetFrame())
		{
			if (dup.copyFrameDataToBuffer(&buffer, width, height))
			{
				
				dup.DoneWithFrame();
				ffmpegFunc.frame->pts = counter;
				ffmpegFunc.ffmpeg_encoder_encode_frame_bgra(buffer);

				delete backupBuffer;
				backupBuffer = buffer;
				buffer = nullptr;
				counter++;
			}
			else
			{
				fprintf(stderr, "Get frame failed!\n");
				getchar();
				return 0;
			}
		}
		else
		{
			ffmpegFunc.frame->pts = counter;
			ffmpegFunc.ffmpeg_encoder_encode_frame_bgra(backupBuffer);
			counter++;
		}
	}
	if (buffer)
		delete buffer;
	if (backupBuffer)
		delete buffer;
	ffmpegFunc.ffmpeg_encoder_finish();
	cout << "FPS:" << (float)counter/(float)time << endl;
	getchar();
	return 0;
}