#include "d3dUtility.h"

//所有画图的操作都是在IDirect3DDevice9 中
IDirect3DDevice9* Device = 0;

//指定屏幕大小
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;

ID3DXMesh* Teapot = 0;
D3DMATERIAL9 TeapotMtrl;

IDirect3DVertexBuffer9* BkGndQuad = 0;
IDirect3DTexture9* BkGndTex = 0;
D3DMATERIAL9 BkGndMtrl;

//茶壶的偏移位置
D3DXVECTOR3 TeapotPosition(5.0f, -1.0f, -1.0f);

//为一个场景增加纹理的必要步骤是：
//1. 用纹理坐标指定的，创建物体的顶点。
//2. 用D3DXCreateTextureFromFile函数读取一个纹理到IDirect3DTexture9接口中。
//3. 设置缩小倍数，放大倍数以及mipmap过滤器。
//4. 在你绘制一个物体前，用IDirect3DDevice9::SetTexture设置与物体关联的纹理。

struct Vertex
{
	Vertex(float x, float y, float z, float nx,float ny,float nz, float u, float v)
	{
		_x = x;
		_y = y;
		_z = z;
		_nx = nx;
		_ny = ny;
		_nz = nz;
		_u = u;
		_v = v;
	}
	float _x, _y, _z, _nx, _ny, _nz, _u, _v;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

//计算三角形的法线
void ComputerNormal(D3DXVECTOR3* p0, D3DXVECTOR3* p1, D3DXVECTOR3* p2, D3DXVECTOR3* out)
{
	D3DXVECTOR3 u = *p1 - *p0;
	D3DXVECTOR3 v = *p2 - *p1;
	D3DXVec3Cross(out, &u, &v);
	D3DXVec3Normalize(out, out);
}

//准备程序需要用到的东西，包括资源的分配，检查设备能力，设置应用程序的状态
//创建顶点和索引缓存，锁定它们，把构成立方体的顶点写入顶点缓存，以及把定义立方体的三角形的索引写入索引缓存。
//然后把摄象机向后移动几个单位以便我们能够看见在世界坐标系中原点处被渲染的立方体。
bool Setup()
{
	TeapotMtrl = d3d::RED_MTRL;
	//TeapotMtrl.Diffuse.a = 0.5f;
	BkGndMtrl = d3d::WHITE_MTRL;

	D3DXCreateTeapot(Device, &Teapot, 0);

	//利用 CreateVertexBuffer 创建顶点
	Device->CreateVertexBuffer(
		6 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&BkGndQuad,
		0);

	//向立方体的顶点缓存填充数据
	Vertex* v;
	BkGndQuad->Lock(0, 0, (void**)&v, 0);

	// quad built from two triangles, note texture coordinates:
	v[0] = Vertex(-5.0f, -5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = Vertex(-5.0f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = Vertex(5.0f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = Vertex(-5.0f, -5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[4] = Vertex(5.0f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[5] = Vertex(5.0f, -5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	BkGndQuad->Unlock();

	D3DXCreateTextureFromFile(
		Device,
		"mirror.bmp",
		&BkGndTex);

	Device->SetTexture(0, BkGndTex);

	//纹理被映射到屏幕中的三角形上。通常纹理三角形和屏幕三角形是不一样大的。
	//当纹理三角形比屏幕三角形小时，纹理三角形会被适当放大。当纹理三角形比屏幕三角形大时，纹理三角形会被适当缩小。
	//这两种情况，变形都将会出现。过滤（ Filtering）是一种Direct3D用它来帮助这些变形变的平滑的技术。
	//D3DTEXF_LINEAR——这种过滤产生还算比较好的效果，在今天的硬件上处理它还是非常快的。
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//为了使这个大小差异变小，	我们为纹理创建mipmaps链。
	//也就是说将一个纹理创建成连续的变小的纹理，但是对它们等级进行定制过滤，因此对我们来说保存细节是很重要的
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	//循环寻址方式
	Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);

	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//允许使用灯光,默认设置
	Device->SetRenderState(D3DRS_LIGHTING, true);
	//创建一个沿x轴照射的方向光
	D3DLIGHT9 dir;
	::ZeroMemory(&dir, sizeof(dir));
	dir.Type = D3DLIGHT_DIRECTIONAL;
	dir.Diffuse = d3d::WHITE;
	dir.Specular = d3d::WHITE * 0.3f;
	dir.Ambient = d3d::WHITE*0.6f;
	dir.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	Device->SetLight(0, &dir);
	Device->LightEnable(0, true);

	//照相机位置（视图矩阵）
	D3DXVECTOR3 position(-0.8f, 0.5f, -5.0f);//camera在世界坐标系中的位置向量
	//D3DXVECTOR3 position(-3.8f, 0.0f, -1.0f);//camera在世界坐标系中的位置向量
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);//target是camera的朝向向量
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//定义向上的方向，一般是[0,1,0]
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	//视图坐标系变换
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

	return true;
		
}

////释放Setup中分配的资源，比如内存
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(BkGndQuad);
}

void RenderMirror()
{
	//设置允许模板缓存和设置渲染状态
	Device->SetRenderState(D3DRS_STENCILENABLE, true);
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);//让所有模板测试都通过
	Device->SetRenderState(D3DRS_STENCILREF, 0x1);//测试成功，设置为0x1
	Device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);//深度测试失败了，不更新模板缓存入口
	Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);//模板测试失败了，不更新（其实不会失败）
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);//深度测试和模板测试都成功，更新模板缓存入口

