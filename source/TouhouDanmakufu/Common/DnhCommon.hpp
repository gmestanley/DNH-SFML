#ifndef __TOUHOUDANMAKUFU_DNHCOMMON__
#define __TOUHOUDANMAKUFU_DNHCOMMON__

#include "DnhConstant.hpp"

/**********************************************************
//ScriptInformation
**********************************************************/
class ScriptInformation {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_PLAYER,
		TYPE_SINGLE,
		TYPE_PLURAL,
		TYPE_STAGE,
		TYPE_PACKAGE,

		MAX_ID = 8,
	};
	const static std::wstring DEFAULT;
	class Sort;
	struct PlayerListSort;

public:
	ScriptInformation() {}
	virtual ~ScriptInformation() {}
	int GetType() const { return type_; }
	void SetType(int type) { type_ = type; }
	std::wstring GetArchivePath() const { return pathArchive_; }
	void SetArchivePath(const std::wstring& path) { pathArchive_ = path; }
	std::wstring GetScriptPath() const { return pathScript_; }
	void SetScriptPath(const std::wstring& path) { pathScript_ = path; }

	std::wstring GetID() const { return id_; }
	void SetID(const std::wstring& id) { id_ = id; }
	std::wstring GetTitle() const { return title_; }
	void SetTitle(const std::wstring& title) { title_ = title; }
	std::wstring GetText() const { return text_; }
	void SetText(const std::wstring& text) { text_ = text; }
	std::wstring GetImagePath() const { return pathImage_; }
	void SetImagePath(const std::wstring& path) { pathImage_ = path; }
	std::wstring GetSystemPath() const { return pathSystem_; }
	void SetSystemPath(const std::wstring& path) { pathSystem_ = path; }
	std::wstring GetBackgroundPath() const { return pathBackground_; }
	void SetBackgroundPath(const std::wstring& path) { pathBackground_ = path; }
	std::wstring GetBgmPath() const { return pathBGM_; }
	void SetBgmPath(const std::wstring& path) { pathBGM_ = path; }
	std::vector<std::wstring> GetPlayerList() const { return listPlayer_; }
	void SetPlayerList(const std::vector<std::wstring>& list) { listPlayer_ = list; }

	std::wstring GetReplayName() const { return replayName_; }
	void SetReplayName(const std::wstring& name) { replayName_ = name; }

	std::vector<ref_count_ptr<ScriptInformation>> CreatePlayerScriptInformationList();

public:
	static ref_count_ptr<ScriptInformation> CreateScriptInformation(const std::wstring& pathScript, bool bNeedHeader = true);
	static ref_count_ptr<ScriptInformation> CreateScriptInformation(const std::wstring& pathScript, const std::wstring& pathArchive, const std::string& source, bool bNeedHeader = true);

	static std::vector<ref_count_ptr<ScriptInformation>> CreateScriptInformationList(const std::wstring& path, bool bNeedHeader = true);
	static std::vector<ref_count_ptr<ScriptInformation>> FindPlayerScriptInformationList(const std::wstring& dir);
	static bool IsExcludeExtension(const std::wstring& ext);

private:
	static std::wstring _GetString(Scanner& scanner);
	static std::vector<std::wstring> _GetStringList(Scanner& scanner);

private:
	int type_;
	std::wstring pathArchive_;
	std::wstring pathScript_;

	std::wstring id_;
	std::wstring title_;
	std::wstring text_;
	std::wstring pathImage_;
	std::wstring pathSystem_;
	std::wstring pathBackground_;
	std::wstring pathBGM_;
	std::vector<std::wstring> listPlayer_;

	std::wstring replayName_;
};

