#include "d3dUtility.h"

//所有画图的操作都是在IDirect3DDevice9 中
IDirect3DDevice9* Device = 0;

//定义两个全局变量来保存立方体的顶点和索引数据
IDirect3DVertexBuffer9* VB = 0;
IDirect3DIndexBuffer9* IB = 0;

//指定屏幕大小
const int Width = 800;
const int Height = 600;

//定义顶点结构以及结构中顶点的格式，这里只保存了顶点的位置信息
struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z)
	{
		_x = x; _y = y; _z = z;
	}
	float _x, _y, _z;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ;

//准备程序需要用到的东西，包括资源的分配，检查设备能力，设置应用程序的状态
//创建顶点和索引缓存，锁定它们，把构成立方体的顶点写入顶点缓存，以及把定义立方体的三角形的索引写入索引缓存。
//然后把摄象机向后移动几个单位以便我们能够看见在世界坐标系中原点处被渲染的立方体。
bool Setup()
{
	//利用 CreateVertexBuffer 和 CreateIndexBuffer 创建顶点和索引缓存
	Device->CreateVertexBuffer(
		8 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Device->CreateIndexBuffer(
		36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		0);

	//向立方体的顶点缓存填充数据
	Vertex* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	//设置顶点的数据
	vertices[0] = Vertex(-1.0f, -1.0f, -1.0f);
	vertices[1] = Vertex(-1.0f, 1.0f, -1.0f);
	vertices[2] = Vertex(1.0f, 1.0f, -1.0f);
	vertices[3] = Vertex(1.0f, -1.0f, -1.0f);
	vertices[4] = Vertex(-1.0f, -1.0f, 1.0f);
	vertices[5] = Vertex(-1.0f, 1.0f, 1.0f);
	vertices[6] = Vertex(1.0f, 1.0f, 1.0f);
	vertices[7] = Vertex(1.0f, -1.0f, 1.0f);

	VB->Unlock();

	//定义立方体的三角形
	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	//前面
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	//背面
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	//左面
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	//右面
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	//上面
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	//下面
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;

	IB->Unlock();

	//照相机位置（视图矩阵）
	D3DXVECTOR3 position(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);

	Device->SetTransform(D3DTS_VIEW, &V);

	//投影矩阵
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI*0.5f,
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//渲染状态（填充模式，框架填充）
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	return true;
		
}

////释放Setup中分配的资源，比如内存
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(VB);
	d3d::Release<IDirect3DIndexBuffer9*>(IB);
}

//绘图和显示的代码，timeDelta为每一帧的时间间隔，用来控制每秒的帧数
bool Display(float timeDelta)
{
	if (Device)
	{
		//旋转立方体
		D3DXMATRIX Rx, Ry;
		//x轴旋转45弧度
		D3DXMatrixRotationX(&Rx, 3.14f / 4.0f);

		//每一帧中增加y轴的弧度
		static float y = 0.0f;
		D3DXMatrixRotationY(&Ry, y);
		y += timeDelta;

		//当y轴旋转2周时，重新回到0弧度
		if (y >= 6.28f)
			y = 0.0f;

		//结合x轴与y轴的选择矩阵
		D3DXMATRIX p = Rx*Ry;
		Device->SetTransform(D3DTS_WORLD, &p);

		//清空目标缓存和深度缓存，把屏幕背景填充成白色
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0xffffffff, 1.0f, 0);

		Device->BeginScene();

		//把 vertex buffer 中的内容放到一个 stream 中， 这是stream会最终把几何图型渲染成为图像
		Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
		//设置 index buffer
		Device->SetIndices(IB);
		//设置点的格式， 利用SetFVF函数
		Device->SetFVF(Vertex::FVF);

		//绘制三角形
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);

	}
	return true;
}

//Windows通过调用窗口消息处理程序对窗口发送消息
//在第一次建立窗口时、在窗口关闭时、窗口改变大小、移动或者变成图标时，从菜单中选择某一项目、挪动滚动条、按下鼠标按钮或者从键盘输入字符时，以及窗口客户区必须被更新时，Windows都要调用WndProc
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM IParam)
{
	switch (msg)
	{
	//WM_CLOSE 消息发出的时候，用户可以根据自己的意愿来选择到底是否关闭，WM_DESTORY 是真的关闭一个窗口,WM_QUIT是退出一个应用程序
	case WM_DESTROY:
		//该函数向系统表明有个线程有终止请求。通常用来响应WM_DESTROY消息。
		//PostQuitMessage寄送一个WM_QUIT消息给线程的消息队列并立即返回；此函数向系统表明有个线程请求在随后的某一时间终止。
		::PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			//销毁指定的窗口。这个函数通过发送WM_DESTROY 消息和 WM_NCDESTROY 消息使窗口无效并移除其键盘焦点。
			//这个函数还销毁窗口的菜单，清空线程的消息队列，销毁与窗口过程相关的定时器，解除窗口对剪贴板的拥有权，打断剪贴板器的查看链。
			::DestroyWindow(hwnd);
		break;
	}
	//DefWindowProc函数调用缺省的窗口过程来为应用程序没有处理的任何窗口消息提供缺省的处理。该函数确保每一个消息得到处理。
	return ::DefWindowProc(hwnd, msg, wParam, IParam);
}

//应用程序主入口
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	//初始化一个应用程序的主窗口并进行Direct3D的初始化
	if (!d3d::InitD3D(hinstance, 800, 600, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "INitD3D() - FAILED", 0, 0);
		return 0;
	}

	//准备程序需要用到的东西，包括资源的分配，检查设备能力，设置应用程序的状态
	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	//程序的消息循环，他需要输入一个显示函数指针，这样做是为了使显示函数能够在空闲的时候被调用并显示场景
	d3d::EnterMsgLoop(Display);

	//释放Setup中分配的资源，比如内存
	Cleanup();

	Device->Release();
	return 0;
}
