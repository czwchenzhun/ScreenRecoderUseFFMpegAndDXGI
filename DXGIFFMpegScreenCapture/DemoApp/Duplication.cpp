#include"Duplication.h"

Duplication::Duplication():m_DeskDupl(nullptr),
m_AcquiredDesktopImage(nullptr),
m_OutputNumber(0),
m_Device(nullptr)
{
}

Duplication::~Duplication()
{
	if (m_DeskDupl)
	{
		m_DeskDupl->Release();
		m_DeskDupl = nullptr;
	}

	if (m_AcquiredDesktopImage)
	{
		m_AcquiredDesktopImage->Release();
		m_AcquiredDesktopImage = nullptr;
	}

	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
}

char Duplication::GetFrame()
{
	IDXGIResource* DesktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	// Get new frame
	HRESULT hr = m_DeskDupl->AcquireNextFrame(100, &FrameInfo, &DesktopResource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
	{
		return 0;
	}

	if (FAILED(hr))
	{
		return 0;
	}

	// If still holding old frame, destroy it
	if (m_AcquiredDesktopImage)
	{
		m_AcquiredDesktopImage->Release();
		m_AcquiredDesktopImage = nullptr;
	}

	// QI for IDXGIResource
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&m_AcquiredDesktopImage));
	DesktopResource->Release();
	DesktopResource = nullptr;
	if (FAILED(hr))
	{
		return 0;
	}
	return 1;
}

char Duplication::copyFrameDataToBuffer(UCHAR** buffer, int& width, int& height)
{
	HRESULT hr;
	ID3D11DeviceContext* context;
	m_Device->GetImmediateContext(&context);

	D3D11_MAPPED_SUBRESOURCE mapRes;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);

	D3D11_TEXTURE2D_DESC desc;
	m_AcquiredDesktopImage->GetDesc(&desc);

	hr=context->Map(m_AcquiredDesktopImage, subresource, D3D11_MAP_READ, 0, &mapRes);
	ID3D11Texture2D* destImage = nullptr;
	if (FAILED(hr))
	{
		if (hr == E_INVALIDARG) 
		{
			D3D11_TEXTURE2D_DESC desc2;
			desc2.Width = desc.Width;
			desc2.Height = desc.Height;
			desc2.MipLevels = desc.MipLevels;
			desc2.ArraySize = desc.ArraySize;
			desc2.Format = desc.Format;
			desc2.SampleDesc = desc.SampleDesc;
			desc2.Usage = D3D11_USAGE_STAGING;
			desc2.BindFlags = 0;
			desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc2.MiscFlags = 0;
			//不能直接使用m_AquiredDesktopImage的desc描述信息,需要拷贝一个
			hr = m_Device->CreateTexture2D(&desc2, nullptr, &destImage);
			if (FAILED(hr)) {
				return 0;
			}

			context->CopyResource(destImage, m_AcquiredDesktopImage);//源Texture2D和目的Texture2D需要有相同的多重采样计数和质量时（就是一些属性相同）
			//所以目的纹理对象应该初始化设置，使用m_AcquiredDesktopImage的描述信息来创建纹理（代码是前几行）

			hr = context->Map(destImage, subresource, D3D11_MAP_READ, 0, &mapRes);
			if (FAILED(hr)) {
				printf("%x\n", hr);
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	width = desc.Width;
	height = desc.Height;
	std::unique_ptr<BYTE> pBuf(new BYTE[mapRes.RowPitch*desc.Height]);
	UINT lBmpRowPitch = desc.Width * 4;
	BYTE* sptr = reinterpret_cast<BYTE*>(mapRes.pData);
	BYTE* dptr = pBuf.get() + mapRes.RowPitch*desc.Height - lBmpRowPitch;
	UINT lRowPitch = std::min<UINT>(lBmpRowPitch, mapRes.RowPitch);

	for (size_t h = 0; h < desc.Height; ++h)
	{
		memcpy_s(dptr, lBmpRowPitch, sptr, lRowPitch);
		sptr += mapRes.RowPitch;
		dptr -= lBmpRowPitch;
	}

	context->Unmap(destImage, subresource);
	long g_captureSize = lRowPitch*desc.Height;
	*buffer = new UCHAR[g_captureSize];
	//buffer = (UCHAR*)malloc(g_captureSize);

	//Copying to UCHAR buffer 
	memcpy(*buffer, pBuf.get(), g_captureSize);
	return 1;
}

char Duplication::copyFrameDataToBuffer1(UCHAR ** buffer, int & width, int & height)
{
	HRESULT hr;
	ID3D11DeviceContext* context;
	m_Device->GetImmediateContext(&context);

	D3D11_TEXTURE2D_DESC desc;
	m_AcquiredDesktopImage->GetDesc(&desc);

	ID3D11Texture2D* destImage = nullptr;
	D3D11_TEXTURE2D_DESC desc2;
	desc2.Width = desc.Width;
	desc2.Height = desc.Height;
	desc2.MipLevels = desc.MipLevels;
	desc2.ArraySize = desc.ArraySize;
	desc2.Format = desc.Format;
	desc2.SampleDesc = desc.SampleDesc;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.MiscFlags = 0;
	//不能直接使用m_AquiredDesktopImage的desc描述信息,需要拷贝一个
	hr = m_Device->CreateTexture2D(&desc2, nullptr, &destImage);
	if (FAILED(hr)) {
		return 0;
	}

	context->CopyResource(destImage, m_AcquiredDesktopImage);//源Texture2D和目的Texture2D需要有相同的多重采样计数和质量时（就是一些属性相同）
																//所以目的纹理对象应该初始化设置，使用m_AcquiredDesktopImage的描述信息来创建纹理（代码是前几行）
	D3D11_MAPPED_SUBRESOURCE mapRes;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	hr = context->Map(destImage, subresource, D3D11_MAP_READ, 0, &mapRes);
	if (FAILED(hr)) {
		printf("%x\n", hr);
		return 0;
	}
	width = desc2.Width;
	height = desc2.Height;
	UINT lBmpRowPitch = desc2.Width * 4;
	UINT lRowPitch = std::min<UINT>(lBmpRowPitch, mapRes.RowPitch);
	long totalSize = lRowPitch*desc2.Height;
	*buffer = new UCHAR[totalSize];
	BYTE* sptr = reinterpret_cast<BYTE*>(mapRes.pData);
	BYTE* dptr = *buffer;
	
	memcpy_s(dptr, totalSize, sptr, totalSize);

	context->Unmap(destImage, subresource);
	destImage->Release();
	return 1;
}

char Duplication::DoneWithFrame()
{
	HRESULT hr = m_DeskDupl->ReleaseFrame();
	if (FAILED(hr))
	{
		return 0;
	}

	if (m_AcquiredDesktopImage)
	{
		m_AcquiredDesktopImage->Release();
		m_AcquiredDesktopImage = nullptr;
	}

	return 1;
}

char Duplication::InitDevice()
{
	HRESULT hr = S_OK;

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &m_Device, &FeatureLevel, nullptr);
		if (SUCCEEDED(hr))
		{
			// Device creation success, no need to loop anymore
			return 1;
		}
	}

	return 0;
}