class ScriptInformation::Sort {
public:
	BOOL operator()(const ref_count_ptr<ScriptInformation>& lf, const ref_count_ptr<ScriptInformation>& rf) const
	{
		ref_count_ptr<ScriptInformation> lsp = lf;
		ref_count_ptr<ScriptInformation> rsp = rf;
		ScriptInformation* lp = (ScriptInformation*)lsp.GetPointer();
		ScriptInformation* rp = (ScriptInformation*)rsp.GetPointer();
		std::wstring lPath = lp->GetScriptPath();
		std::wstring rPath = rp->GetScriptPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};

struct ScriptInformation::PlayerListSort {
	BOOL operator()(const ref_count_ptr<ScriptInformation>& lf, const ref_count_ptr<ScriptInformation>& rf) const
	{
		ref_count_ptr<ScriptInformation> lsp = lf;
		ref_count_ptr<ScriptInformation> rsp = rf;
		std::wstring lPath = lsp->GetScriptPath();
		std::wstring rPath = rsp->GetScriptPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};

/**********************************************************
//ErrorDialog
**********************************************************/
class ErrorDialog : public ModalDialog {
public:
	ErrorDialog(HWND hParent);
	bool ShowModal(const std::wstring& msg);

	static void SetParentWindowHandle(HWND hWndParent) { hWndParentStatic_ = hWndParent; }
	static void ShowErrorDialog(const std::wstring& msg)
	{
		ErrorDialog dialog(hWndParentStatic_);
		dialog.ShowModal(msg);
	}

protected:
	static HWND hWndParentStatic_;

	WEditBox edit_;
	WButton button_;

	virtual LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/**********************************************************
//DnhConfiguration
**********************************************************/
class DnhConfiguration : public Singleton<DnhConfiguration> {
	static const int VERSION;

public:
	enum {
		WINDOW_SIZE_640x480 = 0,
		WINDOW_SIZE_800x600,
		WINDOW_SIZE_960x720,
		WINDOW_SIZE_1280x960,
		WINDOW_SIZE_1600x1200,
		WINDOW_SIZE_1920x1200,

		FPS_NORMAL = 0,
		FPS_1_2, // 1/2
		FPS_1_3, // 1/3
		FPS_AUTO,
	};

public:
	DnhConfiguration();
	virtual ~DnhConfiguration();
	bool LoadConfigFile();
	bool SaveConfigFile();

	int GetScreenMode() const { return modeScreen_; }
	void SetScreenMode(int mode) { modeScreen_ = mode; }
	int GetWindowSize() const { return sizeWindow_; }
	void SetWindowSize(int size) { sizeWindow_ = size; }
	int GetFpsType() const { return fpsType_; }
	void SetFpsType(int type) { fpsType_ = type; }

	int GetPadIndex() { return padIndex_; }
	void SetPadIndex(int index) { padIndex_ = index; }
	ref_count_ptr<VirtualKey> GetVirtualKey(int id);

	bool IsLogWindow() const { return bLogWindow_; }
	void SetLogWindow(bool b) { bLogWindow_ = b; }
	bool IsLogFile() const { return bLogFile_; }
	void SetLogFile(bool b) { bLogFile_ = b; }
	bool IsMouseVisible() const { return bMouseVisible_; }
	void SetMouseVisible(bool b) { bMouseVisible_ = b; }

	std::wstring GetPackageScriptPath() const { return pathPackageScript_; }
	std::wstring GetWindowTitle() const { return windowTitle_; }
	int GetScreenWidth() const { return screenWidth_; }
	int GetScreenHeight() const { return screenHeight_; }

private:
	int modeScreen_; //DirectGraphics::SCREENMODE_FULLSCREEN,SCREENMODE_WINDOW
	int sizeWindow_;
	int fpsType_;

	int padIndex_;
	std::map<int, ref_count_ptr<VirtualKey>> mapKey_;

	bool bLogWindow_;
	bool bLogFile_;
	bool bMouseVisible_;

	std::wstring pathPackageScript_;
	std::wstring windowTitle_;
	int screenWidth_;
	int screenHeight_;

	bool _LoadDefintionFile();
};

#endif
