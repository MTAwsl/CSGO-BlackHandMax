#include "stdafx.h"
#include <d3d9.h>

Cheat* Graphics::pHax = nullptr;
RECT Graphics::WndRect = { 0,0,0,0 };
struct Graphics::WndSize Graphics::WndSize = {0,0};
std::mutex Graphics::renderMutex;

Graphics::Graphics(Cheat* ptr) : D3DHook(D3DHook::D3DHook(this)) {
	pHax = reinterpret_cast<Cheat*>(ptr);
}

BOOL CALLBACK Graphics::enumWind(HWND handle, LPARAM lp) {
	DWORD procId;
	GetWindowThreadProcessId(handle, &procId);
	if (GetCurrentProcessId() != procId)
		return TRUE;

	hWnd = handle;
	return FALSE;
}

Graphics::D3DHook::D3DHook(Graphics* ptr){
	EnumWindows(Graphics::enumWind, NULL);
	if (hWnd == NULL) throw BHException(__LINE__, __FILE__, "Error when creating D3D9 dummy device: The specific window could not be found.");
	parentObj = ptr;
	if (!GetD3D9VMT()) throw BHException(__LINE__, __FILE__, "GetD3D9VMT() Failed!");
	HookEndScene();
}

Graphics::D3DHook::~D3DHook() {
	parentObj->renderMutex.lock();
	UnHookEndScene();
	parentObj->renderMutex.unlock();
	LineL->Release();
}

bool Graphics::D3DHook::GetD3D9VMT() {
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	IDirect3DDevice9* pDummyDevice = nullptr;
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = false;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = parentObj->GetProcessWindow();
	HRESULT dummyDevCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);
	if (dummyDevCreated != S_OK) {
		d3dpp.Windowed = !d3dpp.Windowed;
		dummyDevCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

		if (dummyDevCreated != S_OK) {
			pD3D->Release();
			return false;
		}
	}
	memcpy(d3d9VMT, *(void***)pDummyDevice, sizeof(d3d9VMT));
	pDummyDevice->Release();
	pD3D->Release();
	return true;
}

void Graphics::D3DHook::HookEndScene() {
	memcpy(originalEndSceneBytes, d3d9VMT[42], sizeof(originalEndSceneBytes));
	parentObj->originalEndScene = (fEndScene)Hook::TrampHook(d3d9VMT[42], (byte*)hkEndScene, sizeof(originalEndSceneBytes));
	if (parentObj->originalEndScene == nullptr)
		throw BHException(__LINE__, __FILE__, GetLastError());
}

void Graphics::D3DHook::UnHookEndScene() {
	Hook::Patch(d3d9VMT[42], originalEndSceneBytes, sizeof(originalEndSceneBytes));
}

bool Graphics::initD3DDevice(LPDIRECT3DDEVICE9 o_pDevice) {
	Graphics::pDevice = o_pDevice;
	D3DXCreateLine(o_pDevice, &Graphics::LineL);

	// Fix hWnd to the Direct3D window
	D3DDEVICE_CREATION_PARAMETERS pp;
	o_pDevice->GetCreationParameters(&pp);
	hWnd = pp.hFocusWindow;
	GetWindowRect(hWnd, &WndRect);
	WndSize.Width = WndRect.right - WndRect.left - 5;
	WndSize.Height = WndRect.bottom - WndRect.top - 29;
	return true;
}

HWND Graphics::GetProcessWindow() const { return hWnd; }
Vec2Int Graphics::GetWindowSize() const { return { WndSize.Width, WndSize.Height }; }
bool Graphics::WorldToScreen(Vec3 pos, Vec2Int& screen, float matrix[16]) {
	float x, y, z, w;
	x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
	y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
	z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
	w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];
	
	if (w < 0.1f)
		return false;
	
	Vec3 NDC;
	NDC.x = x / w;
	NDC.y = y / w;
	NDC.z = z / w;
	
	screen.x = (NDC.x + 1.0f) * 0.5f * WndSize.Width;
	screen.y = (1.0f - NDC.y) * 0.5f * WndSize.Height;
	return true;
}

Vec3 Graphics::TransformVec(Vec3 src, Vec2 ang, float l)
{
	// ang.x = Pitch
	// ang.y = Yaw
	// l is the amplification factor
	Vec3 newPos;
	newPos.x = src.x + (cosf(TORAD(ang.y)) * l);
	newPos.y = src.y + (sinf(TORAD(ang.y)) * l);
	newPos.z = src.z + (tanf(TORAD(ang.x)) * l);
	return newPos;
}

bool Graphics::DrawLine(Vec2Int startPoint, Vec2Int endPoint, D3DCOLOR color, unsigned int thickness) {
	D3DXVECTOR2 Line[2];
	Line[0] = D3DXVECTOR2(startPoint.x, startPoint.y);
	Line[1] = D3DXVECTOR2(endPoint.x, endPoint.y);
	LineL->SetWidth(thickness);
	LineL->Draw(Line, 2, color);
	return true;
}

bool Graphics::DrawSquare(Vec2Int startPoint, Vec2Int endPoint, D3DCOLOR color, unsigned int thickness) {
	DrawLine(startPoint, { startPoint.x, endPoint.y }, color, thickness);
	DrawLine(startPoint, { endPoint.x, startPoint.y }, color, thickness);
	DrawLine(endPoint, { endPoint.x, startPoint.y }, color, thickness);
	DrawLine(endPoint, { startPoint.x, endPoint.y }, color, thickness);
	return true;
}