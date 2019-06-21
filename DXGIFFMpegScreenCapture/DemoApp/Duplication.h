#pragma once
#ifndef _DUPLICATION_H_
#define _DUPLICATION_H_
#include"CommonTypes.h"

class Duplication
{
public:
	Duplication();
	~Duplication();
	bool GetFrame(int timeout=100); //millisecond
	bool copyFrameDataToBuffer(BYTE** buffer,int& width,int& height);
	bool DoneWithFrame();
	bool InitDevice();
	bool InitDupl(UINT Output);
	//void saveFrameToBmp(PCWSTR FileName);
	//static void saveTextureToBmp(PCWSTR FileName, ID3D11Texture2D* Texture);

private:
	// vars
	IDXGIOutputDuplication* m_DeskDupl;
	ID3D11Texture2D* m_AcquiredDesktopImage;
	UINT m_OutputNumber;
	DXGI_OUTPUT_DESC m_OutputDesc;
	ID3D11Device* m_Device;
};

#endif
