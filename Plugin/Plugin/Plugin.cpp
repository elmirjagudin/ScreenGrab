// Plugin.cpp : Defines the exported functions for the DLL application.
//
//#include "stdafx.h"
#include <d3d11.h>

#include "PluginAPI/IUnityGraphics.h"
#include "PluginAPI/IUnityInterface.h"
#include "PluginAPI/IUnityGraphicsD3D11.h"


#include <stdio.h>

#include <Windows.h>

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;

ID3D11Device *m_Device = NULL;
ID3D11DeviceContext *context = NULL;

ID3D11Texture2D *grabTexture = NULL;

ID3D11ShaderResourceView  *resourceView;
UINT screenWidth;
UINT screenHeight;

ID3D11Texture2D *screenTexture;
ID3D11Texture2D *screenCopyTexture;


void InitConsole()
{
	/* pop-up a Console window and send stdout there (for debugging) */
	FILE * pConsole;
	AllocConsole();
	freopen_s(&pConsole, "CONOUT$", "wb", stdout);
}

void InitMeep();

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	InitConsole();

	IUnityGraphicsD3D11* d3d = unityInterfaces->Get<IUnityGraphicsD3D11>();
	m_Device = d3d->GetDevice();

	InitMeep();
}


ID3D11Resource *screenResource;

char *rawBuffer;

void InitMeep()
{
	ID3D11RenderTargetView *targetView;
	

	m_Device->GetImmediateContext(&context);
	context->OMGetRenderTargets(1, &targetView, NULL);

	targetView->GetResource(&screenResource);

	HRESULT hr = screenResource->QueryInterface(IID_ID3D11Texture2D, (void **)&screenTexture);
	if (FAILED(hr))
	{
		printf("error upcasting resource to texture2d");
		return;
	}

	D3D11_TEXTURE2D_DESC description;
	screenTexture->GetDesc(&description);

	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.Usage = D3D11_USAGE_STAGING;
	description.SampleDesc.Count = 1;
	
	printf(" Width %d Height %d MipLevels %d  ArraySize %d Format %d  Usage %d BindFlags %d CPUAccessFlags %d MiscFlags %d\n",
		description.Width, description.Height, description.MipLevels, description.ArraySize, description.Format, description.Usage, description.BindFlags, description.CPUAccessFlags, description.MiscFlags);

	printf("sample desk Count %d Quality %d\n", description.SampleDesc.Count, description.SampleDesc.Quality);



	hr = m_Device->CreateTexture2D(&description, NULL, &screenCopyTexture);
	if (FAILED(hr))
	{
		printf("failed to create screenCopyTexture %x\n", hr);
	}


	printf("device %p context %p targetView %p screen %p screenCopyTexture %p\n", m_Device, context, targetView, screenResource, screenCopyTexture);


	screenWidth = description.Width;
	screenHeight = description.Height;

	rawBuffer = (char*)malloc(screenWidth * screenHeight * 4);

}


// If exported by a plugin, this function will be called when the plugin is about to be unloaded.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
}

void CopyScreen()
{
	context->CopyResource(screenCopyTexture, screenResource);
}


//extern "C" UNITY_INTERFACE_EXPORT ID3D11ShaderResourceView* UNITY_INTERFACE_API MakeGrabTexture(UINT width, UINT height)
//{
//	screenWidth = width;
//	screenHeight = height;
//
//	printf("creating grab texture %d %d\n", screenWidth, screenHeight);
//	D3D11_TEXTURE2D_DESC description;
//
//	description.Width = width;
//	description.Height = height;
//	description.MipLevels = 1;
//	description.ArraySize = 1;
//	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	description.SampleDesc.Count = 1;
//	description.SampleDesc.Quality = 0;
//	description.MiscFlags = 0;
//	//description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//	description.BindFlags = 0;
//	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
//	description.Usage = D3D11_USAGE_STAGING;
//
//	auto res = m_Device->CreateTexture2D(&description, NULL, &grabTexture);
//
//	printf("grabTexture %p res %x S_OK %x\n", grabTexture, res, S_OK);
//
//	res = m_Device->CreateShaderResourceView(
//		grabTexture,
//		NULL,
//		&resourceView);
//	printf("resource view %p res %x", resourceView, res);
//
//	return resourceView;
//}



void ssMap()
{
	D3D11_MAPPED_SUBRESOURCE resource;
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);


	auto res = context->Map(screenCopyTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	printf("RowPitch %d DepthPitch %d", resource.RowPitch, resource.DepthPitch);

	auto hFile = CreateFile(L"C:\\Users\\brab\\hippscreen.txt",                // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		OPEN_ALWAYS,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(L"invalid file! ");
		auto err = GetLastError();
		return;
	}

	DWORD dwBytesWritten = 0;

	auto  bErrorFlag = WriteFile(
		hFile,           // open file handle
		resource.pData,      // start of data to write
		resource.RowPitch * screenHeight,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);



	CloseHandle(hFile);

	//memcpy(rawBuffer, resource.pData, screenWidth * screenHeight * 4);

	context->Unmap(screenCopyTexture, subresource);
}



static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	printf("eventID %d\n", eventID);

	switch (eventID)
	{
	case 1:
		CopyScreen();
		break;
	case 2:
		ssMap();
		break;
	}
}



extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return OnRenderEvent;
}
