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

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	FILE * pConsole;
	AllocConsole();
	freopen_s(&pConsole, "CONOUT$", "wb", stdout);

	IUnityGraphicsD3D11* d3d = unityInterfaces->Get<IUnityGraphicsD3D11>();
	m_Device = d3d->GetDevice();
	m_Device->GetImmediateContext(&context);

	printf("loading context %p\n", context);
}


// If exported by a plugin, this function will be called when the plugin is about to be unloaded.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
}

extern "C" UNITY_INTERFACE_EXPORT ID3D11ShaderResourceView* UNITY_INTERFACE_API MakeGrabTexture(UINT width, UINT height)
{
	screenWidth = width;
	screenHeight = height;

	printf("creating grab texture %d %d\n", screenWidth, screenHeight);
	D3D11_TEXTURE2D_DESC description;

	description.Width = width;
	description.Height = height;
	description.MipLevels = 1;
	description.ArraySize = 1;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.MiscFlags = 0;
	//description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.Usage = D3D11_USAGE_STAGING;

	auto res = m_Device->CreateTexture2D(&description, NULL, &grabTexture);

	printf("grabTexture %p res %x S_OK %x\n", grabTexture, res, S_OK);

	res = m_Device->CreateShaderResourceView(
		grabTexture,
		NULL,
		&resourceView);
	printf("resource view %p res %x", resourceView, res);

	return resourceView;
}

void ssMap()
{
	printf("mapping...\n");
	D3D11_MAPPED_SUBRESOURCE resource;
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);

	printf("Map context %p grabTexture %p subresource %d\n", context, grabTexture, subresource);
	//auto res = context->Map(grabTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
	//ID3D11Resource *hmm;
	printf("resourceView %p", resourceView);
	//resourceView->GetResource(&hmm);
	//printf("get resource %p\n", hmm);
	return;

    //auto res = context->Map(hmm, subresource, D3D11_MAP_READ_WRITE, 0, &resource);


	//printf("mappped res %x\n", res);
	//	D3D11_TEXTURE2D_DESC texDesc;

	//	pNewTexture->GetDesc(&texDesc);

	printf("open file\n");
	auto hFile = CreateFile(L"C:\\Users\\brab\\hej.txt",                // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		OPEN_ALWAYS,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("invalid file! %d", GetLastError());
		return;
	}

	printf("write file\n");

	DWORD dwBytesWritten = 0;
	auto  bErrorFlag = WriteFile(
		hFile,           // open file handle
		resource.pData,      // start of data to write
		screenWidth * screenHeight * 4,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);

	//if ()

	CloseHandle(hFile);

	printf("mapping done\n");

}



static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	switch (eventID)
	{
	case 1:
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
