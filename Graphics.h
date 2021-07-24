#pragma once
#include "types.h"
#include <d3d9.h>
#include <d3dx9.h>

typedef HRESULT(__stdcall* fEndScene)(LPDIRECT3DDEVICE9 device);
class Game;

class Graphics {
public:
	Graphics(Game* ptr);
	HWND GetProcessWindow() const;
	Vec2Int GetWindowSize() const;
	static bool initD3DDevice(LPDIRECT3DDEVICE9 o_pDevice);
	bool DrawLine(Vec2Int startPoint, Vec2Int endPoint, D3DCOLOR color, unsigned int thickness=2);
	bool DrawSquare(Vec2Int startPoint, Vec2Int endPoint, D3DCOLOR color, unsigned int thickness=2);
	bool WorldToScreen(Vec3 pos, Vec2Int& screen, float matrix[16]);
	Vec3 TransformVec(Vec3 src, Vec2 ang, float l);
private:
	class D3DHook {
	public:
		D3DHook(Graphics* ptr);
		~D3DHook();
	private:
		bool GetD3D9VMT();
		bool HookEndScene();
		bool UnHookEndScene();
	private:
		byte originalEndSceneBytes[7];
		byte* d3d9VMT[119];
		Graphics* parentObj;
	};
	D3DHook D3DHook;
private:
	static HWND hWnd;
	static ID3DXLine* LineL;
	static LPDIRECT3DDEVICE9 pDevice;
	static RECT WndRect;
	static struct WndSize {
		int Width;
		int Height;
	} WndSize;
private:
	static Game* pHax;
private:
	static BOOL CALLBACK enumWind(HWND handle, LPARAM lp);
protected:
	static fEndScene originalEndScene;
	static HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice);
	static std::mutex renderMutex;
};