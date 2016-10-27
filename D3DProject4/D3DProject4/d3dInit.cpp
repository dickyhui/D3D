#include "d3dUtility.h"

//所有画图的操作都是在IDirect3DDevice9 中
IDirect3DDevice9* Device = 0;

//指定屏幕大小
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;
//IDirect3DVertexBuffer9* Triangle = 0;
IDirect3DVertexBuffer9* Pyramid = 0;

//声明茶壶的网格
ID3DXMesh* mesh = 0;

/*给场景增加灯光的步骤是：
1、 允许使用灯光。
2、 为每个物体创建材质并且在渲染相应物体前应将材质附予物体。
3、 创建一个或多个光源，设置它们，把它们设为可用。
4、 将其他附加光源设为可用，比如镜面高光。*/



struct Vertex
{
	Vertex(float x, float y, float z, float nx,float ny,float nz)
	{
		_x = x;
		_y = y;
		_z = z;
		_nx = nx;
		_ny = ny;
		_nz = nz;
	}
	float _x, _y, _z, _nx, _ny, _nz;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL;

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
	//创建茶壶网格
	D3DXCreateTeapot(Device, &mesh, 0);

	//允许使用灯光,默认设置
	Device->SetRenderState(D3DRS_LIGHTING, true);
	//利用 CreateVertexBuffer 创建顶点
	Device->CreateVertexBuffer(
		12 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&Pyramid,
		0);

	//向立方体的顶点缓存填充数据
	Vertex* v;
	Pyramid->Lock(0, 0, (void**)&v, 0);

	// front face
	v[0] = Vertex(-1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);
	v[1] = Vertex(0.0f, 1.0f, 0.0f, 0.0f, 0.707f, -0.707f);
	v[2] = Vertex(1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);
	// left face
	v[3] = Vertex(-1.0f, 0.0f, 1.0f, -0.707f, 0.707f, 0.0f);
	v[4] = Vertex(0.0f, 1.0f, 0.0f, -0.707f, 0.707f, 0.0f);
	v[5] = Vertex(-1.0f, 0.0f, -1.0f, -0.707f, 0.707f, 0.0f);
	// right face
	v[6] = Vertex(1.0f, 0.0f, -1.0f, 0.707f, 0.707f, 0.0f);
	v[7] = Vertex(0.0f, 1.0f, 0.0f, 0.707f, 0.707f, 0.0f);
	v[8] = Vertex(1.0f, 0.0f, 1.0f, 0.707f, 0.707f, 0.0f);
	// back face
	v[9] = Vertex(1.0f, 0.0f, 1.0f, 0.0f, 0.707f, 0.707f);
	v[10] = Vertex(0.0f, 1.0f, 0.0f, 0.0f, 0.707f, 0.707f);
	v[11] = Vertex(-1.0f, 0.0f, 1.0f, 0.0f, 0.707f, 0.707f);

	Pyramid->Unlock();

	//定义材质，反射出白光，自身不发光，且会产生一些高光
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = d3d::WHITE;//环境光
	mtrl.Diffuse = d3d::WHITE;//漫色光
	mtrl.Specular = d3d::WHITE;//镜面光
	mtrl.Emissive = d3d::RED;//自发光
	mtrl.Power = 5.0f;//高光
	Device->SetMaterial(&mtrl);

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

	//设置状态使法线从新规格化且把镜面高光设置为可用
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);


	//照相机位置（视图矩阵）
	D3DXVECTOR3 position(2.0f, 2.0f, -2.0f);//camera在世界坐标系中的位置向量
	D3DXVECTOR3 target(-2.0f, -2.0f, 2.0f);//target是camera的朝向向量
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//定义向上的方向，一般是[0,1,0]
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	//视图坐标系变换
	Device->SetTransform(D3DTS_VIEW, &V);

	//背面拣选，按D3DCULL_CCW逆时针方向进行剔除（这是DX的默认剔除方式）
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//投影矩阵
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI*0.5f,
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	/*device->setrenderstate(d3drs_cullmode, d3dcull_none);
	device->setrenderstate(d3drs_fillmode, d3dfill_wireframe);*/

	return true;
		
}

////释放Setup中分配的资源，比如内存
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(Pyramid);
	mesh->Release();
	mesh = 0;
}

//绘图和显示的代码，timeDelta为每一帧的时间间隔，用来控制每秒的帧数
bool Display(float timeDelta)
{
	if (Device)
	{
		//清空目标缓存和深度缓存，把屏幕背景填充成白色
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0xffffffff, 1.0f, 0);

		Device->BeginScene();
		//设置茶壶的偏移
		D3DXMatrixTranslation(&World, 1.0f, 0.0f, -0.5f);
		Device->SetTransform(D3DTS_WORLD, &World);
		//绘制茶壶
		mesh->DrawSubset(0);
		//设置金字塔的偏移
		D3DXMatrixTranslation(&World, -1.5f, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &World);
		//绘制准备
		//把 vertex buffer 中的内容放到一个 stream 中， stream会最终把几何图型渲染成为图像
		Device->SetStreamSource(0, Pyramid, 0, sizeof(Vertex));
		//设置点的格式， 利用SetFVF函数
		Device->SetFVF(Vertex::FVF);

		//绘制金字塔
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 4);

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
