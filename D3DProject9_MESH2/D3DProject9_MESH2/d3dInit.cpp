#include "d3dUtility.h"

//���л�ͼ�Ĳ���������IDirect3DDevice9 ��
IDirect3DDevice9* Device = 0;

//ָ����Ļ��С
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;

ID3DXFont* font = 0;

ID3DXMesh* Mesh = 0;
std::vector<D3DMATERIAL9> Mtrls(0);
std::vector<IDirect3DTexture9*> Textrue(0);

std::ofstream OutFile; //�ļ�д���� �ڴ�д��洢�豸   

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

//���������εķ���
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

//׼��������Ҫ�õ��Ķ�����������Դ�ķ��䣬����豸����������Ӧ�ó����״̬
//����������������棬�������ǣ��ѹ���������Ķ���д�붥�㻺�棬�Լ��Ѷ���������������ε�����д���������档
//Ȼ������������ƶ�������λ�Ա������ܹ���������������ϵ��ԭ�㴦����Ⱦ�������塣
bool Setup()
{
	CreateFont();

	OutFile.open("Mesh Dump.txt");

	dumpVertices(OutFile, Mesh);
	dumpIndices(OutFile, Mesh);
	dumpAttributeTable(OutFile, Mesh);
	dumpAttributeBuffer(OutFile, Mesh);
	dumpAdjacencyBuffer(OutFile, Mesh);

	OutFile.close();

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

////�ͷ�Setup�з������Դ�������ڴ�
void Cleanup()
{

}

DWORD FrameCnt;
float TimeElapsed;
float FPS;
RECT textRect;

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

	textRect.left = 0;
	textRect.right = 200;
	textRect.top = 0;
	textRect.bottom = 50;
}

void DrawFPS()
{
	std::ostringstream fpsBuff;
	fpsBuff << FPS;
	std::string s(fpsBuff.str());
	s = "FPS: " + s;
	font->DrawText(0, s.c_str(), -1, &textRect, DT_TOP | DT_LEFT, 0xffff0000);//����fps�ı�
}

//��ͼ����ʾ�Ĵ��룬timeDeltaΪÿһ֡��ʱ��������������ÿ���֡��
bool Display(float timeDelta)
{
	if (Device)
	{
		CalcFPS(timeDelta);

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

		DrawFPS();

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}



//Windowsͨ�����ô�����Ϣ�������Դ��ڷ�����Ϣ
//�ڵ�һ�ν�������ʱ���ڴ��ڹر�ʱ�����ڸı��С���ƶ����߱��ͼ��ʱ���Ӳ˵���ѡ��ĳһ��Ŀ��Ų����������������갴ť���ߴӼ��������ַ�ʱ���Լ����ڿͻ������뱻����ʱ��Windows��Ҫ����WndProc
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM IParam)
{
	switch (msg)
	{
	//WM_CLOSE ��Ϣ������ʱ���û����Ը����Լ�����Ը��ѡ�񵽵��Ƿ�رգ�WM_DESTORY ����Ĺر�һ������,WM_QUIT���˳�һ��Ӧ�ó���
	case WM_DESTROY:
		//�ú�����ϵͳ�����и��߳�����ֹ����ͨ��������ӦWM_DESTROY��Ϣ��
		//PostQuitMessage����һ��WM_QUIT��Ϣ���̵߳���Ϣ���в��������أ��˺�����ϵͳ�����и��߳�����������ĳһʱ����ֹ��
		::PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			//����ָ���Ĵ��ڡ��������ͨ������WM_DESTROY ��Ϣ�� WM_NCDESTROY ��Ϣʹ������Ч���Ƴ�����̽��㡣
			//������������ٴ��ڵĲ˵�������̵߳���Ϣ���У������봰�ڹ�����صĶ�ʱ����������ڶԼ������ӵ��Ȩ����ϼ��������Ĳ鿴����
			::DestroyWindow(hwnd);
		break;
	}
	//DefWindowProc��������ȱʡ�Ĵ��ڹ�����ΪӦ�ó���û�д�����κδ�����Ϣ�ṩȱʡ�Ĵ����ú���ȷ��ÿһ����Ϣ�õ�����
	return ::DefWindowProc(hwnd, msg, wParam, IParam);
}

//Ӧ�ó��������
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	AllocConsole();
	//��ʼ��һ��Ӧ�ó���������ڲ�����Direct3D�ĳ�ʼ��
	if (!d3d::InitD3D(hinstance, 800, 600, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "INitD3D() - FAILED", 0, 0);
		return 0;
	}

	//׼��������Ҫ�õ��Ķ�����������Դ�ķ��䣬����豸����������Ӧ�ó����״̬
	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	//�������Ϣѭ��������Ҫ����һ����ʾ����ָ�룬��������Ϊ��ʹ��ʾ�����ܹ��ڿ��е�ʱ�򱻵��ò���ʾ����
	d3d::EnterMsgLoop(Display);

	//�ͷ�Setup�з������Դ�������ڴ�
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