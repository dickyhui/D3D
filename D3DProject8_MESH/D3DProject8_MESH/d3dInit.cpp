#include "d3dUtility.h"

//所有画图的操作都是在IDirect3DDevice9 中
IDirect3DDevice9* Device = 0;

//指定屏幕大小
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;

ID3DXFont* font = 0;

ID3DXMesh* Mesh = 0;
const DWORD NumSubsets = 3;
IDirect3DTexture9* Textures[3] = { 0, 0, 0 };// texture for each subset
std::ofstream OutFile; //文件写操作 内存写入存储设备   

void dumpVertices(std::ofstream& outFile, ID3DXMesh* mesh);
void dumpIndices(std::ofstream& outFile, ID3DXMesh* mesh);
void dumpAttributeBuffer(std::ofstream& outFile, ID3DXMesh* mesh);
void dumpAdjacencyBuffer(std::ofstream& outFile, ID3DXMesh* mesh);
void dumpAttributeTable(std::ofstream& outFile, ID3DXMesh* mesh);

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

void CreateFont()
{
	D3DXFONT_DESC fontDesc;
	ZeroMemory(&fontDesc, sizeof(D3DXFONT_DESC));
	fontDesc.Height = 25; // in logical units
	fontDesc.Width = 12; // in logical units
	fontDesc.Weight = 500; // boldness, range 0(light) - 1000(bold)
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	strcpy_s(fontDesc.FaceName, "Times New Roman"); // font style
	D3DXCreateFontIndirect(Device, &fontDesc, &font);
}

