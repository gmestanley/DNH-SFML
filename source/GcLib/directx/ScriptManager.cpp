#include "ScriptManager.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ScriptManager
**********************************************************/
int64_t ScriptManager::idScript_ = 0;
ScriptManager::ScriptManager()
{
	mainThreadID_ = GetCurrentThreadId();

	FileManager::GetBase()->AddLoadThreadListener(this);
}
ScriptManager::~ScriptManager()
{
	FileManager::GetBase()->RemoveLoadThreadListener(this);
	FileManager::GetBase()->WaitForThreadLoadComplete();
}

void ScriptManager::Work()
{
	Work(ManagedScript::TYPE_ALL);
}
void ScriptManager::Work(int targetType)
{
	bHasCloseScriptWork_ = false;
	for (auto itr = listScriptRun_.begin(); itr != listScriptRun_.end();) {
		ref_count_ptr<ManagedScript> script = (*itr);
		int type = script->GetScriptType();
		if (targetType != ManagedScript::TYPE_ALL && targetType != type) {
			++itr;
			continue;
		}

		if (script->IsEndScript()) {
			if (script->IsEventExists("Finalize"))
				script->Run("Finalize");
			itr = listScriptRun_.erase(itr);
			bHasCloseScriptWork_ |= true;
		} else {
			if (script->IsEventExists("MainLoop"))
				script->Run("MainLoop");
			++itr;

			bHasCloseScriptWork_ |= script->IsEndScript();
		}
	}
}
void ScriptManager::Render()
{
	//ここではオブジェクトの描画を行わない。
}
ref_count_ptr<ManagedScript> ScriptManager::GetScript(int64_t id)
{
	{
		Lock lock(lock_);
		auto scriptLoadItr = mapScriptLoad_.find(id);
		if (scriptLoadItr != mapScriptLoad_.end()) {
			return scriptLoadItr->second;
		} else {
			for (auto& script : listScriptRun_) {
				if (script->GetScriptID() == id) {
					return script;
				}
			}
		}
	}
	return nullptr;
}
void ScriptManager::StartScript(int64_t id)
{
	ref_count_ptr<ManagedScript> script = nullptr;
	{
		Lock lock(lock_);
		auto scriptLoadItr = mapScriptLoad_.find(id);
		if (scriptLoadItr == mapScriptLoad_.end())
			return;

		script = scriptLoadItr->second;
	}

	if (!script->IsLoad()) {
		int count = 0;
		while (!script->IsLoad()) {
			if (count % 1000 == 999)
				Logger::WriteTop(StringUtility::Format(L"読み込み完了待機(ScriptManager)：%s", script->GetPath().c_str()));
			Sleep(1);
			++count;
		}
	}

	{
		Lock lock(lock_);
		mapScriptLoad_.erase(id);
		listScriptRun_.push_back(script);
	}

	if (script != nullptr && !IsError()) {
		if (script->IsEventExists("Initialize"))
			script->Run("Initialize");
	}
}
void ScriptManager::CloseScript(int64_t id)
{
	for (auto& script : listScriptRun_) {
		if (script->GetScriptID() == id) {
			script->SetEndScript();
			mapClosedScriptResult_[id] = script->GetResultValue();
			if (mapClosedScriptResult_.size() > MAX_CLOSED_SCRIPT_RESULT) {
				int64_t targetID = mapClosedScriptResult_.begin()->first;
				mapClosedScriptResult_.erase(targetID);
			}

			if (script->IsAutoDeleteObject())
				script->GetObjectManager()->DeleteObjectByScriptID(id);

			break;
		}
	}
}
void ScriptManager::CloseScriptOnType(int type)
{
	for (auto& script : listScriptRun_) {
		if (script->GetScriptType() == type) {
			script->SetEndScript();
			int64_t id = script->GetScriptID();
			mapClosedScriptResult_[id] = script->GetResultValue();
			if (mapClosedScriptResult_.size() > MAX_CLOSED_SCRIPT_RESULT) {
				int64_t targetID = mapClosedScriptResult_.begin()->first;
				mapClosedScriptResult_.erase(targetID);
			}

			if (script->IsAutoDeleteObject())
				script->GetObjectManager()->DeleteObjectByScriptID(id);
		}
	}
}
bool ScriptManager::IsCloseScript(int64_t id)
{
	ref_count_ptr<ManagedScript> script = GetScript(id);
	return (script == nullptr || script->IsEndScript());
}
int ScriptManager::GetAllScriptThreadCount() const
{
	int res = 0;
	Lock lock(lock_);
	for (auto& script : listScriptRun_) {
		res += script->GetThreadCount();
	}
	return res;
}
void ScriptManager::TerminateScriptAll(const std::wstring& message)
{
	Lock lock(lock_);
	for (auto& script : listScriptRun_) {
		script->Terminate(message);
	}
}
int64_t ScriptManager::_LoadScript(const std::wstring& path, ref_count_ptr<ManagedScript> script)
{
	int64_t res = 0;

	res = script->GetScriptID();

	script->SetSourceFromFile(path);
	script->Compile();
	if (script->IsEventExists("Loading"))
		script->Run("Loading");

	{
		Lock lock(lock_);
		script->bLoad_ = true;
		mapScriptLoad_[res] = script;
	}

	return res;
}
int64_t ScriptManager::LoadScript(const std::wstring& path, ref_count_ptr<ManagedScript> script)
{
	int64_t res = 0;
	Lock lock(lock_);
	res = _LoadScript(path, script);
	mapScriptLoad_[res] = script;
	return res;
}
int64_t ScriptManager::LoadScript(const std::wstring& path, int type)
{
	ref_count_ptr<ManagedScript> script = Create(type);
	int64_t res = LoadScript(path, script);
	return res;
}
int64_t ScriptManager::LoadScriptInThread(const std::wstring& path, ref_count_ptr<ManagedScript> script)
{
	Lock lock(lock_);

	int64_t res = script->GetScriptID();
	mapScriptLoad_[res] = script;

	ref_count_ptr<FileManager::LoadThreadEvent> event = new FileManager::LoadThreadEvent(this, path, script);
	FileManager::GetBase()->AddLoadThreadEvent(event);
	return res;
}
int64_t ScriptManager::LoadScriptInThread(const std::wstring& path, int type)
{
	ref_count_ptr<ManagedScript> script = Create(type);
	return LoadScriptInThread(path, script);
}
void ScriptManager::CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event)
{
	std::wstring path = event->GetPath();

	ref_count_ptr<ManagedScript> script = ref_count_ptr<ManagedScript>::DownCast(event->GetSource());
	if (script == nullptr || script->IsLoad())
		return;

	try {
		_LoadScript(path, script);
	} catch (gstd::wexception& e) {
		Logger::WriteTop(e.what());
		script->bLoad_ = true;
		SetError(e.what());
	}
}
void ScriptManager::RequestEventAll(int type, const std::vector<gstd::value>& listValue)
{
	{
		for (auto& script : listScriptRun_) {
			if (script->IsEndScript())
				continue;

			script->RequestEvent(type, listValue);
		}
	}

	if (!listRelativeManager_.empty()) {
		for (auto itrManager = listRelativeManager_.begin(); itrManager != listRelativeManager_.end();) {
			gstd::ref_count_weak_ptr<ScriptManager> manager = (*itrManager);
			if (manager != nullptr) {
				for (auto& script : manager->listScriptRun_) {
					if (script->IsEndScript())
						continue;

					script->RequestEvent(type, listValue);
				}
				++itrManager;
			} else {
				itrManager = listRelativeManager_.erase(itrManager);
			}
		}
	}
}
gstd::value ScriptManager::GetScriptResult(int64_t idScript)
{
	gstd::value res;
	ref_count_ptr<ManagedScript> script = GetScript(idScript);
	if (script != nullptr) {
		res = script->GetResultValue();
	} else {
		auto closedScriptResult = mapClosedScriptResult_.find(idScript) ;
		if (closedScriptResult != mapClosedScriptResult_.end()) {
			res = closedScriptResult->second;
		}
	}
	return res;
}
void ScriptManager::AddRelativeScriptManagerMutual(gstd::ref_count_weak_ptr<ScriptManager> manager1, gstd::ref_count_weak_ptr<ScriptManager> manager2)
{
	manager1->AddRelativeScriptManager(manager2);
	manager2->AddRelativeScriptManager(manager1);
}

