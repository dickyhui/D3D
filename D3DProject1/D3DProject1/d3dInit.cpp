#include "d3dUtility.h"

IDirect3DDevice9* Device = 0;

bool Setup()
{
	return true;
}

void Cleanup()
{

}

//��ͼ����ʾ�Ĵ��룬timeDeltaΪÿһ֡��ʱ��������������ÿ���֡��
bool Display(float timeDelta)
{
	if (Device)
	{
		//�����̨���棬������Ϊ�ض�����ɫ
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffff0000, 0.5f, 0);
		//������̨�����ǰ̨���棬���ύ����Ⱦ�ܵ�������̨����������Ⱦ������
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
	return 0;
}