//准备程序需要用到的东西，包括资源的分配，检查设备能力，设置应用程序的状态
//创建顶点和索引缓存，锁定它们，把构成立方体的顶点写入顶点缓存，以及把定义立方体的三角形的索引写入索引缓存。
//然后把摄象机向后移动几个单位以便我们能够看见在世界坐标系中原点处被渲染的立方体。
bool Setup()
{
	CreateFont();

	HRESULT hr = 0;

	//   
	// We are going to fill the empty mesh with the geometry of a box,   
	// so we need 12 triangles and 24 vetices.   
	//   

	hr = D3DXCreateMeshFVF(
		12,
		24,
		D3DXMESH_MANAGED,
		Vertex::FVF,
		Device,
		&Mesh);

	if (FAILED(hr))
	{
		::MessageBox(0, "D3DXCreateMeshFVF() - FAILED", 0, 0);
		return false;
	}

	//   
	// Fill in vertices of a box   
	//   
	Vertex* v = 0;
	Mesh->LockVertexBuffer(0, (void**)&v);

	// fill in the front face vertex data   
	v[0] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[1] = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[2] = Vertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	v[3] = Vertex(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	// fill in the back face vertex data   
	v[4] = Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[5] = Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = Vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[7] = Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// fill in the top face vertex data   
	v[8] = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[9] = Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[10] = Vertex(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	v[11] = Vertex(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	// fill in the bottom face vertex data   
	v[12] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[13] = Vertex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[15] = Vertex(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// fill in the left face vertex data   
	v[16] = Vertex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[17] = Vertex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[18] = Vertex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[19] = Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// fill in the right face vertex data   
	v[20] = Vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[21] = Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[22] = Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[23] = Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	Mesh->UnlockVertexBuffer();

	//   
	// Define the triangles of the box   
	//   
	WORD* i = 0;
	Mesh->LockIndexBuffer(0, (void**)&i);

	// fill in the front face index data   
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// fill in the back face index data   
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// fill in the top face index data   
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// fill in the bottom face index data   
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// fill in the left face index data   
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// fill in the right face index data   
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	Mesh->UnlockIndexBuffer();

	//   
	// Specify the subset each triangle belongs to, in this example   
	// we will use three subsets, the first two faces of the cube specified   
	// will be in subset 0, the next two faces will be in subset 1 and   
	// the the last two faces will be in subset 2.   
	//   
	DWORD* attributeBuffer = 0;
	Mesh->LockAttributeBuffer(0, &attributeBuffer);

	for (int a = 0; a < 4; a++)
		attributeBuffer[a] = 0;

	for (int b = 4; b < 8; b++)
		attributeBuffer[b] = 1;

	for (int c = 8; c < 12; c++)
		attributeBuffer[c] = 2;

	Mesh->UnlockAttributeBuffer();

	//   
	// Optimize the mesh to generate an attribute table.   
	//   

	std::vector<DWORD> adjacencyBuffer(Mesh->GetNumFaces() * 3);
	Mesh->GenerateAdjacency(0.0f, &adjacencyBuffer[0]);

	hr = Mesh->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT |
		D3DXMESHOPT_VERTEXCACHE,
		&adjacencyBuffer[0],
		0, 0, 0);

	//   
	// Dump the Mesh Data to file.   
	//   

	OutFile.open("Mesh Dump.txt");

	dumpVertices(OutFile, Mesh);
	dumpIndices(OutFile, Mesh);
	dumpAttributeTable(OutFile, Mesh);
	dumpAttributeBuffer(OutFile, Mesh);
	dumpAdjacencyBuffer(OutFile, Mesh);

	OutFile.close();

	//   
	// Load the textures and set filters.   
	//   

	D3DXCreateTextureFromFile(
		Device,
		"brick0.bmp",
		&Textures[0]);

	D3DXCreateTextureFromFile(
		Device,
		"brick1.bmp",
		&Textures[1]);

	D3DXCreateTextureFromFile(
		Device,
		"brick2.bmp",
		&Textures[2]);

	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	//    
	// Disable lighting.   
	//   

	Device->SetRenderState(D3DRS_LIGHTING, false);

	//   
	// Set camera.   
	//   

	D3DXVECTOR3 pos(0.0f, 0.f, -4.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	D3DXMATRIX V;
	D3DXMatrixLookAtLH(
		&V,
		&pos,
		&target,
		&up);

	Device->SetTransform(D3DTS_VIEW, &V);

	//   
	// Set projection matrix.   
	//   

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.5f, // 90 - degree   
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	return true;
		
}

////释放Setup中分配的资源，比如内存
void Cleanup()
{
}

DWORD FrameCnt;
float TimeElapsed;
float FPS;
void CalcFPS(float timeDelta)
{
	FrameCnt++;
	TimeElapsed += timeDelta;
	if (TimeElapsed >= 1.0f)
	{
		FPS = (float)FrameCnt / TimeElapsed;
		TimeElapsed = 0.0f;
		FrameCnt = 0;
	}
}

//绘图和显示的代码，timeDelta为每一帧的时间间隔，用来控制每秒的帧数
bool Display(float timeDelta)
{
	if (Device)
	{
		CalcFPS(timeDelta);
		RECT textRect;
		textRect.left = 0;
		textRect.right = 200;
		textRect.top = 0;
		textRect.bottom = 50;

		D3DXMATRIX xRot;
		D3DXMatrixRotationX(&xRot, D3DX_PI * 0.2f);

		static float y = 0.0f;
		D3DXMATRIX yRot;
		D3DXMatrixRotationY(&yRot, y);
		y += timeDelta;

		if (y >= 6.28f)
			y = 0.0f;

		D3DXMATRIX World = xRot * yRot;

		Device->SetTransform(D3DTS_WORLD, &World);

		//   
		// Render   
		//   

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
		Device->BeginScene();

		std::ostringstream fpsBuff;
		fpsBuff << FPS;
		std::string s(fpsBuff.str());
		s = "FPS: " + s;
		font->DrawText(0, s.c_str(), -1, &textRect, DT_TOP | DT_LEFT, 0xffff0000);//绘制fps文本

		for (int i = 0; i < NumSubsets; i++)
		{
			Device->SetTexture(0, Textures[i]);
			Mesh->DrawSubset(i);
		}

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


//   
// Prototype Implementations   
//   

void dumpVertices(std::ofstream& outFile, ID3DXMesh* mesh)
{
	outFile << "Vertices:" << std::endl;
	outFile << "---------" << std::endl << std::endl;

	Vertex* v = 0;
	mesh->LockVertexBuffer(0, (void**)&v);
	for (int i = 0; i < mesh->GetNumVertices(); i++)
	{
		outFile << "Vertex " << i << ": (";
		outFile << v[i]._x << ", " << v[i]._y << ", " << v[i]._z << ", ";
		outFile << v[i]._nx << ", " << v[i]._ny << ", " << v[i]._nz << ", ";
		outFile << v[i]._u << ", " << v[i]._v << ")" << std::endl;
	}
	mesh->UnlockVertexBuffer();

	outFile << std::endl << std::endl;
}

void dumpIndices(std::ofstream& outFile, ID3DXMesh* mesh)
{
	outFile << "Indices:" << std::endl;
	outFile << "--------" << std::endl << std::endl;

	WORD* indices = 0;
	mesh->LockIndexBuffer(0, (void**)&indices);

	for (int i = 0; i < mesh->GetNumFaces(); i++)
	{
		outFile << "Triangle " << i << ": ";
		outFile << indices[i * 3] << " ";
		outFile << indices[i * 3 + 1] << " ";
		outFile << indices[i * 3 + 2] << std::endl;
	}
	mesh->UnlockIndexBuffer();

	outFile << std::endl << std::endl;
}

void dumpAttributeBuffer(std::ofstream& outFile, ID3DXMesh* mesh)
{
	outFile << "Attribute Buffer:" << std::endl;
	outFile << "-----------------" << std::endl << std::endl;

	DWORD* attributeBuffer = 0;
	mesh->LockAttributeBuffer(0, &attributeBuffer);

	// an attribute for each face   
	for (int i = 0; i < mesh->GetNumFaces(); i++)
	{
		outFile << "Triangle lives in subset " << i << ": ";
		outFile << attributeBuffer[i] << std::endl;
	}
	mesh->UnlockAttributeBuffer();

	outFile << std::endl << std::endl;
}

void dumpAdjacencyBuffer(std::ofstream& outFile, ID3DXMesh* mesh)
{
	outFile << "Adjacency Buffer:" << std::endl;
	outFile << "-----------------" << std::endl << std::endl;

	// three enttries per face   
	std::vector<DWORD> adjacencyBuffer(mesh->GetNumFaces() * 3);
	//DWORD* adjacencyBuffer(mesh->GetNumFaces() * 3);

	mesh->GenerateAdjacency(0.0f, &adjacencyBuffer[0]);

	for (int i = 0; i < mesh->GetNumFaces(); i++)
	{
		outFile << "Triangle's adjacent to triangle " << i << ": ";
		outFile << adjacencyBuffer[i * 3] << " ";
		outFile << adjacencyBuffer[i * 3 + 1] << " ";
		outFile << adjacencyBuffer[i * 3 + 2] << std::endl;
	}

	outFile << std::endl << std::endl;
}

void dumpAttributeTable(std::ofstream& outFile, ID3DXMesh* mesh)
{
	outFile << "Attribute Table:" << std::endl;
	outFile << "----------------" << std::endl << std::endl;

	// number of entries in the attribute table   
	DWORD numEntries = 0;

	mesh->GetAttributeTable(0, &numEntries);

	std::vector<D3DXATTRIBUTERANGE> table(numEntries);

	mesh->GetAttributeTable(&table[0], &numEntries);

	for (int i = 0; i < numEntries; i++)
	{
		outFile << "Entry " << i << std::endl;
		outFile << "-----------" << std::endl;

		outFile << "Subset ID:    " << table[i].AttribId << std::endl;
		outFile << "Face Start:   " << table[i].FaceStart << std::endl;
		outFile << "Face Count:   " << table[i].FaceCount << std::endl;
		outFile << "Vertex Start: " << table[i].VertexStart << std::endl;
		outFile << "Vertex Count: " << table[i].VertexCount << std::endl;
		outFile << std::endl;
	}

	outFile << std::endl << std::endl;
}