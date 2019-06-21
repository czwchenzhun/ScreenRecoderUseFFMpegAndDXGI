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

	int fps = 20;
	int width, height;
	UCHAR* buffer = nullptr;
	clock_t start, end;
	float duration;
	start = clock();
	int counter = 0;
	int time = 60;
	char fileName[10];
	av_register_all();
	MyFFMpegFunc ffmpegFunc;
	ffmpegFunc.ffmpeg_encoder_start("h264qsv.h264", "h264_qsv", 18, 1920, 1080);
	while (true)
	{
		end = clock();
		duration = (float)(end - start) / (float)CLOCKS_PER_SEC;
		if (duration > time)
			break;
		if (dup.GetFrame())
		{
			if (dup.copyFrameDataToBuffer1(&buffer, width, height))
			{
				dup.DoneWithFrame();

				ffmpegFunc.frame->pts = counter;
				ffmpegFunc.ffmpeg_encoder_encode_frame_bgra(buffer);

				delete buffer;
				buffer = nullptr;
				counter++;
			}
			else
				int a = 1;
		}
	}
	ffmpegFunc.ffmpeg_encoder_finish();
	cout << "FPS:" << (float)counter/(float)time << endl;
	getchar();
	return 0;
}