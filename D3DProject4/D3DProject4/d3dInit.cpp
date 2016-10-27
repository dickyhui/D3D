#include "d3dUtility.h"

//���л�ͼ�Ĳ���������IDirect3DDevice9 ��
IDirect3DDevice9* Device = 0;

//ָ����Ļ��С
const int Width = 800;
const int Height = 600;

D3DXMATRIX World;
//IDirect3DVertexBuffer9* Triangle = 0;
IDirect3DVertexBuffer9* Pyramid = 0;

//�������������
ID3DXMesh* mesh = 0;

/*���������ӵƹ�Ĳ����ǣ�
1�� ����ʹ�õƹ⡣
2�� Ϊÿ�����崴�����ʲ�������Ⱦ��Ӧ����ǰӦ�����ʸ������塣
3�� ����һ��������Դ���������ǣ���������Ϊ���á�
4�� ���������ӹ�Դ��Ϊ���ã����羵��߹⡣*/



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
	//�����������
	D3DXCreateTeapot(Device, &mesh, 0);

	//����ʹ�õƹ�,Ĭ������
	Device->SetRenderState(D3DRS_LIGHTING, true);
	//���� CreateVertexBuffer ��������
	Device->CreateVertexBuffer(
		12 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&Pyramid,
		0);

	//��������Ķ��㻺���������
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

	//������ʣ�������׹⣬�������⣬�һ����һЩ�߹�
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = d3d::WHITE;//������
	mtrl.Diffuse = d3d::WHITE;//��ɫ��
	mtrl.Specular = d3d::WHITE;//�����
	mtrl.Emissive = d3d::RED;//�Է���
	mtrl.Power = 5.0f;//�߹�
	Device->SetMaterial(&mtrl);

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

	//����״̬ʹ���ߴ��¹���ҰѾ���߹�����Ϊ����
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);


	//�����λ�ã���ͼ����
	D3DXVECTOR3 position(2.0f, 2.0f, -2.0f);//camera����������ϵ�е�λ������
	D3DXVECTOR3 target(-2.0f, -2.0f, 2.0f);//target��camera�ĳ�������
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//�������ϵķ���һ����[0,1,0]
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	//��ͼ����ϵ�任
	Device->SetTransform(D3DTS_VIEW, &V);

	//�����ѡ����D3DCULL_CCW��ʱ�뷽������޳�������DX��Ĭ���޳���ʽ��
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//ͶӰ����
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

////�ͷ�Setup�з������Դ�������ڴ�
void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(Pyramid);
	mesh->Release();
	mesh = 0;
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
		//���ò����ƫ��
		D3DXMatrixTranslation(&World, 1.0f, 0.0f, -0.5f);
		Device->SetTransform(D3DTS_WORLD, &World);
		//���Ʋ��
		mesh->DrawSubset(0);
		//���ý�������ƫ��
		D3DXMatrixTranslation(&World, -1.5f, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &World);
		//����׼��
		//�� vertex buffer �е����ݷŵ�һ�� stream �У� stream�����հѼ���ͼ����Ⱦ��Ϊͼ��
		Device->SetStreamSource(0, Pyramid, 0, sizeof(Vertex));
		//���õ�ĸ�ʽ�� ����SetFVF����
		Device->SetFVF(Vertex::FVF);

		//���ƽ�����
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 4);

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
