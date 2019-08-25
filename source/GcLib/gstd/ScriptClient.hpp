#ifndef __GSTD_SCRIPTCLIENT__
#define __GSTD_SCRIPTCLIENT__

#include "File.hpp"
#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Logger.hpp"
#include "MersenneTwister.hpp"
#include "Script.hpp"
#include "Thread.hpp"

namespace gstd {

class ScriptFileLineMap;
class ScriptCommonDataManager;
/**********************************************************
//ScriptException
**********************************************************/
class ScriptException : public gstd::wexception {
public:
	ScriptException()
		: gstd::wexception(L""){};
	ScriptException(std::wstring str)
		: gstd::wexception(str.c_str()){};
};

/**********************************************************
//ScriptEngineData
**********************************************************/
class ScriptEngineData {
public:
	ScriptEngineData();
	virtual ~ScriptEngineData();

	void SetPath(const std::wstring& path) { path_ = path; }
	std::wstring GetPath() const { return path_; }
	void SetSource(const std::vector<char>& source);
	std::vector<char>& GetSource() { return source_; }
	int GetEncoding() const { return encoding_; }
	void SetEngine(gstd::ref_count_ptr<script_engine> engine) { engine_ = engine; }
	gstd::ref_count_ptr<script_engine> GetEngine() { return engine_; }
	gstd::ref_count_ptr<ScriptFileLineMap> GetScriptFileLineMap() { return mapLine_; }
	void SetScriptFileLineMap(gstd::ref_count_ptr<ScriptFileLineMap> mapLine) { mapLine_ = mapLine; }

protected:
	std::wstring path_;
	int encoding_;
	std::vector<char> source_;
	gstd::ref_count_ptr<script_engine> engine_;
	gstd::ref_count_ptr<ScriptFileLineMap> mapLine_;
};

/**********************************************************
//ScriptEngineCache
**********************************************************/
class ScriptEngineCache {
public:
	ScriptEngineCache();
	virtual ~ScriptEngineCache();
	void Clear();

	void AddCache(const std::wstring& name, ref_count_ptr<ScriptEngineData> data);
	ref_count_ptr<ScriptEngineData> GetCache(const std::wstring& name);
	bool IsExists(const std::wstring& name) const;

protected:
	std::map<std::wstring, ref_count_ptr<ScriptEngineData>> cache_;
};

/**********************************************************
//ScriptBase
**********************************************************/
class ScriptClientBase {
private:
	static script_type_manager typeManagerDefault_;

public:
	enum {
		ID_SCRIPT_FREE = -1,
	};
	static script_type_manager* GetDefaultScriptTypeManager() { return &typeManagerDefault_; }

public:
	ScriptClientBase();
	virtual ~ScriptClientBase();
	void SetScriptEngineCache(gstd::ref_count_ptr<ScriptEngineCache> cache) { cache_ = cache; }
	gstd::ref_count_ptr<ScriptEngineData> GetEngine() { return engine_; }
	virtual bool SetSourceFromFile(const std::wstring& path);
	virtual void SetSource(const std::vector<char>& source);
	virtual void SetSource(const std::string& source);

	std::wstring GetPath() { return engine_->GetPath(); }
	void SetPath(const std::wstring& path) { engine_->SetPath(path); }

	virtual void Compile();
	virtual bool Run();
	virtual bool Run(const std::string& target);
	bool IsEventExists(const std::string& name) const;
	void RaiseError(const std::wstring& error) { _RaiseError(machine_->get_error_line(), error); }
	void Terminate(const std::wstring& error) { machine_->terminate(error); }
	_int64 GetScriptID() const { return idScript_; }
	int GetThreadCount() const;

	void AddArgumentValue(value v) { listValueArg_.push_back(v); }
	void SetArgumentValue(value v, int index = 0);
	value GetResultValue() const { return valueRes_; }

	value CreateRealValue(long double r);
	value CreateBooleanValue(bool b);
	value CreateStringValue(const std::string& s);
	value CreateStringValue(const std::wstring& s);
	value CreateRealArrayValue(const std::vector<long double>& list);
	value CreateStringArrayValue(const std::vector<std::string>& list);
	value CreateStringArrayValue(const std::vector<std::wstring>& list);
	value CreateValueArrayValue(const std::vector<value>& list);
	bool IsRealValue(const value& v);
	bool IsBooleanValue(const value& v);
	bool IsStringValue(const value& v);
	bool IsRealArrayValue(const value& v);

