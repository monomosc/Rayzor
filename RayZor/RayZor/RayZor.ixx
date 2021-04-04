#include <memory>
#include <d3d12.h>
#include <wrl/client.h>
#include <directxmath.h>


#include "framework.h"
#include "RayZor.h"


using namespace std;
using Microsoft::WRL::ComPtr;
#define MAX_LOADSTRING 100

import rayzor_main;
import scene_graph;
import cube;
import basic_entity;

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hwnd;
unique_ptr<RayzorMain> Application = nullptr;
int height = 600;
int width = 800;

void populate_demo_scene(scene_graph& s, ComPtr<ID3D12Device> device);
// Forward declarations of functions included in this code module:

void InitializeWindows(int& screenHeight, int& screenWidth);
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	InitializeWindows(height, width);
	Application = make_unique<RayzorMain>(hwnd);


	scene_graph scene;
	populate_demo_scene(scene, Application->get_device());
	MSG msg;
	bool done = false;
	ZeroMemory(&msg, sizeof(MSG));
	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			done = true;
		}
		else {
			done = !Application->Frame(scene);
		}
	}
	MessageBox(hwnd, L"Done=True, end", L"End", MB_OK);
	return 0;
}

void InitializeWindows(int& screenHeight, int& screenWidth)
{
	WNDCLASSEX wc;
	int posX, posY;



	// Get the instance of this application.
	hInst = GetModuleHandle(NULL);

	// Give the application a name.
	LPCWSTR appName = L"Engine";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = appName;
	wc.cbSize = sizeof(WNDCLASSEX);
	// Register the window class.
	RegisterClassEx(&wc);
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, appName, appName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, hInst, NULL);
	if (hwnd == NULL)
		throw std::exception("CreateWindowEx failed");


	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	ShowCursor(false);
	return;

}
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return Application->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}


void populate_demo_scene(scene_graph& s, ComPtr<ID3D12Device> device) {
	auto root = s.get_root();
	auto& children = root->get_children();
	entities::cube::initialize_factory(device);
	auto c = entities::cube::create_cube(device.Get());
	DirectX::XMFLOAT4 transl = { 0.2f, 0.3f, 0.1f, 0.0f };
	auto move_vec = DirectX::XMLoadFloat4(&transl);
	c->move(move_vec);
	children.push_back(c);
}