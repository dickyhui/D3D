#include "d3dUtility.h"

//���л�ͼ�Ĳ���������IDirect3DDevice9 ��
IDirect3DDevice9* Device = 0;

//ָ����Ļ��С
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;
IDirect3DVertexBuffer9* Triangle = 0;


struct ColorVertex
{
	ColorVertex(float x, float y, float z, D3DCOLOR color)
	{
		_x = x;
		_y = y;
		_z = z;
		_color = color;
	}
	float _x, _y, _z;
	D3DCOLOR _color;
	static const DWORD FVF;
};
const DWORD ColorVertex::FVF = D3DFVF_XYZ|D3DFVF_DIFFUSE;

//׼��������Ҫ�õ��Ķ�����������Դ�ķ��䣬����豸����������Ӧ�ó����״̬
//����������������棬�������ǣ��ѹ���������Ķ���д�붥�㻺�棬�Լ��Ѷ���������������ε�����д���������档
//Ȼ������������ƶ�������λ�Ա������ܹ���������������ϵ��ԭ�㴦����Ⱦ�������塣
bool Setup()
{
	//���� CreateVertexBuffer �� CreateIndexBuffer �����������������
	Device->CreateVertexBuffer(
		3 * sizeof(ColorVertex),
		D3DUSAGE_WRITEONLY,
		ColorVertex::FVF,
		D3DPOOL_MANAGED,
		&Triangle,
		0);

	//��������Ķ��㻺���������
	ColorVertex* v;
	Triangle->Lock(0, 0, (void**)&v, 0);

	//���ö�������ݣ�ֱ������Ϊ��������ϵ������
	v[0] = ColorVertex(-1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(255, 0, 0));
	v[1] = ColorVertex(0.0f, 1.0f, 2.0f, D3DCOLOR_XRGB(0, 255, 0));
	v[2] = ColorVertex(1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(0, 0, 255));

	Triangle->Unlock();

	////�����λ�ã���ͼ����
	//D3DXVECTOR3 position(0.0f, 0.0f, -1.0f);//camera����������ϵ�е�λ������
	//D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);//target��camera�ĳ�������
	//D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//�������ϵķ���һ����[0,1,0]
	//D3DXMATRIX V;
	//D3DXMatrixLookAtLH(&V, &position, &target, &up);
	////��ͼ����ϵ�任
	//Device->SetTransform(D3DTS_VIEW, &V);

	//�����ѡ����D3DCULL_CCW��ʱ�뷽������޳�������DX��Ĭ���޳���ʽ��
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//ͶӰ����
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI*0.5f,
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//��Ⱦ״̬
	Device->SetRenderState(D3DRS_LIGHTING, false);

	return true;
		
}

////�ͷ�Setup�з������Դ�������ڴ�
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(Triangle);
}

//��ͼ����ʾ�Ĵ��룬timeDeltaΪÿһ֡��ʱ��������������ÿ���֡��
bool Display(float timeDelta)
{
	if (Device)
	{
		//���Ŀ�껺�����Ȼ��棬����Ļ�������ɰ�ɫ
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0xffffffff, 1.0f, 0);

		Device->BeginScene();

		//����׼��
		//�� vertex buffer �е����ݷŵ�һ�� stream �У� stream�����հѼ���ͼ����Ⱦ��Ϊͼ��
		Device->SetStreamSource(0, Triangle, 0, sizeof(ColorVertex));
		//���õ�ĸ�ʽ�� ����SetFVF����
		Device->SetFVF(ColorVertex::FVF);

		D3DXMatrixTranslation(&World, -1.25f, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &World);

		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

		D3DXMatrixTranslation(&World, 1.25f, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &World);

		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

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
