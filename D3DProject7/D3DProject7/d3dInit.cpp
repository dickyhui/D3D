#include "d3dUtility.h"

//���л�ͼ�Ĳ���������IDirect3DDevice9 ��
IDirect3DDevice9* Device = 0;

//ָ����Ļ��С
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;

ID3DXMesh* Teapot = 0;
D3DMATERIAL9 TeapotMtrl;

IDirect3DVertexBuffer9* BkGndQuad = 0;
IDirect3DTexture9* BkGndTex = 0;
D3DMATERIAL9 BkGndMtrl;

//�����ƫ��λ��
D3DXVECTOR3 TeapotPosition(5.0f, -1.0f, -1.0f);

//Ϊһ��������������ı�Ҫ�����ǣ�
//1. ����������ָ���ģ���������Ķ��㡣
//2. ��D3DXCreateTextureFromFile������ȡһ������IDirect3DTexture9�ӿ��С�
//3. ������С�������Ŵ����Լ�mipmap��������
//4. �������һ������ǰ����IDirect3DDevice9::SetTexture�������������������

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

//׼��������Ҫ�õ��Ķ�����������Դ�ķ��䣬����豸����������Ӧ�ó����״̬
//����������������棬�������ǣ��ѹ���������Ķ���д�붥�㻺�棬�Լ��Ѷ���������������ε�����д���������档
//Ȼ������������ƶ�������λ�Ա������ܹ���������������ϵ��ԭ�㴦����Ⱦ�������塣
bool Setup()
{
	TeapotMtrl = d3d::RED_MTRL;
	//TeapotMtrl.Diffuse.a = 0.5f;
	BkGndMtrl = d3d::WHITE_MTRL;

	D3DXCreateTeapot(Device, &Teapot, 0);

	//���� CreateVertexBuffer ��������
	Device->CreateVertexBuffer(
		6 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&BkGndQuad,
		0);

	//��������Ķ��㻺���������
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

	//����ӳ�䵽��Ļ�е��������ϡ�ͨ�����������κ���Ļ�������ǲ�һ����ġ�
	//�����������α���Ļ������Сʱ�����������λᱻ�ʵ��Ŵ󡣵����������α���Ļ�����δ�ʱ�����������λᱻ�ʵ���С��
	//��������������ζ�������֡����ˣ� Filtering����һ��Direct3D������������Щ���α��ƽ���ļ�����
	//D3DTEXF_LINEAR�������ֹ��˲�������ȽϺõ�Ч�����ڽ����Ӳ���ϴ��������Ƿǳ���ġ�
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//Ϊ��ʹ�����С�����С��	����Ϊ������mipmaps����
	//Ҳ����˵��һ���������������ı�С���������Ƕ����ǵȼ����ж��ƹ��ˣ���˶�������˵����ϸ���Ǻ���Ҫ��
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	//ѭ��Ѱַ��ʽ
	Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);

	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//����ʹ�õƹ�,Ĭ������
	Device->SetRenderState(D3DRS_LIGHTING, true);
	//����һ����x������ķ����
	D3DLIGHT9 dir;
	::ZeroMemory(&dir, sizeof(dir));
	dir.Type = D3DLIGHT_DIRECTIONAL;
	dir.Diffuse = d3d::WHITE;
	dir.Specular = d3d::WHITE * 0.3f;
	dir.Ambient = d3d::WHITE*0.6f;
	dir.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	Device->SetLight(0, &dir);
	Device->LightEnable(0, true);

	//�����λ�ã���ͼ����
	D3DXVECTOR3 position(-0.8f, 0.5f, -5.0f);//camera����������ϵ�е�λ������
	//D3DXVECTOR3 position(-3.8f, 0.0f, -1.0f);//camera����������ϵ�е�λ������
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);//target��camera�ĳ�������
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//�������ϵķ���һ����[0,1,0]
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	//��ͼ����ϵ�任
	Device->SetTransform(D3DTS_VIEW, &V);

	//ͶӰ����
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

////�ͷ�Setup�з������Դ�������ڴ�
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(BkGndQuad);
}

void RenderMirror()
{
	//��������ģ�建���������Ⱦ״̬
	Device->SetRenderState(D3DRS_STENCILENABLE, true);
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);//������ģ����Զ�ͨ��
	Device->SetRenderState(D3DRS_STENCILREF, 0x1);//���Գɹ�������Ϊ0x1
	Device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);//��Ȳ���ʧ���ˣ�������ģ�建�����
	Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);//ģ�����ʧ���ˣ������£���ʵ����ʧ�ܣ�
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);//��Ȳ��Ժ�ģ����Զ��ɹ�������ģ�建�����

	//������Ȼ��治��д
	Device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	//��ģ�建������Ⱦ����
	Device->SetStreamSource(0, BkGndQuad, 0, sizeof(Vertex));
	Device->SetFVF(Vertex::FVF);
	Device->SetMaterial(&BkGndMtrl);
	Device->SetTexture(0, BkGndTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	Device->SetTransform(D3DTS_WORLD, &I);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	//������Ȼ����д
	Device->SetRenderState(D3DRS_ZWRITEENABLE, true);

	//����ֻ�����ھ����е���Ԫ
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	//�����ڳ����з���λ�õľ���
	D3DXMATRIX W, T, R;
	D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f);//xyƽ��
	D3DXMatrixReflect(&R, &plane);
	D3DXMatrixTranslation(&T,
		TeapotPosition.x,
		TeapotPosition.y,
		TeapotPosition.z);
	W = T*R;
	//������Ĳ������ȱȾ��ӵ���ȴ�,��Ҫ�����Ȼ���
	Device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	//��Ͼ��Ӻͷ���Ĳ������֪��ΪʲôҪ��ô��ϣ�
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	//���Ʊ�����Ĳ��
	D3DXMATRIX WS;
	D3DXMatrixScaling(&WS, 0.2f, 0.2f, 0.2f);
	Device->SetTransform(D3DTS_WORLD, &WS);
	Device->SetTransform(D3DTS_WORLD, &W);
	Device->SetMaterial(&TeapotMtrl);
	Device->SetTexture(0, 0);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);//�ı䱳���ѡģʽ
	Teapot->DrawSubset(0);

	//��ԭ״̬
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	Device->SetRenderState(D3DRS_STENCILENABLE, false);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

//��ͼ����ʾ�Ĵ��룬timeDeltaΪÿһ֡��ʱ��������������ÿ���֡��
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

		//���Ŀ�껺�����Ȼ��棬����Ļ�������ɰ�ɫ
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
			0xffffffff, 1.0f, 0);

		Device->BeginScene();

		
		D3DXMATRIX W,T;
		D3DXMatrixIdentity(&W);//���嵥λ����
		Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetStreamSource(0, BkGndQuad, 0, sizeof(Vertex));
		Device->SetFVF(Vertex::FVF);
		Device->SetMaterial(&BkGndMtrl);
		Device->SetTexture(0, BkGndTex);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		D3DXMatrixScaling(&W, 0.2f, 0.2f, 0.2f); //����
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
