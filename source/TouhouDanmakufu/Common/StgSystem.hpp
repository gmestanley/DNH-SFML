#ifndef __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__
#define __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__

#include "StgCommon.hpp"
#include "StgEnemy.hpp"
#include "StgIntersection.hpp"
#include "StgItem.hpp"
#include "StgPackageController.hpp"
#include "StgPlayer.hpp"
#include "StgShot.hpp"
#include "StgStageController.hpp"
#include "StgStageScript.hpp"
#include "StgUserExtendScene.hpp"

class StgSystemInformation;
/**********************************************************
//StgSystemController
**********************************************************/
class StgSystemController : public TaskBase {
public:
	enum {
		TASK_PRI_WORK = 4,
		TASK_PRI_RENDER = 4,
	};

public:
	StgSystemController();
	~StgSystemController();
	void Initialize(ref_count_ptr<StgSystemInformation> infoSystem);
	void Start(ref_count_ptr<ScriptInformation> infoPlayer, ref_count_ptr<ReplayInformation> infoReplay);
	void Work();
	void Render();
	void RenderScriptObject();
	void RenderScriptObject(int priMin, int priMax);

	ref_count_ptr<StgSystemInformation>& GetSystemInformation() { return infoSystem_; }
	StgStageController* GetStageController() { return stageController_.GetPointer(); }
	StgPackageController* GetPackageController() { return packageController_.GetPointer(); }
	ref_count_ptr<StgControlScriptInformation>& GetControlScriptInformation() { return infoControlScript_; }

	gstd::ref_count_ptr<ScriptEngineCache> GetScriptEngineCache() { return scriptEngineCache_; }
	gstd::ref_count_ptr<ScriptCommonDataManager> GetCommonDataManager() { return commonDataManager_; }

	void StartStgScene(ref_count_ptr<StgStageInformation> infoStage, ref_count_ptr<ReplayInformation::StageData> replayStageData);
	void StartStgScene(ref_count_ptr<StgStageStartData> startData);

	void TransStgEndScene();
	void TransReplaySaveScene();

	ref_count_ptr<ReplayInformation> CreateReplayInformation();
	void TerminateScriptAll();

protected:
	virtual void DoEnd() = 0;
	virtual void DoRetry() = 0;
	void _ControlScene();

	ref_count_ptr<StgSystemInformation> infoSystem_;
	ref_count_ptr<ScriptEngineCache> scriptEngineCache_;
	gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;

	ref_count_ptr<StgEndScene> endScene_;
	ref_count_ptr<StgReplaySaveScene> replaySaveScene_;

	ref_count_ptr<StgStageController> stageController_;

	ref_count_ptr<StgPackageController> packageController_;
	ref_count_ptr<StgControlScriptInformation> infoControlScript_;
};

/**********************************************************
//StgSystemInformation
**********************************************************/
class StgSystemInformation {
public:
	enum {
		SCENE_NULL,
		SCENE_PACKAGE_CONTROL,
		SCENE_STG,
		SCENE_REPLAY_SAVE,
		SCENE_END,
	};

public:
	StgSystemInformation();
	virtual ~StgSystemInformation();

	bool IsPackageMode();
	void ResetRetry();
	int GetScene() const { return scene_; }
	void SetScene(int scene) { scene_ = scene; }
	bool IsStgEnd() const { return bEndStg_; }
	void SetStgEnd() { bEndStg_ = true; }
	bool IsRetry() const { return bRetry_; }
	void SetRetry() { bRetry_ = true; }
	bool IsError() const { return listError_.size() > 0; }
	void SetError(const std::wstring& error) { listError_.push_back(error); }
	std::wstring GetErrorMessage() const;

	std::wstring GetPauseScriptPath() const { return pathPauseScript_; }
	void SetPauseScriptPath(const std::wstring& path) { pathPauseScript_ = path; }
	std::wstring GetEndSceneScriptPath() const { return pathEndSceneScript_; }
	void SetEndSceneScriptPath(const std::wstring& path) { pathEndSceneScript_ = path; }
	std::wstring GetReplaySaveSceneScriptPath() const { return pathReplaySaveSceneScript_; }
	void SetReplaySaveSceneScriptPath(const std::wstring& path) { pathReplaySaveSceneScript_ = path; }

	ref_count_ptr<ScriptInformation> GetMainScriptInformation() { return infoMain_; }
	void SetMainScriptInformation(ref_count_ptr<ScriptInformation> info) { infoMain_ = info; }

	ref_count_ptr<ReplayInformation> GetActiveReplayInformation() { return infoReplayActive_; }
	void SetActiveReplayInformation(ref_count_ptr<ReplayInformation> info) { infoReplayActive_ = info; }

	void SetInvaridRenderPriority(int priMin, int priMax);
	int GetInvaridRenderPriorityMin() const { return invalidPriMin_; }
	int GetInvaridRenderPriorityMax() const { return invalidPriMax_; }

	void AddReplayTargetKey(int id) { listReplayTargetKey_.insert(id); }
	std::set<int> GetReplayTargetKeyList() const { return listReplayTargetKey_; }

private:
	int scene_;
	bool bEndStg_;
	bool bRetry_;

	std::wstring pathPauseScript_;
	std::wstring pathEndSceneScript_;
	std::wstring pathReplaySaveSceneScript_;

	std::list<std::wstring> listError_;
	ref_count_ptr<ScriptInformation> infoMain_;
	ref_count_ptr<ReplayInformation> infoReplayActive_; //アクティブリプレイ情報

	int invalidPriMin_;
	int invalidPriMax_;
	std::set<int> listReplayTargetKey_;
};

#endif
