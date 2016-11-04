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
	����Ӧ�ó����������   
	1������WNDCLASS(WNDCLASSEX)���崰�ڶ���
	2������RegisterClass(RegisterClassEx)����ע�ᴰ�ڶ���
	3������CreateWindow(CreateWindowEx)���������Ѿ����岢��ע��Ĵ��ڶ���
	4������ShowWindow(hwnd,SW_SHOWNORMAL)���ô�����ϸ��ʾ״̬;
	5������UpdateWindow(hwnd)ˢ����ʾ����;
	*/

	//1������WNDCLASS(WNDCLASSEX)���崰�ڶ���
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

	//2������RegisterClass(RegisterClassEx)����ע�ᴰ�ڶ���
	//RegisterClass��������ʲô������
	//RegisterClass������������֪ͨϵͳ����Ҫ����һ���µĴ������ͣ�Ȼ���������ͼ�¼��ϵͳ���棬�Ժ���Ϳ���ʹ��CreateWindow������һ�����ڴ����͵Ĵ��塣���ڴ����͵Ĵ��嶼������ͬ�����ԣ����磬����ɫ����꣬ͼ��ȵ�
	//���ڶԻ�����ԣ�ϵͳ�Ѿ�ע���˶Ի����Լ������ͣ�������������RegisterClass�Ϳ���ʹ��DialogBox��ʾģ̬�Ի���ʹ��CreateDialog������ģ̬�Ի���
	//���ڿؼ����ԣ�ϵͳ�Ѿ�ע���˶Ի����Լ������ͣ������Ҳ�������RegisterClass�Ϳ���ʹ��CreateWindow�������ؼ�����CreateWindow�еĵ�һ������������RegisterClass����ʹ�õ�WNDCLASS�ṹ�еĳ�ԱlpszClassName��
	//�ؼ���Ի�����������ڣ��Ի���Ĵ�������ɵ��ó����ṩ�����ؼ��Ĵ�������Ƕ�����ϵͳ�еģ������Դ���������໯�����ܶ��ƿؼ��Ĵ�����Ϣ�߼���
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}

	//3������CreateWindow(CreateWindowEx)���������Ѿ����岢��ע��Ĵ��ڶ���
	//Ϊʲô��ҪCreateWindow������
	//RegisterClass�����������Ƕ���һ�������࣬�����C++�е�class�����CreateWindow��������Ƕ������������͵Ķ��������C++�еĶ�����
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

	//4������ShowWindow(hwnd,SW_SHOWNORMAL)���ô�����ϸ��ʾ״̬;
	//5������UpdateWindow(hwnd)ˢ����ʾ����;
	//���CreateWindow�Ǵ��������õģ���Ϊʲô��ҪShowWindow��UpdateWindow �أ�
	//����ʹ��CreateWindow����ʱ�����ָ����WS_VISIBLE��־����ô���������ShowWindow�����Ϳ�����ʾ���壬������ͱ���ʹ��ShowWindow������
	//UpdateWindow�����������Ƿ���һ��WM_PAINT��Ϣ��������̣��ô����ڴ�����֮�����̽��л��ơ����û��UpdateWindow��������ôֻ�е����崴����֮�� ���Ŀ����������仯�ˣ��Ż���ӦWM_PAINT.
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	/*
	��ʼ��D3D
	1�����һ��Idirect3D9�ӿ�ָ�롣����ӿ����ڻ�������豸����Ϣ�ʹ���һ��IDirect3DDecive9�ӿڡ�
	2������豸������D3DCAPS9����������Կ��Ƿ�֧��Ӳ�����㴦��
	3����ʼ��һ��D3DPRESENT_PARAMETERS�ṹʵ��������ṹ��������ഴ��IDirect3DDevice9�ӿڵ����ݳ�Ա
	4������һ�������Ѿ���ʼ���õ�D3DPRESENT_PARAMETERS�ṹ��IDIRECT3DDeviece9��������һ������������ʾ3Dͼ�ε������豸��C++����
	*/
	HRESULT hr = 0;

	//1�����һ��Idirect3D9�ӿ�ָ�롣����ӿ����ڻ�������豸����Ϣ�ʹ���һ��IDirect3DDecive9�ӿڡ� 
	//Direct3DCreate9��Ψһһ����������D3D_SDK_VERSION������Ա�֤Ӧ�ó���ͨ����ȷ��ͷ�ļ������ɣ������������ʧ�ܣ�����һ����ָ��
	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0);
		return false;
	}

	//2������豸������D3DCAPS9����������Կ��Ƿ�֧��Ӳ�����㴦��

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

	//3����ʼ��һ��D3DPRESENT_PARAMETERS�ṹʵ��������ṹ��������ഴ��IDirect3DDevice9�ӿڵ����ݳ�Ա  

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

	//4������һ�������Ѿ���ʼ���õ�D3DPRESENT_PARAMETERS�ṹ��IDIRECT3DDeviece9��������һ������������ʾ3Dͼ�ε������豸��C++����  

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
	//ZeroMemory��һ���ꡣ ����������0�����һ���ڴ�����
	//������Destination :ָ��һ��׼����0�������ڴ�����Ŀ�ʼ��ַ��Length :׼����0�������ڴ�����Ĵ�С�����ֽ������㡣
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));

	//����ϵͳʱ�䣬�Ժ���Ϊ��λ��ϵͳʱ���Ǵ�ϵͳ���������ú���ʱ�������ĺ�������
	static float lastTime = (float)timeGetTime();

	//Win32�����������̨(Console)�������������Ϣ
	//#include <conio.h> AllocConsole();_cprintf("i=%d\n", i);FreeConsole();
	//AllocConsole();
	while (msg.message != WM_QUIT)
	{
		//PeekMessageΪһ����Ϣ����߳���Ϣ���У���������Ϣ��������ڣ�����ָ���Ľṹ��
		//��GetMessage��֮ͬ�����ڣ�PeekMessage����ȴ���Ϣ�����ǲ���ϵط�����Ϣ���У�������Ϣ���е�Ŀǰ״̬��Ρ�
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			//���������Ϣת��Ϊ�ַ���Ϣ���ַ���Ϣ���͵������̵߳���Ϣ�����У�����һ���̵߳��ú���GetMessage��PeekMessageʱ��������
			::TranslateMessage(&msg);
			//_cprintf("i=%d\n", msg.message);
			//�ú�������һ����Ϣ�����ڳ���ͨ�����ȴ�GetMessage��PeekMessageȡ�õ���Ϣ����Ϣ�����ȵ��Ĵ��ڳ�����WndProc()������
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