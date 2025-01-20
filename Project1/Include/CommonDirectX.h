#pragma once
#ifndef PROJECT_COMMON_DIRECTX_H
#define PROJECT_COMMON_DIRECTX_H

#include <Windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <wincodec.h>
#include <string>
#include <Settings.h>

#include <SpriteEnum.h>

#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")

#define DebugLog(wstr) OutputDebugString(wstr)


template<class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

#ifdef SETTING_DEBUG

class Reference
{
protected:

	UINT RefCount { 0 };
	Reference()
	{
		AddRef();
	};
	virtual ~Reference(){};

public:

	void AddRef();
	void Release();

	inline static int InstanceCount = 0;
	void printCount();
};

#else

class Reference
{
protected:

	UINT RefCount { 0 };
	Reference()
	{
		AddRef();
	};
	virtual ~Reference(){};

public:

	void AddRef();
	void Release();

	inline static int InstanceCount = 0;
	void printCount();
};

#endif

#endif