/**********************************************************
//ManagedScript
**********************************************************/
const function commonFunction[] = {
	//関数：

	//制御共通関数：スクリプト操作
	{ "LoadScript", ManagedScript::Func_LoadScript, 1 },
	{ "LoadScriptInThread", ManagedScript::Func_LoadScriptInThread, 1 },
	{ "StartScript", ManagedScript::Func_StartScript, 1 },
	{ "CloseScript", ManagedScript::Func_CloseScript, 1 },
	{ "IsCloseScript", ManagedScript::Func_IsCloseScript, 1 },
	{ "GetOwnScriptID", ManagedScript::Func_GetOwnScriptID, 0 },
	{ "GetEventType", ManagedScript::Func_GetEventType, 0 },
	{ "GetEventArgument", ManagedScript::Func_GetEventArgument, 1 },
	{ "SetScriptArgument", ManagedScript::Func_SetScriptArgument, 3 },
	{ "GetScriptResult", ManagedScript::Func_GetScriptResult, 1 },
	{ "SetAutoDeleteObject", ManagedScript::Func_SetAutoDeleteObject, 1 },
	{ "NotifyEvent", ManagedScript::Func_NotifyEvent, 3 },
	{ "NotifyEventAll", ManagedScript::Func_NotifyEventAll, 2 },

};
ManagedScript::ManagedScript()
{
	scriptManager_ = nullptr;
	_AddFunction(commonFunction, sizeof(commonFunction) / sizeof(function));

	bLoad_ = false;
	bEndScript_ = false;
	bAutoDeleteObject_ = false;
}
void ManagedScript::SetScriptManager(ScriptManager* manager)
{
	scriptManager_ = manager;
	mainThreadID_ = scriptManager_->GetMainThreadID();
	idScript_ = scriptManager_->IssueScriptID();
}
gstd::value ManagedScript::RequestEvent(int type, const std::vector<gstd::value>& listValue)
{
	gstd::value res;
	std::string event = "Event";

	if (!IsEventExists(event)) {
		// std::string log = StringUtility::Format("@Eventがありません。(%s)", GetPath().c_str());
		// throw std::exception(log.c_str());
		return res;
	}

	//値を退避(Run中に書き換わる可能性があるため)
	int tEventType = typeEvent_;
	gstd::value tValue = valueRes_;

	typeEvent_ = type;
	listValueEvent_ = listValue;
	valueRes_ = gstd::value();

	Run(event);
	res = GetResultValue();

	//値を戻す
	typeEvent_ = tEventType;
	valueRes_ = tValue;

	return res;
}

