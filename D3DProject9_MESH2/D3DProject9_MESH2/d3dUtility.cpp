#include "d3dUtility.h"   

D3DMATERIAL9 d3d::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;
	mtrl.Diffuse = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power = p;
	return mtrl;
}

d3d::BoundingBox::BoundingBox()
{
	// infinite small bounding box
	_min.x = d3d::INFINITYNUM;
	_min.y = d3d::INFINITYNUM;
	_min.z = d3d::INFINITYNUM;
	_max.x = -d3d::INFINITYNUM;
	_max.y = -d3d::INFINITYNUM;
	_max.z = -d3d::INFINITYNUM;
}
bool d3d::BoundingBox::isPointInside(D3DXVECTOR3& p)
{
	// is the point inside the bounding box?
	if (p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
		p.x <= _max.x && p.y <= _max.y && p.z <= _max.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

d3d::BoundingSphere::BoundingSphere()
{
	_radius = 0.0f;
}

bool d3d::Equals(float lhs, float rhs)
{
	return fabs(lhs - rhs) < d3d::EPSILON ? true : false;
}

bool d3d::InitD3D(
	HINSTANCE hInstance,
	int width, int height,
	bool windowed,
	D3DDEVTYPE deviceType,
	IDirect3DDevice9** device)
{
	/*
	创建应用程序的主窗口   
	1、利用WNDCLASS(WNDCLASSEX)定义窗口对象；
	2、调用RegisterClass(RegisterClassEx)函数注册窗口对象；
	3、调用CreateWindow(CreateWindowEx)方法创建已经定义并且注册的窗口对象；
	4、调用ShowWindow(hwnd,SW_SHOWNORMAL)设置窗口详细显示状态;
	5、调用UpdateWindow(hwnd)刷新显示窗口;
	*/

	//1、利用WNDCLASS(WNDCLASSEX)定义窗口对象；
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D9App";

	//2、调用RegisterClass(RegisterClassEx)函数注册窗口对象；
	//RegisterClass到底做了什么工作？
	//RegisterClass函数的作用是通知系统，你要定义一个新的窗体类型，然后把这个类型记录到系统里面，以后你就可以使用CreateWindow来创建一个基于此类型的窗体。基于此类型的窗体都具有相同的属性，比如，背景色，光标，图标等等
	//对于对话框而言，系统已经注册了对话框自己的类型，因此你无需调用RegisterClass就可以使用DialogBox显示模态对话框，使用CreateDialog创建非模态对话框
	//对于控件而言，系统已经注册了对话框自己的类型，因此你也无需调用RegisterClass就可以使用CreateWindow来创建控件，而CreateWindow中的第一个参数，正是RegisterClass函数使用的WNDCLASS结构中的成员lpszClassName。
	//控件与对话框的区别在于，对话框的窗体过程由调用程序提供，而控件的窗体过程是定义在系统中的，你必须对窗体进行子类化，才能定制控件的处理消息逻辑。
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}

	//3、调用CreateWindow(CreateWindowEx)方法创建已经定义并且注册的窗口对象；
	//为什么需要CreateWindow函数？
	//RegisterClass函数的作用是定义一个窗体类，相对于C++中的class概念，而CreateWindow这个函数是定义基于这个类型的对象，相对于C++中的对象概念。
	HWND hwnd = 0;
	hwnd = ::CreateWindow("Direct3D9App", "Direct3D9App",
		WS_EX_TOPMOST,
		0, 0, width, height,
		0 /*parent hwnd*/, 0 /* menu */, hInstance, 0 /*extra*/);

	if (!hwnd)
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0);
		return false;
	}

	//4、调用ShowWindow(hwnd,SW_SHOWNORMAL)设置窗口详细显示状态;
	//5、调用UpdateWindow(hwnd)刷新显示窗口;
	//如果CreateWindow是创建窗口用的，那为什么需要ShowWindow和UpdateWindow 呢？
	//当你使用CreateWindow函数时，如果指定了WS_VISIBLE标志，那么就无需调用ShowWindow函数就可以显示窗体，否则，你就必须使用ShowWindow函数。
	//UpdateWindow函数的作用是发送一条WM_PAINT消息给窗体过程，让窗体在创建完之后，立刻进行绘制。如果没有UpdateWindow函数，那么只有当窗体创建完之后， 它的可视区域发生变化了，才会响应WM_PAINT.
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	/*
	初始化D3D
	1、获得一个Idirect3D9接口指针。这个接口用于获得物理设备的信息和创建一个IDirect3DDecive9接口。
	2、检查设备能力（D3DCAPS9），搞清楚显卡是否支持硬件顶点处理。
	3、初始化一个D3DPRESENT_PARAMETERS结构实例，这个结构包含了许多创建IDirect3DDevice9接口的数据成员
	4、创建一个基于已经初始化好的D3DPRESENT_PARAMETERS结构的IDIRECT3DDeviece9对象，它是一个代表我们显示3D图形的物理设备的C++对象。
	*/
	HRESULT hr = 0;

	//1、获得一个Idirect3D9接口指针。这个接口用于获得物理设备的信息和创建一个IDirect3DDecive9接口。 
	//Direct3DCreate9的唯一一个参数总是D3D_SDK_VERSION，这可以保证应用程序通过正确的头文件被生成，如果函数调用失败，返回一个空指针
	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0);
		return false;
	}

	//2、检查设备能力（D3DCAPS9），搞清楚显卡是否支持硬件顶点处理。

	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		_cprintf("vp = D3DCREATE_HARDWARE_VERTEXPROCESSING");
	}
		
	else
	{
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		_cprintf("vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING");
	}

	//3、初始化一个D3DPRESENT_PARAMETERS结构实例，这个结构包含了许多创建IDirect3DDevice9接口的数据成员  

	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = windowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//4、创建一个基于已经初始化好的D3DPRESENT_PARAMETERS结构的IDIRECT3DDeviece9对象，它是一个代表我们显示3D图形的物理设备的C++对象。  

	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter   
		deviceType,         // device type   
		hwnd,               // window associated with device   
		vp,                 // vertex processing   
		&d3dpp,             // present parameters   
		device);            // return created device   

	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer   
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			deviceType,
			hwnd,
			vp,
			&d3dpp,
			device);

		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object   
			::MessageBox(0, "CreateDevice() - FAILED", 0, 0);
			return false;
		}
	}

	d3d9->Release(); // done with d3d9 object   

	return true;
}

