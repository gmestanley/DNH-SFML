#ifndef __TOUHOUDANMAKUFU_DNHGCLIBIMPL__
#define __TOUHOUDANMAKUFU_DNHGCLIBIMPL__

#include "DnhConstant.hpp"

/**********************************************************
//EPathProperty
**********************************************************/
class EPathProperty : public Singleton<EPathProperty>, public PathProperty {
public:
	static std::wstring GetSystemResourceDirectory();
	static std::wstring GetSystemImageDirectory();
	static std::wstring GetSystemBgmDirectory();
	static std::wstring GetSystemSeDirectory();

	static std::wstring GetStgScriptRootDirectory();
	static std::wstring GetStgDefaultScriptDirectory();
	static std::wstring GetPlayerScriptRootDirectory();

	static std::wstring GetReplaySaveDirectory(const std::wstring& scriptPath);
	static std::wstring GetCommonDataPath(const std::wstring& scriptPath, const std::wstring& area);

	static std::wstring ExtendRelativeToFull(const std::wstring& dir, const std::wstring& path);
};

/**********************************************************
//ELogger
**********************************************************/
class ELogger : public Singleton<ELogger>, public WindowLogger {
public:
	ELogger();
	void Initialize(bool bFile, bool bWindow);

	gstd::ref_count_ptr<gstd::ScriptCommonDataInfoPanel> GetScriptCommonDataInfoPanel() { return panelCommonData_; }
	void UpdateCommonDataInfoPanel(gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager);

private:
	gstd::ref_count_ptr<gstd::ScriptCommonDataInfoPanel> panelCommonData_;
};

/**********************************************************
//EFpsController
**********************************************************/
class EFpsController : public Singleton<EFpsController> {
public:
	EFpsController();

	void SetFps(int fps) { controller_->SetFps(fps); }
	int GetFps() const { return controller_->GetFps(); }
	void SetTimerEnable(bool b) { controller_->SetTimerEnable(b); }

	void Wait() { controller_->Wait(); }
	bool IsSkip() const { return controller_->IsSkip(); }
	void SetCriticalFrame() { controller_->SetCriticalFrame(); }
	float GetCurrentFps() const { return controller_->GetCurrentFps(); }
	float GetCurrentWorkFps() const { return controller_->GetCurrentWorkFps(); }
	float GetCurrentRenderFps() const { return controller_->GetCurrentRenderFps(); }
	bool IsFastMode() const { return controller_->IsFastMode(); }
	void SetFastMode(bool b) { controller_->SetFastMode(b); }

	void AddFpsControlObject(ref_count_weak_ptr<FpsControlObject> obj) { controller_->AddFpsControlObject(obj); }
	void RemoveFpsControlObject(ref_count_weak_ptr<FpsControlObject> obj) { controller_->RemoveFpsControlObject(obj); }

	int GetFastModeKey() const { return fastModeKey_; }
	void SetFastModeKey(int key) { fastModeKey_ = key; }

private:
	int fastModeKey_;
	ref_count_ptr<FpsController> controller_;
};

/**********************************************************
//EFileManager
**********************************************************/
class EFileManager : public Singleton<EFileManager>, public FileManager {
public:
	void ResetArchiveFile();
};

/**********************************************************
//ETaskManager
**********************************************************/
class ETaskManager : public Singleton<ETaskManager>, public WorkRenderTaskManager {
	enum {
		TASK_WORK_PRI_MAX = 10,
		TASK_RENDER_PRI_MAX = 10,
	};

public:
	bool Initialize();
};

/**********************************************************
//ETextureManager
**********************************************************/
class ETextureManager : public Singleton<ETextureManager>, public TextureManager {
	enum {
		MAX_RESERVED_RENDERTARGET = 3,
	};

public:
	bool Initialize() override;
	std::wstring GetReservedRenderTargetName(int index);
};

/**********************************************************
//EMeshManager
**********************************************************/
class EMeshManager : public Singleton<EMeshManager>, public DxMeshManager {
};

/**********************************************************
//EMeshManager
**********************************************************/
class EShaderManager : public Singleton<EShaderManager>, public ShaderManager {
};

/**********************************************************
//EDxTextRenderer
**********************************************************/
class EDxTextRenderer : public Singleton<EDxTextRenderer>, public DxTextRenderer {
};

/**********************************************************
//EDirectSoundManager
**********************************************************/
class EDirectSoundManager : public Singleton<EDirectSoundManager>, public DirectSoundManager {
};

/**********************************************************
//EDirectInput
**********************************************************/
class EDirectInput : public Singleton<EDirectInput>, public VirtualKeyManager {
public:
	enum {
		KEY_INVALID = -1,

		KEY_LEFT,
		KEY_RIGHT,
		KEY_UP,
		KEY_DOWN,

		KEY_OK,
		KEY_CANCEL,

		KEY_SHOT,
		KEY_BOMB,
		KEY_SLOWMOVE,
		KEY_USER1,
		KEY_USER2,

		KEY_PAUSE,

		VK_USER_ID_STAGE = 256,
		VK_USER_ID_PLAYER = 512,

	};

	int padIndex_;

public:
	bool Initialize(HWND hWnd) override;
	void ResetVirtualKeyMap();
	int GetPadIndex() const { return padIndex_; }
};

#endif