	//设置深度缓存不可写
	Device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	//在模板缓存中渲染镜子
	Device->SetStreamSource(0, BkGndQuad, 0, sizeof(Vertex));
	Device->SetFVF(Vertex::FVF);
	Device->SetMaterial(&BkGndMtrl);
	Device->SetTexture(0, BkGndTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	Device->SetTransform(D3DTS_WORLD, &I);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	//设置深度缓存可写
	Device->SetRenderState(D3DRS_ZWRITEENABLE, true);

	//设置只绘制在镜子中的像元
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	//计算在场景中反射位置的矩阵
	D3DXMATRIX W, T, R;
	D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f);//xy平面
	D3DXMatrixReflect(&R, &plane);
	D3DXMatrixTranslation(&T,
		TeapotPosition.x,
		TeapotPosition.y,
		TeapotPosition.z);
	W = T*R;
	//被反射的茶壶的深度比镜子的深度大,需要清除深度缓存
	Device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	//混合镜子和反射的茶壶（不知道为什么要这么混合）
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	//绘制被反射的茶壶
	D3DXMATRIX WS;
	D3DXMatrixScaling(&WS, 0.2f, 0.2f, 0.2f);
	Device->SetTransform(D3DTS_WORLD, &WS);
	Device->SetTransform(D3DTS_WORLD, &W);
	Device->SetMaterial(&TeapotMtrl);
	Device->SetTexture(0, 0);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);//改变背面拣选模式
	Teapot->DrawSubset(0);

	//复原状态
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	Device->SetRenderState(D3DRS_STENCILENABLE, false);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

//绘图和显示的代码，timeDelta为每一帧的时间间隔，用来控制每秒的帧数
bool Display(float timeDelta)
{
	if (Device)
	{
		
		
		if (::GetAsyncKeyState('A') & 0x8000f)
		{
			//_cprintf("A 0x%x\n", ::GetAsyncKeyState('A'));
			TeapotPosition.x -= 0.01f;
		}
		if (::GetAsyncKeyState('D') & 0x8000f)
		{
			//_cprintf("A 0x%x\n", ::GetAsyncKeyState('A'));
			TeapotPosition.x += 0.01f;
		}
		if (::GetAsyncKeyState('W') & 0x8000f)
		{
			//_cprintf("A 0x%x\n", ::GetAsyncKeyState('A'));
			TeapotPosition.y += 0.01f;
		}
		if (::GetAsyncKeyState('S') & 0x8000f)
		{
			//_cprintf("A 0x%x\n", ::GetAsyncKeyState('A'));
			TeapotPosition.y -= 0.01f;
		}

		//清空目标缓存和深度缓存，把屏幕背景填充成白色
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
			0xffffffff, 1.0f, 0);

		Device->BeginScene();

		
		D3DXMATRIX W,T;
		D3DXMatrixIdentity(&W);//定义单位矩阵
		Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetStreamSource(0, BkGndQuad, 0, sizeof(Vertex));
		Device->SetFVF(Vertex::FVF);
		Device->SetMaterial(&BkGndMtrl);
		Device->SetTexture(0, BkGndTex);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		D3DXMatrixScaling(&W, 0.2f, 0.2f, 0.2f); //缩放
		D3DXMatrixTranslation(&T,
			TeapotPosition.x,
			TeapotPosition.y,
			TeapotPosition.z);
		Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetTransform(D3DTS_WORLD, &T);
		Device->SetMaterial(&TeapotMtrl);
		Device->SetTexture(0, 0);
		Teapot->DrawSubset(0);
		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		RenderMirror();

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
	AllocConsole();
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
	FreeConsole();
	return 0;
}