int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta))
{
	//ZeroMemory是一个宏。 其作用是用0来填充一块内存区域。
	//参数：Destination :指向一块准备用0来填充的内存区域的开始地址；Length :准备用0来填充的内存区域的大小，按字节来计算。
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));

	//返回系统时间，以毫秒为单位。系统时间是从系统启动到调用函数时所经过的毫秒数。
	static float lastTime = (float)timeGetTime();

	//Win32程序中向控制台(Console)窗口输出调试信息
	//#include <conio.h> AllocConsole();_cprintf("i=%d\n", i);FreeConsole();
	//AllocConsole();
	while (msg.message != WM_QUIT)
	{
		//PeekMessage为一个消息检查线程消息队列，并将该消息（如果存在）放于指定的结构。
		//跟GetMessage不同之处在于：PeekMessage不会等待消息，而是不间断地访问消息队列，不管消息队列的目前状态如何。
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			//将虚拟键消息转换为字符消息。字符消息被送到调用线程的消息队列中，在下一次线程调用函数GetMessage或PeekMessage时被读出。
			::TranslateMessage(&msg);
			//_cprintf("i=%d\n", msg.message);
			//该函数调度一个消息给窗口程序。通常调度从GetMessage或PeekMessage取得的消息。消息被调度到的窗口程序即是WndProc()函数。
			::DispatchMessage(&msg);
		}
		else
		{
			float currTime = (float)timeGetTime();
			float timeDelta = (currTime - lastTime)*0.001f;

			ptr_display(timeDelta);
			//_cprintf("i=%f\n", timeDelta);
			lastTime = currTime;
		}
	}
	//FreeConsole();
	return msg.wParam;
}