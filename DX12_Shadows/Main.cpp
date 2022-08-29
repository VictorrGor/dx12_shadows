#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <fstream>
#include <mutex>

#include "Render.h"
class RenderSys;

RenderSys rs;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


void DrawTriangle() 
{
	Vertex* vtx = new Vertex[3]; memset(vtx, 0, sizeof(Vertex) * 3);
	UINT* indices = new UINT[3];
	//vtx[0].position = { 1, 0.5, 0 };
	//vtx[1].position = { -1, 0.5, 0 };
	//vtx[2].position = { 0, 0, 0 };
	float m_aspectRatio = 1.f;
	vtx[0].position = { 0.0f, 0.25f * m_aspectRatio, 0.50f };
	vtx[1].position = { 0.25f, -0.25f * m_aspectRatio, 0.50f };
	vtx[2].position = { -0.25f, -0.25f * m_aspectRatio, 0.50f };
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	rs.DrawObject(vtx, 3, indices, 3);
}

void CubeScene_2()
{
	Vertex cube[8];
	cube[0].position = { 0.5, 0.5, 0.5 };
	cube[1].position = { 0.5, 0.5, -0.5 };
	cube[2].position = { -0.5, 0.5, -0.5 };
	cube[3].position = { -0.5, 0.5, 0.5 };

	cube[4].position = { 0.5,  -0.5, 0.5 };
	cube[5].position = { 0.5,  -0.5, -0.5 };
	cube[6].position = { -0.5, -0.5, -0.5 };
	cube[7].position = { -0.5, -0.5, 0.5 };
	UINT indices[36] = {
		0, 4, 5, 0, 5, 1, //Front
		0, 1, 2, 0, 2, 3, //Up
		1, 5, 6, 6, 2, 1, // Left
		0, 7, 4, 0, 3, 7, //Right
		3, 2, 6, 3, 6, 7, //Rear
		4, 6, 5, 4, 7, 6, // Bottom
	};
	calcWeightedNormals(cube, 8, indices, 36);
	rs.DrawObject(cube, 8, indices, 36);
	
	Vertex plane[4];
	float plane_size = 5.f;
	plane[0].position = { -plane_size, -0.5, -plane_size };
	plane[1].position = { -plane_size, -0.5, plane_size };
	plane[2].position = { plane_size, -0.5, plane_size};
	plane[3].position = { plane_size, -0.5, -plane_size };
	UINT plane_indices[6] = {0, 1, 2, 2, 3, 0};
	calcWeightedNormals(plane, 4, plane_indices, 6);
	rs.DrawObject(plane, 4, plane_indices, 6);;

}
void CubeScene()
{
	Vertex cube[24];
	//up
	cube[0].position = { 0.5, 0.5, 0.5 };
	cube[0].color = { 1, 0, 0, 1 };
	cube[0].normal = { 0, 1, 0 };
	cube[1].position = { -0.5, 0.5, 0.5 };
	cube[1].color = { 0, 1, 0, 1 };
	cube[1].normal = { 0, 1, 0 };
	cube[2].position = { -0.5, 0.5, -0.5 };
	cube[2].color = { 0, 0, 1, 1 };
	cube[2].normal = { 0, 1, 0 };
	cube[3].position = { 0.5, 0.5, -0.5 };
	cube[3].color = { 1, 0, 0, 1 };
	cube[3].normal = { 0, 1, 0 };
	//front
	cube[4].position = { 0.5, 0.5, 0.5 };
	cube[4].color = { 1, 0, 0, 1 };
	cube[4].normal = { 1, 0, 0 };
	cube[5].position = { 0.5, 0.5, -0.5 };
	cube[5].color = { 0, 1, 0, 1 };
	cube[5].normal = { 1, 0, 0 };
	cube[6].position = { 0.5, -0.5, -0.5 };
	cube[6].color = { 0, 0, 1, 1 };
	cube[6].normal = { 1, 0, 0 };
	cube[7].position = { 0.5, -0.5, 0.5 };
	cube[7].color = { 1, 0, 0, 1 };
	cube[7].normal = { 1, 0, 0 };
	//back
	cube[8].position = { -0.5, 0.5, 0.5 };
	cube[8].color = { 1, 0, 0, 1 };
	cube[8].normal = { -1, 0, 0 };
	cube[9].position = { -0.5, 0.5, -0.5 };
	cube[9].color = { 0, 1, 0, 1 };
	cube[9].normal = { -1, 0, 0 };
	cube[10].position = { -0.5, -0.5, -0.5 };
	cube[10].color = { 0, 0, 1, 1 };
	cube[10].normal = { -1, 0, 0 };
	cube[11].position = { -0.5, -0.5, 0.5 };
	cube[11].color = { 1, 0, 0, 1 };
	cube[11].normal = { -1, 0, 0 };
	//left
	cube[12].position = { -0.5, 0.5, -0.5 };
	cube[12].color = { 1, 0, 0, 1 };
	cube[12].normal = { 0, 0, -1 };
	cube[13].position = { 0.5, 0.5, -0.5 };
	cube[13].color = { 0, 1, 0, 1 };
	cube[13].normal = { 0, 0, -1 };
	cube[14].position = { -0.5, -0.5, -0.5 };
	cube[14].color = { 0, 0, 1, 1 };
	cube[14].normal = { 0, 0, -1 };
	cube[15].position = { 0.5, -0.5, -0.5 };
	cube[15].color = { 1, 0, 0, 1 };
	cube[15].normal = { 0, 0, -1 };
	//right
	cube[16].position = { -0.5, 0.5, 0.5 };
	cube[16].color = { 1, 0, 0, 1 };
	cube[16].normal = { 0, 0, 1 };
	cube[17].position = { 0.5, 0.5, 0.5 };
	cube[17].color = { 0, 1, 0, 1 };
	cube[17].normal = { 0, 0, 1 };
	cube[18].position = { -0.5, -0.5, 0.5 };
	cube[18].color = { 0, 0, 1, 1 };
	cube[18].normal = { 0, 0, 1 };
	cube[19].position = { 0.5, -0.5, 0.5 };
	cube[19].color = { 1, 0, 0, 1 };
	cube[19].normal = { 0, 0, 1 };
	//down
	cube[20].position = { 0.5, -0.5, 0.5 };
	cube[20].color = { 1, 0, 0, 1 };
	cube[20].normal = { 0, -1, 0 };
	cube[21].position = { -0.5, -0.5, 0.5 };
	cube[21].color = { 0, 1, 0, 1 };
	cube[21].normal = { 0, -1, 0 };
	cube[22].position = { -0.5, -0.5, -0.5 };
	cube[22].color = { 0, 0, 1, 1 };
	cube[22].normal = { 0, -1, 0 };
	cube[23].position = { 0.5, -0.5, -0.5 };
	cube[23].color = { 1, 0, 0, 1 };
	cube[23].normal = { 0, -1, 0 };

	UINT indices[] = { 0, 2, 1, 0, 3, 2, 20, 21, 22, 20, 22, 23,
					4, 6, 5, 6, 4, 7, 9, 10, 8, 10, 11, 8,
					12, 13, 15, 14, 12, 15, 17, 16, 19, 19, 16, 18
	};
	calcWeightedNormals(cube, ARRAYSIZE(cube), indices, ARRAYSIZE(indices));
	rs.DrawObject(cube, ARRAYSIZE(cube), indices, ARRAYSIZE(indices));

	Vertex plane[4];
	float plane_size = 50.f;
	plane[0].position = { -plane_size, -0.5, -plane_size };
	plane[1].position = { -plane_size, -0.5, plane_size };
	plane[2].position = { plane_size, -0.5, plane_size };
	plane[3].position = { plane_size, -0.5, -plane_size };
	UINT plane_indices[6] = { 0, 1, 2, 2, 3, 0 };
	calcWeightedNormals(plane, 4, plane_indices, 6);
	rs.DrawObject(plane, 4, plane_indices, 6);;

}

