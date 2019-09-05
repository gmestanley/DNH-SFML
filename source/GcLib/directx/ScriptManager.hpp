#ifndef __DIRECTX_SCRIPTMANAGER__
#define __DIRECTX_SCRIPTMANAGER__

#include "DxScript.hpp"

namespace directx {

class ManagedScript;
/**********************************************************
//ScriptManager
**********************************************************/
class ScriptManager : public gstd::FileManager::LoadThreadListener {
public:
	enum {
		MAX_CLOSED_SCRIPT_RESULT = 100,
		ID_INVALID = -1,
	};

public:
	ScriptManager();
	~ScriptManager() override;
	virtual void Work();
	virtual void Work(int targetType);
	virtual void Render();

	virtual void SetError(const std::wstring& error) { error_ = error; }
	virtual bool IsError() const { return !error_.empty(); }

	int GetMainThreadID() const { return mainThreadID_; }
	int64_t IssueScriptID()
	{
		gstd::Lock lock(lock_);
		++idScript_;
		return idScript_;
	}

	gstd::ref_count_ptr<ManagedScript> GetScript(int64_t id);
	void StartScript(int64_t id);
	void CloseScript(int64_t id);
	void CloseScriptOnType(int type);
	bool IsCloseScript(int64_t id);
	bool IsHasCloseScriptWork() const { return bHasCloseScriptWork_; }
	int GetAllScriptThreadCount() const;
	void TerminateScriptAll(const std::wstring& message);

	int64_t LoadScript(const std::wstring& path, gstd::ref_count_ptr<ManagedScript> script);
	int64_t LoadScript(const std::wstring& path, int type);
	int64_t LoadScriptInThread(const std::wstring& path, gstd::ref_count_ptr<ManagedScript> script);
	int64_t LoadScriptInThread(const std::wstring& path, int type);
	void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event) override;

	virtual gstd::ref_count_ptr<ManagedScript> Create(int type) = 0;
	virtual void RequestEventAll(int type, const std::vector<gstd::value>& listValue = std::vector<gstd::value>());
	gstd::value GetScriptResult(int64_t idScript);
	void AddRelativeScriptManager(gstd::ref_count_weak_ptr<ScriptManager> manager) { listRelativeManager_.push_back(manager); }
	static void AddRelativeScriptManagerMutual(gstd::ref_count_weak_ptr<ScriptManager> manager1, gstd::ref_count_weak_ptr<ScriptManager> manager2);

protected:
	int64_t _LoadScript(const std::wstring& path, gstd::ref_count_ptr<ManagedScript> script);

	mutable gstd::CriticalSection lock_;
	static int64_t idScript_;
	bool bHasCloseScriptWork_;

	std::wstring error_;
	std::map<int64_t, gstd::ref_count_ptr<ManagedScript>> mapScriptLoad_;
	std::list<gstd::ref_count_ptr<ManagedScript>> listScriptRun_;
	std::map<int64_t, gstd::value> mapClosedScriptResult_;
	std::list<gstd::ref_count_weak_ptr<ScriptManager>> listRelativeManager_;

	int mainThreadID_;
};

/**********************************************************
//ManagedScript
**********************************************************/
class ManagedScriptParameter {
public:
	ManagedScriptParameter() = default;
	virtual ~ManagedScriptParameter() = default;
};
class ManagedScript : public DxScript, public gstd::FileManager::LoadObject {
	friend ScriptManager;

public:
	enum {
		TYPE_ALL = -1,
	};

public:
	ManagedScript();
	virtual void SetScriptManager(ScriptManager* manager);
	virtual void SetScriptParameter(gstd::ref_count_ptr<ManagedScriptParameter> param) { scriptParam_ = param; }
	gstd::ref_count_ptr<ManagedScriptParameter> GetScriptParameter() { return scriptParam_; }

	int GetScriptType() const { return typeScript_; }
	bool IsLoad() const { return bLoad_; }
	bool IsEndScript() const { return bEndScript_; }
	void SetEndScript() { bEndScript_ = true; }
	bool IsAutoDeleteObject() const { return bAutoDeleteObject_; }
	void SetAutoDeleteObject(bool bEneble) { bAutoDeleteObject_ = bEneble; }

	gstd::value RequestEvent(int type, const std::vector<gstd::value>& listValue = std::vector<gstd::value>());

	//制御共通関数：共通データ
	static gstd::value Func_SaveCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_LoadCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const* argv);

	//制御共通関数：スクリプト操作
	static gstd::value Func_LoadScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_LoadScriptInThread(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_StartScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_CloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_IsCloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetOwnScriptID(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventType(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventArgument(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetScriptArgument(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetScriptResult(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetAutoDeleteObject(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_NotifyEvent(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_NotifyEventAll(gstd::script_machine* machine, int argc, gstd::value const* argv);

protected:
	ScriptManager* scriptManager_;

	int typeScript_;
	gstd::ref_count_ptr<ManagedScriptParameter> scriptParam_;
	volatile bool bLoad_;
	bool bEndScript_;
	bool bAutoDeleteObject_;

	int typeEvent_;
	std::vector<gstd::value> listValueEvent_;
};

} // namespace directx

#endif