char Duplication::InitDupl(UINT Output)
{
	
	
	m_OutputNumber = Output;
	// Take a reference on the device
	m_Device->AddRef();

	// Get DXGI device
	IDXGIDevice* DxgiDevice = nullptr;
	HRESULT hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	if (FAILED(hr))
	{
		return 0;
	}

	// Get DXGI adapter
	IDXGIAdapter* DxgiAdapter = nullptr;
	hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	if (FAILED(hr))
	{
		return 0;
	}

	// Get output
	IDXGIOutput* DxgiOutput = nullptr;
	hr = DxgiAdapter->EnumOutputs(Output, &DxgiOutput);
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	if (FAILED(hr))
	{
		return 0;
	}

	DxgiOutput->GetDesc(&m_OutputDesc);

	// QI for Output 1
	IDXGIOutput1* DxgiOutput1 = nullptr;
	hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	DxgiOutput->Release();
	DxgiOutput = nullptr;
	if (FAILED(hr))
	{
		return 0;
	}

	// Create desktop duplication
	hr = DxgiOutput1->DuplicateOutput(m_Device, &m_DeskDupl);
	DxgiOutput1->Release();
	DxgiOutput1 = nullptr;
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			MessageBoxW(nullptr, L"There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again.", L"Error", MB_OK);
			return 0;
		}
		return 0;
	}

	return 1;
}

