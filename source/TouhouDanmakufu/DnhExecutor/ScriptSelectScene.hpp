#ifndef __TOUHOUDANMAKUFU_EXE_SCRIPT_SELECT_SCENE__
#define __TOUHOUDANMAKUFU_EXE_SCRIPT_SELECT_SCENE__

#include "Common.hpp"
#include "GcLibImpl.hpp"

class ScriptSelectSceneMenuItem;
class ScriptSelectModel;
/**********************************************************
//ScriptSelectScene
**********************************************************/
class ScriptSelectScene : public TaskBase, public MenuTask {
public:
	enum {
		TASK_PRI_WORK = 5,
		TASK_PRI_RENDER = 5,
	};
	enum {
		TYPE_SINGLE,
		TYPE_PLURAL,
		TYPE_STAGE,
		TYPE_PACKAGE,
		TYPE_DIR,
		TYPE_ALL,
	};
	enum {
		COUNT_MENU_TEXT = 10,
	};
	class Sort;

private:
	ref_count_ptr<ScriptSelectModel> model_;
	ref_count_ptr<Sprite2D> spriteBack_;
	ref_count_ptr<Sprite2D> spriteImage_;
	std::vector<ref_count_ptr<DxTextRenderObject>> objMenuText_;
	int frameSelect_;

	void _ChangePage() override;

public:
	ScriptSelectScene();
	~ScriptSelectScene() override;
	void Work() override;
	void Render() override;
	void Clear() override;

	int GetType();
	void SetModel(ref_count_ptr<ScriptSelectModel> model);
	void ClearModel();
	void AddMenuItem(std::list<ref_count_ptr<ScriptSelectSceneMenuItem>>& listItem);
};

class ScriptSelectSceneMenuItem : public MenuItem {
	friend ScriptSelectScene;

public:
	enum {
		TYPE_SINGLE,
		TYPE_PLURAL,
		TYPE_STAGE,
		TYPE_PACKAGE,
		TYPE_DIR,
	};

private:
	int type_;
	std::wstring path_;
	ref_count_ptr<ScriptInformation> info_;
	ScriptSelectScene* _GetScriptSelectScene() { return (ScriptSelectScene*)menu_; }

public:
	ScriptSelectSceneMenuItem(int type, const std::wstring& path, ref_count_ptr<ScriptInformation> info);
	~ScriptSelectSceneMenuItem() override;

	int GetType() const { return type_; }
	std::wstring GetPath() const { return path_; }
	ref_count_ptr<ScriptInformation> GetScriptInformation() { return info_; }
};

class ScriptSelectScene::Sort {
public:
	BOOL operator()(const ref_count_ptr<MenuItem>& lf, const ref_count_ptr<MenuItem>& rf) const
	{
		ref_count_ptr<MenuItem> lsp = lf;
		ref_count_ptr<MenuItem> rsp = rf;
		auto* lp = (ScriptSelectSceneMenuItem*)lsp.GetPointer();
		auto* rp = (ScriptSelectSceneMenuItem*)rsp.GetPointer();

		if (lp->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR && rp->GetType() != ScriptSelectSceneMenuItem::TYPE_DIR)
			return TRUE;
		if (lp->GetType() != ScriptSelectSceneMenuItem::TYPE_DIR && rp->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR)
			return FALSE;

		std::wstring lPath = lp->GetPath();
		std::wstring rPath = rp->GetPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};

/**********************************************************
//ScriptSelectModel
**********************************************************/
class ScriptSelectModel {
	friend ScriptSelectScene;

protected:
	volatile bool bCreated_;
	ScriptSelectScene* scene_;

public:
	ScriptSelectModel();
	virtual ~ScriptSelectModel();

	virtual void CreateMenuItem() = 0;
	bool IsCreated() const { return bCreated_; }
};

class ScriptSelectFileModel : public ScriptSelectModel, public Thread {
public:
	enum {
		TYPE_SINGLE = ScriptSelectScene::TYPE_SINGLE,
		TYPE_PLURAL = ScriptSelectScene::TYPE_PLURAL,
		TYPE_STAGE = ScriptSelectScene::TYPE_STAGE,
		TYPE_PACKAGE = ScriptSelectScene::TYPE_PACKAGE,
		TYPE_DIR = ScriptSelectScene::TYPE_DIR,
		TYPE_ALL = ScriptSelectScene::TYPE_ALL,
	};

protected:
	int type_;
	std::wstring dir_;
	std::wstring pathWait_;
	int timeLastUpdate_;

	std::list<ref_count_ptr<ScriptSelectSceneMenuItem>> listItem_;
	void _Run() override;
	virtual void _SearchScript(const std::wstring& dir);
	void _CreateMenuItem(const std::wstring& path);
	bool _IsValidScriptInformation(ref_count_ptr<ScriptInformation> info);
	int _ConvertTypeInfoToItem(int typeInfo);

public:
	ScriptSelectFileModel(int type, const std::wstring& dir);
	~ScriptSelectFileModel() override;
	void CreateMenuItem() override;

	int GetType() const { return type_; }
	std::wstring GetDirectory() const { return dir_; }

	std::wstring GetWaitPath() const { return pathWait_; }
	void SetWaitPath(const std::wstring& path) { pathWait_ = path; }
};

/**********************************************************
//PlayTypeSelectScene
**********************************************************/
class PlayTypeSelectScene : public TaskBase, public MenuTask {
public:
	enum {
		TASK_PRI_WORK = 6,
		TASK_PRI_RENDER = 6,
	};

private:
	ref_count_ptr<ScriptInformation> info_;
	ref_count_ptr<ReplayInformationManager> replayInfoManager_;

public:
	PlayTypeSelectScene(ref_count_ptr<ScriptInformation> info);
	void Work() override;
	void Render() override;
};
class PlayTypeSelectMenuItem : public TextLightUpMenuItem {
	ref_count_ptr<DxTextRenderObject> objText_;
	POINT pos_;

	PlayTypeSelectScene* _GetTitleScene() { return (PlayTypeSelectScene*)menu_; }

public:
	PlayTypeSelectMenuItem(std::wstring text, int x, int y);
	~PlayTypeSelectMenuItem() override;
	void Work() override;
	void Render() override;
};

/**********************************************************
//PlayerSelectScene
**********************************************************/
class PlayerSelectScene : public TaskBase, public MenuTask {
public:
	enum {
		TASK_PRI_WORK = 7,
		TASK_PRI_RENDER = 7,
	};

private:
	ref_count_ptr<Sprite2D> spriteBack_;
	ref_count_ptr<Sprite2D> spriteImage_;
	ref_count_ptr<ScriptInformation> info_;
	std::vector<ref_count_ptr<ScriptInformation>> listPlayer_;
	int frameSelect_;

	void _ChangePage() override { frameSelect_ = 0; };

public:
	PlayerSelectScene(ref_count_ptr<ScriptInformation> info);
	void Work() override;
	void Render() override;
};
class PlayerSelectMenuItem : public TextLightUpMenuItem {
	ref_count_ptr<ScriptInformation> info_;

	PlayerSelectScene* _GetTitleScene() { return (PlayerSelectScene*)menu_; }

public:
	PlayerSelectMenuItem(ref_count_ptr<ScriptInformation> info);
	~PlayerSelectMenuItem() override;
	void Work() override;
	void Render() override;

	ref_count_ptr<ScriptInformation> GetScriptInformation() { return info_; }
};

#endif