//STG制御共通関数：スクリプト操作
gstd::value ManagedScript::Func_LoadScript(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	std::wstring path = argv[0].as_string();
	int type = script->GetScriptType();
	ref_count_ptr<ManagedScript> target = scriptManager->Create(type);
	target->scriptParam_ = script->scriptParam_;

	int64_t res = scriptManager->LoadScript(path, target);
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value ManagedScript::Func_LoadScriptInThread(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	std::wstring path = argv[0].as_string();
	int type = script->GetScriptType();
	ref_count_ptr<ManagedScript> target = scriptManager->Create(type);
	target->scriptParam_ = script->scriptParam_;
	int64_t res = scriptManager->LoadScriptInThread(path, target);
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value ManagedScript::Func_StartScript(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	scriptManager->StartScript(idScript);
	return value();
}
gstd::value ManagedScript::Func_CloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	scriptManager->CloseScript(idScript);
	return value();
}
gstd::value ManagedScript::Func_IsCloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	bool res = scriptManager->IsCloseScript(idScript);
	return value(machine->get_engine()->get_boolean_type(), res);
}

gstd::value ManagedScript::Func_GetOwnScriptID(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	int64_t res = script->GetScriptID();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value ManagedScript::Func_GetEventType(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	int res = script->typeEvent_;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value ManagedScript::Func_GetEventArgument(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	int index = (int)argv[0].as_real();
	if (index < 0 || index >= script->listValueEvent_.size())
		throw gstd::wexception(L"スクリプト引数のインデックスが不正です");
	return script->listValueEvent_[index];
}
gstd::value ManagedScript::Func_SetScriptArgument(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	ref_count_ptr<ManagedScript> target = scriptManager->GetScript(idScript);
	if (target == nullptr)
		return value();

	int index = (int)argv[1].as_real();
	target->SetArgumentValue(argv[2], index);
	return value();
}
gstd::value ManagedScript::Func_GetScriptResult(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	gstd::value res = scriptManager->GetScriptResult(idScript);
	return res;
}
gstd::value ManagedScript::Func_SetAutoDeleteObject(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;

	bool bAutoDelete = argv[0].as_boolean();
	script->SetAutoDeleteObject(bAutoDelete);
	return value();
}
gstd::value ManagedScript::Func_NotifyEvent(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	script->CheckRunInMainThread();
	ScriptManager* scriptManager = script->scriptManager_;

	auto idScript = (int64_t)argv[0].as_real();
	ref_count_ptr<ManagedScript> target = scriptManager->GetScript(idScript);

	if (target == nullptr)
		return value();

	int type = (int)argv[1].as_real();
	std::vector<gstd::value> listArg;
	listArg.push_back(argv[2]);
	gstd::value res = target->RequestEvent(type, listArg);
	return res;
}
gstd::value ManagedScript::Func_NotifyEventAll(script_machine* machine, int argc, value const* argv)
{
	auto* script = (ManagedScript*)machine->data;
	script->CheckRunInMainThread();

	ScriptManager* scriptManager = script->scriptManager_;

	int type = (int)argv[0].as_real();
	std::vector<gstd::value> listArg;
	listArg.push_back(argv[1]);
	scriptManager->RequestEventAll(type, listArg);
	return value();
}