	void CheckRunInMainThread();
	ScriptCommonDataManager* GetCommonDataManager() { return commonDataManager_.GetPointer(); }

	//共通関数：スクリプト引数結果
	static value Func_GetScriptArgument(script_machine* machine, int argc, const value* argv);
	static value Func_GetScriptArgumentCount(script_machine* machine, int argc, const value* argv);
	static value Func_SetScriptResult(script_machine* machine, int argc, const value* argv);

	//共通関数：数学系
	static value Func_Min(script_machine* machine, int argc, const value* argv);
	static value Func_Max(script_machine* machine, int argc, const value* argv);
	static value Func_Log(script_machine* machine, int argc, const value* argv);
	static value Func_Log10(script_machine* machine, int argc, const value* argv);
	static value Func_Cos(script_machine* machine, int argc, const value* argv);
	static value Func_Sin(script_machine* machine, int argc, const value* argv);
	static value Func_Tan(script_machine* machine, int argc, const value* argv);
	static value Func_Acos(script_machine* machine, int argc, const value* argv);
	static value Func_Asin(script_machine* machine, int argc, const value* argv);
	static value Func_Atan(script_machine* machine, int argc, const value* argv);
	static value Func_Atan2(script_machine* machine, int argc, const value* argv);
	static value Func_Rand(script_machine* machine, int argc, const value* argv);

	//共通関数：文字列操作
	static value Func_ToString(script_machine* machine, int argc, const value* argv);
	static value Func_IntToString(script_machine* machine, int argc, const value* argv);
	static value Func_ItoA(script_machine* machine, int argc, const value* argv);
	static value Func_RtoA(script_machine* machine, int argc, const value* argv);
	static value Func_RtoS(script_machine* machine, int argc, const value* argv);
	static value Func_VtoS(script_machine* machine, int argc, const value* argv);
	static value Func_AtoI(script_machine* machine, int argc, const value* argv);
	static value Func_AtoR(script_machine* machine, int argc, const value* argv);
	static value Func_TrimString(script_machine* machine, int argc, const value* argv);
	static value Func_SplitString(script_machine* machine, int argc, const value* argv);

	//共通関数：パス関連
	static value Func_GetModuleDirectory(script_machine* machine, int argc, const value* argv);
	static value Func_GetMainScriptDirectory(script_machine* machine, int argc, const value* argv);
	static value Func_GetCurrentScriptDirectory(script_machine* machine, int argc, const value* argv);
	static value Func_GetFileDirectory(script_machine* machine, int argc, const value* argv);
	static value Func_GetFilePathList(script_machine* machine, int argc, const value* argv);
	static value Func_GetDirectoryList(script_machine* machine, int argc, const value* argv);

	//共通関数：時刻関連
	static value Func_GetCurrentDateTimeS(script_machine* machine, int argc, const value* argv);

	//共通関数：デバッグ関連
	static value Func_WriteLog(script_machine* machine, int argc, const value* argv);
	static value Func_RaiseError(script_machine* machine, int argc, const value* argv);

	//共通関数：共通データ
	static value Func_SetDefaultCommonDataArea(script_machine* machine, int argc, const value* argv);
	static value Func_SetCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_GetCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_ClearCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_DeleteCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_SetAreaCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_GetAreaCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_ClearAreaCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_DeleteAreaCommonData(script_machine* machine, int argc, const value* argv);
	static value Func_CreateCommonDataArea(script_machine* machine, int argc, const value* argv);
	static value Func_CopyCommonDataArea(script_machine* machine, int argc, const value* argv);
	static value Func_IsCommonDataAreaExists(script_machine* machine, int argc, const value* argv);
	static value Func_GetCommonDataAreaKeyList(script_machine* machine, int argc, const value* argv);
	static value Func_GetCommonDataValueKeyList(script_machine* machine, int argc, const value* argv);

protected:
	bool bError_;
	gstd::ref_count_ptr<ScriptEngineCache> cache_;
	gstd::ref_count_ptr<ScriptEngineData> engine_;
	script_machine* machine_;

