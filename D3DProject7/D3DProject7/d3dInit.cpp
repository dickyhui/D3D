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

ID3DXFont* font = 0;


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

	TeapotMtrl = d3d::RED_MTRL;
	//TeapotMtrl.Diffuse.a = 0.5f;
	BkGndMtrl = d3d::WHITE_MTRL;

	D3DXCreateTeapot(Device, &Teapot, 0);

	//���� CreateVertexBuffer ��������
	Device->CreateVertexBuffer(
		12 * sizeof(Vertex),
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

	v[6] = Vertex(-5.0f, -2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[7] = Vertex(-5.0f, -2.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = Vertex(5.0f, -2.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[9] = Vertex(-5.0f, -2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[10] = Vertex(5.0f, -2.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[11] = Vertex(5.0f, -2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

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
	D3DXVECTOR3 position(-2.0f, 0.5f, -5.0f);//camera����������ϵ�е�λ������
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
	//(ref & mask) ComparisonOperation (value & mask)
	//����	���ԵĽ����true����ô���ǰ�����д��󻺴档������ԵĽ����false, ���Ǿ���ֹ���ر�д��󻺴档��Ȼ��������ز��ܱ�д��󻺴棬��ô��Ҳ���ܱ�д����Ȼ��档
	Device->SetRenderState(D3DRS_STENCILENABLE, true);
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);//���ñȽ����㣬����������ģ����Զ�ͨ��
	Device->SetRenderState(D3DRS_STENCILREF, 0x1);//ģ��ο�ֵ
	Device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);//ģ������
	Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);//ģ��д����
	Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);//����ģ�建�棬��Ȳ���ʧ���ˣ�������ģ�建�����
	Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);//����ģ�建�棬ģ�����ʧ���ˣ������£���ʵ����ʧ�ܣ�
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);//����ģ�建�棬��Ȳ��Ժ�ģ����Զ��ɹ����òο�ֵ�滻

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
	/*D3DXMATRIX WS;
	D3DXMatrixScaling(&WS, 0.2f, 0.2f, 0.2f);
	Device->SetTransform(D3DTS_WORLD, &WS);*/
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

void RenderShadow()
{
	//��������ģ�建���������Ⱦ״̬
	Device->SetRenderState(D3DRS_STENCILENABLE, true);
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);//���ñȽ����㣬����������ģ����Զ�ͨ��
	Device->SetRenderState(D3DRS_STENCILREF, 0x0);//ģ��ο�ֵ
	Device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);//ģ������
	Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);//ģ��д����
	Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);//����ģ�建�棬��Ȳ���ʧ���ˣ�������ģ�建�����
	Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);//����ģ�建�棬ģ�����ʧ���ˣ������£���ʵ����ʧ�ܣ�
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);//����ģ�建�棬��Ȳ��Ժ�ģ����Զ��ɹ����òο�ֵ�滻

	/*D3DXMATRIX WS;
	D3DXMatrixScaling(&WS, 0.2f, 0.2f, 0.2f);*/
	//Device->SetTransform(D3DTS_WORLD, &WS);
	//D3DXVECTOR4 lightDirection(0.707f, -0.707f, 0.707f, 0.0f);
	D3DXVECTOR4 lightDirection(0.0f, -1.0f, 0.0f, 0.0f); //����ƽ�й�
	D3DXPLANE groundPlane(0.0f, -1.0f, 0.0f, -2.0f); //���õ���
	D3DXMATRIX S;
	D3DXMatrixShadow(&S, &lightDirection, &groundPlane); //�ó���Ӱת������
	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, TeapotPosition.x, TeapotPosition.y, TeapotPosition.z); 
	D3DXMATRIX W = T * S;
	Device->SetTransform(D3DTS_WORLD, &W);

	//������Ĳ������ȱȾ��ӵ���ȴ�,��Ҫ�����Ȼ���
	Device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	//���Ʋ������Ӱ
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	D3DMATERIAL9 mtrl = d3d::InitMtrl(d3d::BLACK, d3d::BLACK, d3d::BLACK, d3d::BLACK, 0.0f);
	mtrl.Diffuse.a = 0.5f; // 50% transparency.
	// Disable depth buffer so that z-fighting doesn't occur when we
	// render the shadow on top of the floor.
	Device->SetRenderState(D3DRS_ZENABLE, false);
	Device->SetMaterial(&mtrl);
	Device->SetTexture(0, 0);
	Teapot->DrawSubset(0);
	Device->SetRenderState(D3DRS_ZENABLE, true);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	Device->SetRenderState(D3DRS_STENCILENABLE, false);

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

//��ͼ����ʾ�Ĵ��룬timeDeltaΪÿһ֡��ʱ��������������ÿ���֡��
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

		std::ostringstream fpsBuff;
		fpsBuff << FPS;
		std::string s(fpsBuff.str());
		s = "FPS: " + s;
		font->DrawText(0, s.c_str(), -1, &textRect, DT_TOP | DT_LEFT, 0xffff0000);//����fps�ı�
		
		D3DXMATRIX W,T;
		D3DXMatrixIdentity(&W);//���嵥λ����
		Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetStreamSource(0, BkGndQuad, 0, sizeof(Vertex));
		Device->SetFVF(Vertex::FVF);
		Device->SetMaterial(&BkGndMtrl);
		Device->SetTexture(0, BkGndTex);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

		Device->SetFVF(Vertex::FVF);
		Device->SetMaterial(&BkGndMtrl);
		Device->SetTexture(0, BkGndTex);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 2);

		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		//D3DXMatrixScaling(&W, 0.2f, 0.2f, 0.2f); //����
		D3DXMatrixTranslation(&T,
			TeapotPosition.x,
			TeapotPosition.y,
			TeapotPosition.z);
		//Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetTransform(D3DTS_WORLD, &T);
		Device->SetMaterial(&TeapotMtrl);
		Device->SetTexture(0, 0);
		Teapot->DrawSubset(0);
		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		RenderMirror();
		RenderShadow();
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
