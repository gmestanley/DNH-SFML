#ifndef __TOUHOUDANMAKUFU_DNHSTG_USER_EXTEND_SCENE__
#define __TOUHOUDANMAKUFU_DNHSTG_USER_EXTEND_SCENE__

#include "StgCommon.hpp"
#include "StgControlScript.hpp"

/**********************************************************
//StgUserExtendScene
**********************************************************/
class StgUserExtendSceneScriptManager;
class StgUserExtendScene {
public:
	StgUserExtendScene(StgSystemController* controller);
	virtual ~StgUserExtendScene();
	ref_count_ptr<StgUserExtendSceneScriptManager> GetScriptManager() { return scriptManager_; }

	virtual void Work();
	virtual void Render();

	virtual void Start();
	virtual void Finish();

protected:
	void _InitializeTransitionTexture();
	void _InitializeScript(const std::wstring& path, int type);
	void _CallScriptMainLoop();
	void _CallScriptFinalize();
	void _AddRelativeManager();

	StgSystemController* systemController_;
	ref_count_ptr<StgUserExtendSceneScriptManager> scriptManager_;
};

/**********************************************************
//StgUserExtendSceneScriptManager
**********************************************************/
class StgUserExtendSceneScript;
class StgUserExtendSceneScriptManager : public StgControlScriptManager {
public:
	StgUserExtendSceneScriptManager(StgSystemController* controller);
	~StgUserExtendSceneScriptManager() override;
	void Work() override;
	void Render() override;
	ref_count_ptr<ManagedScript> Create(int type) override;

	void CallScriptFinalizeAll();
	gstd::value GetResultValue();
	bool IsRealValue(const gstd::value& val);

protected:
	StgSystemController* systemController_;
	ref_count_ptr<DxScriptObjectManager> objectManager_;
};

/**********************************************************
//StgUserExtendSceneScript
**********************************************************/
class StgUserExtendSceneScript : public StgControlScript {
public:
	enum {
		TYPE_PAUSE_SCENE,
		TYPE_END_SCENE,
		TYPE_REPLAY_SCENE,
	};

public:
	StgUserExtendSceneScript(StgSystemController* controller);
	~StgUserExtendSceneScript() override;
};

/**********************************************************
//StgPauseScene
**********************************************************/
class StgPauseSceneScript;
class StgPauseScene : public StgUserExtendScene {
public:
	StgPauseScene(StgSystemController* controller);
	~StgPauseScene() override;

	void Work() override;

	void Start() override;
	void Finish() override;
};

class StgPauseSceneScript : public StgUserExtendSceneScript {
public:
	enum {

	};

public:
	StgPauseSceneScript(StgSystemController* controller);
	~StgPauseSceneScript() override;
};

/**********************************************************
//StgEndScene
**********************************************************/
class StgEndScript;
class StgEndScene : public StgUserExtendScene {
public:
	StgEndScene(StgSystemController* controller);
	~StgEndScene() override;

	void Work() override;

	void Start() override;
	void Finish() override;
};

/**********************************************************
//StgEndSceneScript
**********************************************************/
class StgEndSceneScript : public StgUserExtendSceneScript {
public:
	enum {

	};

public:
	StgEndSceneScript(StgSystemController* controller);
	~StgEndSceneScript() override;
};

/**********************************************************
//StgReplaySaveScene
**********************************************************/
class StgReplaySaveScript;
class StgReplaySaveScene : public StgUserExtendScene {
public:
	StgReplaySaveScene(StgSystemController* controller);
	~StgReplaySaveScene() override;

	void Work() override;

	void Start() override;
	void Finish() override;
};

/**********************************************************
//StgReplaySaveScript
**********************************************************/
class StgReplaySaveScript : public StgUserExtendSceneScript {
public:
	enum {

	};

public:
	StgReplaySaveScript(StgSystemController* controller);
	~StgReplaySaveScript() override;
};

#endif