	std::vector<function> func_;
	ref_count_ptr<MersenneTwister> mt_;
	gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;
	int mainThreadID_;
	_int64 idScript_;

	gstd::CriticalSection criticalSection_;

	std::vector<gstd::value> listValueArg_;
	gstd::value valueRes_;

	void _AddFunction(const char* name, callback f, unsigned arguments);
	void _AddFunction(const function* f, int count);
	void _RaiseErrorFromEngine();
	void _RaiseErrorFromMachine();
	void _RaiseError(int line, const std::wstring& message);
	std::wstring _GetErrorLineSource(int line);
	virtual std::vector<char> _Include(std::vector<char>& source);
	virtual bool _CreateEngine();
	std::wstring _ExtendPath(const std::wstring& path);
};

/**********************************************************
//ScriptFileLineMap
**********************************************************/
class ScriptFileLineMap {
public:
	struct Entry {
		int lineStart_;
		int lineEnd_;
		int lineStartOriginal_;
		int lineEndOriginal_;
		std::wstring path_;
	};

public:
	ScriptFileLineMap();
	virtual ~ScriptFileLineMap();
	void AddEntry(std::wstring path, int lineAdd, int lineCount);
	Entry GetEntry(int line) const;
	std::wstring GetPath(int line) const;
	std::list<Entry> GetEntryList() const { return listEntry_; }

protected:
	std::list<Entry> listEntry_;
};

/**********************************************************
//ScriptCommonDataManager
**********************************************************/
class ScriptCommonData;
class ScriptCommonDataManager {
public:
	ScriptCommonDataManager();
	virtual ~ScriptCommonDataManager();
	void Clear();

	std::string GetDefaultAreaName() const { return nameAreaDefailt_; }
	void SetDefaultAreaName(const std::string& name) { nameAreaDefailt_ = name; }
	bool IsExists(const std::string& name);
	void CreateArea(const std::string& name);
	void CopyArea(const std::string& nameDest, const std::string& nameSrc);
	gstd::ref_count_ptr<ScriptCommonData> GetData(const std::string& name);
	void SetData(const std::string& name, gstd::ref_count_ptr<ScriptCommonData> commonData);
	std::vector<std::string> GetKeyList();

	gstd::CriticalSection& GetLock() { return lock_; }

protected:
	gstd::CriticalSection lock_;
	std::string nameAreaDefailt_;
	std::map<std::string, gstd::ref_count_ptr<ScriptCommonData>> mapData_;
};

/**********************************************************
//ScriptCommonData
**********************************************************/
class ScriptCommonData {
public:
	ScriptCommonData();
	virtual ~ScriptCommonData();
	void Clear();
	bool IsExists(const std::string& name);
	gstd::value GetValue(const std::string& name);
	void SetValue(const std::string& name, const gstd::value& val);
	void DeleteValue(const std::string& name);
	void Copy(gstd::ref_count_ptr<ScriptCommonData> dataSrc);
	std::vector<std::string> GetKeyList();

	void ReadRecord(gstd::RecordBuffer& record);
	void WriteRecord(gstd::RecordBuffer& record);

protected:
	std::map<std::string, gstd::value> mapValue_;

	gstd::value _ReadRecord(gstd::ByteBuffer& buffer);
	void _WriteRecord(gstd::ByteBuffer& buffer, gstd::value& comValue);
};

/**********************************************************
//ScriptCommonDataInfoPanel
**********************************************************/
class ScriptCommonDataInfoPanel : public WindowLogger::Panel {
public:
	ScriptCommonDataInfoPanel();
	void SetUpdateInterval(int time) { timeUpdateInterval_ = time; }
	virtual void LocateParts();
	virtual void Update(gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager);

protected:
	enum {
		COL_AREA = 0,
		COL_KEY = 0,
		COL_VALUE,
	};

	gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;
	gstd::CriticalSection lock_;

	WSplitter wndSplitter_;
	WListView wndListViewArea_;
	WListView wndListViewValue_;
	int timeLastUpdate_;
	int timeUpdateInterval_;

	virtual bool _AddedLogger(HWND hTab);
	void _UpdateListViewKey(WListView* listView, std::vector<std::string> listKey);
	void _UpdateAreaView();
	void _UpdateValueView();
};

} // namespace gstd

#endif