HRESULT InitWindow(HINSTANCE hInst, int nCmdShow, HWND* hWnd)
{
	WNDCLASSEX wc = { 0 };
	const char name[] = "Shadowmaps";

	wc.cbSize = sizeof(wc);
	wc.hInstance = hInst;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = name;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Cann't register class", "Error", MB_OK);
		return E_FAIL;
	}

	(*hWnd) = CreateWindow(name, "Shadowmaps", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInst, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, "Cann't create window", "Error", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG msg = { 0 };

	if (FAILED(InitWindow(hInst, nCmdShow, &hWnd)))
	{
		return 0;
	}

	RECT rc;
	if (!GetClientRect(hWnd, &rc))
	{
		MessageBox(NULL, "Getting rect faild! Something goes wrong.", "Error!", MB_OK);
		return 0;
	}
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	
	rs.Initialize(hWnd);

	CubeScene();


	ShowWindow(hWnd, nCmdShow);

	try
	{
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				rs.Render();
			}
		}
	}
	catch (const char* msg)
	{
		MessageBoxA(hWnd, msg, "Error!", MB_OK);
	}

	rs.Flush();

	return static_cast<int>(msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;

	static int wheel_dist = 1; 
	static std::once_flag onceFlag;
	static int xPosMouse = 0;
	static int yPosMouse = 0;
	std::call_once(onceFlag, []() {POINT p; GetCursorPos(&p); xPosMouse = p.x; yPosMouse = p.y; });

	switch (uMsg)
	{
		case WM_PAINT:
		{
			hDC = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_MOUSEMOVE:
		{
			int actualX = GET_X_LPARAM(lParam);
			int actualY = GET_Y_LPARAM(lParam);

			rs.RotateCamera(actualX - xPosMouse, actualY - yPosMouse);
			xPosMouse = actualX;
			yPosMouse = actualY;
		}
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_ESCAPE:
				{
					DestroyWindow(hWnd);
					break;
				}
				case VK_UP:
				{
					rs.MoveForward();
					break;
				}
				case VK_DOWN:
				{
					rs.MoveBackward();
					break;
				}
				case VK_LEFT:
				{
					rs.MoveLeft();
					break;
				}
				case VK_RIGHT:
				{
					rs.MoveRight();
					break;
				}
			}
		}
		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	return 0;
}
