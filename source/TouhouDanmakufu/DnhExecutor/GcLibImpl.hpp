#ifndef __TOUHOUDANMAKUFU_EXE_GCLIBIMPL__
#define __TOUHOUDANMAKUFU_EXE_GCLIBIMPL__

#include "../Common/DnhGcLibImpl.hpp"
#include "Constant.hpp"

/**********************************************************
//EApplication
**********************************************************/
class EApplication : public Singleton<EApplication>, public Application {
	friend Singleton<EApplication>;

private:
	EApplication();

protected:
	bool _Initialize() override;
	bool _Loop() override;
	bool _Finalize() override;

public:
	~EApplication() override;
};

/**********************************************************
//EDirectGraphics
**********************************************************/
class EDirectGraphics : public Singleton<EDirectGraphics>, public DirectGraphicsPrimaryWindow {
	friend Singleton<EDirectGraphics>;

private:
	EDirectGraphics();

protected:
	LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

public:
	~EDirectGraphics() override;
	bool Initialize() override;
	void SetRenderStateFor2D(int blend);
};

#endif