//void Duplication::saveFrameToBmp(PCWSTR FileName)
//{
//	saveTextureToBmp(FileName, m_AcquiredDesktopImage);
//}
//
//void Duplication::saveTextureToBmp(PCWSTR FileName, ID3D11Texture2D* Texture)
//{
//	HRESULT hr;
//
//	// First verify that we can map the texture
//	D3D11_TEXTURE2D_DESC desc;
//	Texture->GetDesc(&desc);
//
//	// translate texture format to WIC format. We support only BGRA and ARGB.
//	GUID wicFormatGuid;
//	switch (desc.Format) {
//	case DXGI_FORMAT_R8G8B8A8_UNORM:
//		wicFormatGuid = GUID_WICPixelFormat32bppRGBA;
//		break;
//	case DXGI_FORMAT_B8G8R8A8_UNORM:
//		wicFormatGuid = GUID_WICPixelFormat32bppBGRA;
//		break;
//	default:
//		return;
//	}
//
//	// Get the device context
//	ComPtr<ID3D11Device> d3dDevice;
//	Texture->GetDevice(&d3dDevice);
//	ComPtr<ID3D11DeviceContext> d3dContext;
//	d3dDevice->GetImmediateContext(&d3dContext);
//
//	// map the texture
//	ComPtr<ID3D11Texture2D> mappedTexture;
//	D3D11_MAPPED_SUBRESOURCE mapInfo;
//	mapInfo.RowPitch;
//	hr = d3dContext->Map(
//		Texture,
//		0,  // Subresource
//		D3D11_MAP_READ,
//		0,  // MapFlags
//		&mapInfo);
//
//	if (FAILED(hr)) {
//		// If we failed to map the texture, copy it to a staging resource
//		if (hr == E_INVALIDARG) {
//			D3D11_TEXTURE2D_DESC desc2;
//			desc2.Width = desc.Width;
//			desc2.Height = desc.Height;
//			desc2.MipLevels = desc.MipLevels;
//			desc2.ArraySize = desc.ArraySize;
//			desc2.Format = desc.Format;
//			desc2.SampleDesc = desc.SampleDesc;
//			desc2.Usage = D3D11_USAGE_STAGING;
//			desc2.BindFlags = 0;
//			desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//			desc2.MiscFlags = 0;
//
//			ComPtr<ID3D11Texture2D> stagingTexture;
//			hr = d3dDevice->CreateTexture2D(&desc2, nullptr, &stagingTexture);
//			if (FAILED(hr)) {
//				return;
//			}
//
//			// copy the texture to a staging resource
//			d3dContext->CopyResource(stagingTexture.Get(), Texture);
//
//			// now, map the staging resource
//			hr = d3dContext->Map(
//				stagingTexture.Get(),
//				0,
//				D3D11_MAP_READ,
//				0,
//				&mapInfo);
//			if (FAILED(hr)) {
//				return;
//			}
//
//			mappedTexture = std::move(stagingTexture);
//		}
//		else {
//			return;
//		}
//	}
//	else {
//		mappedTexture = Texture;
//	}
//	d3dContext->Unmap(mappedTexture.Get(), 0);
//	ComPtr<IWICImagingFactory2> wicFactory;
//	hr = CoCreateInstance(
//		CLSID_WICImagingFactory,
//		nullptr,
//		CLSCTX_INPROC_SERVER,
//		IID_IWICImagingFactory2,
//		(LPVOID*)(wicFactory.GetAddressOf()));
//	if (FAILED(hr)) {
//		return;
//	}
//
//	ComPtr<IWICBitmapEncoder> wicEncoder;
//	hr = wicFactory->CreateEncoder(
//		GUID_ContainerFormatBmp,
//		nullptr,
//		&wicEncoder);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	ComPtr<IWICStream> wicStream;
//	hr = wicFactory->CreateStream(&wicStream);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	hr = wicStream->InitializeFromFilename(FileName, GENERIC_WRITE);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	hr = wicEncoder->Initialize(wicStream.Get(), WICBitmapEncoderNoCache);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	// Encode and commit the frame
//	{
//		ComPtr<IWICBitmapFrameEncode> frameEncode;
//		wicEncoder->CreateNewFrame(&frameEncode, nullptr);
//		if (FAILED(hr)) {
//			return;
//		}
//
//		hr = frameEncode->Initialize(nullptr);
//		if (FAILED(hr)) {
//			return;
//		}
//
//
//		hr = frameEncode->SetPixelFormat(&wicFormatGuid);
//		if (FAILED(hr)) {
//			return;
//		}
//
//		hr = frameEncode->SetSize(desc.Width, desc.Height);
//		if (FAILED(hr)) {
//			return;
//		}
//
//		hr = frameEncode->WritePixels(
//			desc.Height,
//			mapInfo.RowPitch,
//			desc.Height * mapInfo.RowPitch,
//			reinterpret_cast<BYTE*>(mapInfo.pData));
//		if (FAILED(hr)) {
//			return;
//		}
//
//		hr = frameEncode->Commit();
//		if (FAILED(hr)) {
//			return;
//		}
//	}
//
//	hr = wicEncoder->Commit();
//	if (FAILED(hr)) {
//		return;
//	}
//}
