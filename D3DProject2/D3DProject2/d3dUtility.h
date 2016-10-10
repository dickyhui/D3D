#ifndef __d3dUtilityH__
#define __d3dUitlityH__

#include <d3dx9.h>
#include <conio.h>
//#include <iostream>
//#include <d3d9.h>
//#include <Mmsystem.h> 
//#pragma comment(lib, "Winmm.lib")  

namespace d3d
{
	bool InitD3D(
		HINSTANCE hInstance,
		int width, int height,
		bool windowed,
		D3DDEVTYPE deviceType,
		IDirect3DDevice9** device);

	int EnterMsgLoop(
		bool(*ptr_display)(float timeDelta));

	LRESULT CALLBACK WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM IParam);

	template<class T> void Release(T t)
	{
		if (t)
		{
			t->Release();
			t = 0;
		}
	}

	template<class T> void Delete(T t)
	{
		if (t)
		{
			delete t;
			t = 0;
		}
	}
}

#endif