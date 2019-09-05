#ifndef __DIRECTX_EVENTSCRIPT__
#define __DIRECTX_EVENTSCRIPT__

#include "DxScript.hpp"
#include "DxText.hpp"
#include "DxWindow.hpp"
#include "RenderObject.hpp"
#include "TransitionEffect.hpp"

namespace directx {

class EventScriptSource;
class EventScriptCompiler;
class EventScriptCode;
class EventScriptBlock;
class EventScriptBlock_Main;

class EventValueParser;
class EventEngine;
class EventImage;

class DxScriptForEvent;

/**********************************************************
//EventScriptSource
//コンパイルされたイベントスクリプトコード
**********************************************************/
class EventScriptSource {
	friend EventScriptCompiler;

public:
	EventScriptSource();
	virtual ~EventScriptSource();

	int GetCodeCount() const { return code_.size(); }
	void AddCode(gstd::ref_count_ptr<EventScriptCode> code);
	gstd::ref_count_ptr<EventScriptCode> GetCode(int index) { return code_[index]; }
	gstd::ref_count_ptr<EventScriptBlock_Main> GetEventBlock(const std::string& name);

protected:
	std::vector<gstd::ref_count_ptr<EventScriptCode>> code_;
	std::map<std::string, gstd::ref_count_ptr<EventScriptBlock_Main>> event_;
};

/**********************************************************
//EventScriptScanner
**********************************************************/
class EventScriptScanner;
class EventScriptToken {
	friend EventScriptScanner;

public:
	enum Type {
		TK_UNKNOWN,
		TK_EOF,
		TK_NEWLINE,
		TK_ID,
		TK_INT,
		TK_REAL,
		TK_STRING,

		TK_OPENP,
		TK_CLOSEP,
		TK_OPENB,
		TK_CLOSEB,
		TK_OPENC,
		TK_CLOSEC,
		TK_SHARP,

		TK_COMMA,
		TK_EQUAL,
		TK_ASTERISK,
		TK_SLASH,
		TK_COLON,
		TK_SEMICOLON,
		TK_TILDE,
		TK_EXCLAMATION,
		TK_PLUS,
		TK_MINUS,
		TK_LESS,
		TK_GREATER,

		TK_TEXT,
	};

public:
	EventScriptToken() { type_ = TK_UNKNOWN; }
	EventScriptToken(Type type, const std::string& element)
	{
		type_ = type;
		element_ = element;
	}

	virtual ~EventScriptToken() = default;
	Type GetType() const { return type_; }
	std::string GetElement() const { return element_; }

	int GetInteger() const;
	double GetReal() const;
	bool GetBoolean() const;
	std::string GetString() const;
	std::string GetIdentifier() const;

protected:
	Type type_;
	std::string element_;
};

class EventScriptScanner {
public:
	const static int TOKEN_TAG_START;
	const static int TOKEN_TAG_END;
	const static std::string TAG_START;
	const static std::string TAG_END;
	const static std::string TAG_NEW_LINE;
	const static std::string TAG_RUBY;
	const static std::string TAG_FONT;

public:
	EventScriptScanner(const char* str, int size);
	EventScriptScanner(const std::string& str);
	EventScriptScanner(const std::vector<char>& buf);
	virtual ~EventScriptScanner();

	EventScriptToken& GetToken(); //現在のトークンを取得
	EventScriptToken& Next();
	bool HasNext();
	void CheckType(const EventScriptToken& tok, int type);
	void CheckIdentifer(const EventScriptToken& tok, const std::string& id);
	int GetCurrentLine() const;
	int SearchCurrentLine();

	std::vector<char>::iterator GetCurrentPointer() const;
	void SetCurrentPointer(std::vector<char>::iterator pos);
	void SetPointerBegin() { pointer_ = buffer_.begin(); }
	int GetCurrentPosition() const;
	void SetTagScanEnable(bool bEnable) { bTagScan_ = bEnable; }

protected:
	int line_;
	std::vector<char> buffer_;
	std::vector<char>::iterator pointer_; //今の位置
	EventScriptToken token_; //現在のトークン
	bool bTagScan_;

	char _NextChar(); //ポインタを進めて次の文字を調べる
	virtual void _SkipComment(); //コメントをとばす
	virtual void _SkipSpace(); //空白をとばす
	virtual void _RaiseError(const std::wstring& str); //例外を投げます
	bool _IsTextStartSign();
	bool _IsTextScan();
};

/**********************************************************
//EventScriptCompiler
**********************************************************/
class EventScriptCompiler {
public:
	EventScriptCompiler();
	virtual ~EventScriptCompiler();

	void SetPath(const std::wstring& path) { path_ = path; }
	gstd::ref_count_ptr<EventScriptSource> Compile();

private:
	std::wstring path_;
	gstd::ref_count_ptr<EventScriptScanner> scan_;
	gstd::ref_count_ptr<EventScriptSource> source_;
	std::list<std::vector<gstd::ref_count_ptr<EventScriptBlock>>> block_;

	void _ParseBlock(gstd::ref_count_ptr<EventScriptCode> blockStartCode);
	gstd::ref_count_ptr<EventScriptCode> _ParseTag(gstd::ref_count_ptr<EventScriptCode> blockStartCode);
	std::map<std::string, EventScriptToken> _GetAllTagElement();
	std::string _GetNextTagElement();
	std::string _GetNextTagElement(int type);
};

/**********************************************************
//EventScriptBlock
**********************************************************/
class EventScriptBlock {
	friend EventScriptSource;

public:
	enum {
		POS_NULL = -1,
	};
	const static std::string BLOCK_GLOBAL;

public:
	EventScriptBlock()
	{
		bInner_ = false;
		posStart_ = POS_NULL;
		posEnd_ = POS_NULL;
		posReturn_ = POS_NULL;
	}
	virtual ~EventScriptBlock() = default;
	int GetStartPosition() const { return posStart_; }
	void SetStartPosition(int pos) { posStart_ = pos; }
	int GetEndPosition() const { return posEnd_; }
	void SetEndPosition(int pos) { posEnd_ = pos; }
	int GetReturnPosition() const { return posReturn_; }
	void SetReturnPosition(int pos) { posReturn_ = pos; }
	bool IsInner() const { return bInner_; }
	virtual bool IsGlobal() const { return false; }

protected:
	bool bInner_;
	int posStart_;
	int posEnd_;
	int posReturn_;
};

class EventScriptBlock_Main : public EventScriptBlock {
	friend EventScriptSource;

public:
	EventScriptBlock_Main() { bInner_ = false; }
	~EventScriptBlock_Main() override = default;
	std::string GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }
	bool IsGlobal() const override { return name_ == BLOCK_GLOBAL; }

private:
	std::string name_;
};

/**********************************************************
//EventScriptCode
**********************************************************/
class EventScriptCode {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_TEXT,
		TYPE_NEXT_LINE,
		TYPE_WAIT_CLICK,
		TYPE_WAIT_NEXT_PAGE,
		TYPE_WAIT_TIME,
		TYPE_CLEAR_MESSAGE,
		TYPE_NAME,
		TYPE_TRANSITION,
		TYPE_VISIBLE_TEXT,

		TYPE_VAR,
		TYPE_EVAL,
		TYPE_SYSVAL,
		TYPE_OUTPUT,

		TYPE_IMAGE,
		TYPE_SOUND,

		TYPE_IF,
		TYPE_ELSEIF,
		TYPE_ENDIF,
		TYPE_LOOP,
		TYPE_JUMP,

		TYPE_SCRIPT,
		TYPE_END,
		TYPE_BATTLE,
	};
	const static std::string TAG_WAIT_CLICK;
	const static std::string TAG_WAIT_NEXT_PAGE;
	const static std::string TAG_WAIT_TIME;
	const static std::string TAG_CLEAR_MESSAGE;
	const static std::string TAG_NAME;
	const static std::string TAG_TRANSITION;
	const static std::string TAG_VISIBLE_TEXT;
	const static std::string TAG_VAR;
	const static std::string TAG_EVAL;
	const static std::string TAG_SYSVAL;
	const static std::string TAG_OUTPUT;
	const static std::string TAG_IMAGE;
	const static std::string TAG_BGM;
	const static std::string TAG_SE;
	const static std::string TAG_IF;
	const static std::string TAG_ELSEIF;
	const static std::string TAG_ENDIF;
	const static std::string TAG_GOSUB;
	const static std::string TAG_GOTO;
	const static std::string TAG_SCRIPT;
	const static std::string TAG_SCRIPT_END;
	const static std::string TAG_END;
	const static std::string TAG_BATTLE;

	const static std::string STRING_INVALID;

public:
	EventScriptCode();
	virtual ~EventScriptCode();

	int GetType() const { return type_; }
	int GetLine() const { return line_; }
	void SetLine(int line) { line_ = line; }
	virtual std::string GetCodeText() const { return ""; }

protected:
	int type_;
	int line_;
};

class EventScriptCode_Text : public EventScriptCode {
public:
	EventScriptCode_Text();
	std::string GetText() const { return text_; }
	std::string GetCodeText() const override { return GetText(); }
	void SetText(const std::string& text) { text_ = text; }

protected:
	std::string text_;
};
class EventScriptCode_NextLine : public EventScriptCode {
public:
	EventScriptCode_NextLine();
	std::string GetCodeText() const override;
};
class EventScriptCode_WaitClick : public EventScriptCode {
public:
	EventScriptCode_WaitClick();
};
class EventScriptCode_WaitNextPage : public EventScriptCode {
public:
	EventScriptCode_WaitNextPage();
};
class EventScriptCode_WaitTime : public EventScriptCode {
public:
	EventScriptCode_WaitTime();
	std::string GetTime() const { return time_; }
	void SetTime(const std::string& time) { time_ = time; }
	bool IsSkipEnable() const { return bSkipEnable_; }
	void SetSkipEnable(bool bEnable) { bSkipEnable_ = bEnable; }

private:
	std::string time_;
	bool bSkipEnable_;
};
class EventScriptCode_ClearMessage : public EventScriptCode {
public:
	EventScriptCode_ClearMessage();
};
class EventScriptCode_Name : public EventScriptCode {
public:
	EventScriptCode_Name();
	std::string& GetName() { return name_; }
	void SetName(const std::string& name) { name_ = name; }

private:
	std::string name_;
};
class EventScriptCode_Transition : public EventScriptCode {
public:
	enum {
		TRANS_NONE,
		TRANS_FADE,
		TRANS_SCRIPT,
	};

public:
	EventScriptCode_Transition();
	int GetTransType() const { return typeTrans_; }
	void SetTransType(int type) { typeTrans_ = type; }
	std::string GetFrame() const { return frame_; }
	void SetFrame(std::string frame) { frame_ = frame; }
	std::string GetPath() const { return path_; }
	void SetPath(std::string path) { path_ = path; }
	std::string GetMethod() const { return method_; }
	void SetMethod(std::string method) { method_ = method; }

protected:
	int typeTrans_;
	std::string frame_;
	std::string path_;
	std::string method_;
};
class EventScriptCode_VisibleText : public EventScriptCode {
public:
	EventScriptCode_VisibleText();
	bool IsVisible() const { return bVisible_; }
	void SetVisible(bool bVisible) { bVisible_ = bVisible; }

private:
	bool bVisible_;
};
class EventScriptCode_Var : public EventScriptCode {
public:
	EventScriptCode_Var();
	std::string& GetName() { return name_; }
	void SetName(const std::string& name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(const std::string& value) { value_ = value; }

private:
	std::string name_;
	std::string value_;
};
class EventScriptCode_Eval : public EventScriptCode {
public:
	EventScriptCode_Eval();
	std::string& GetName() { return name_; }
	void SetName(const std::string& name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(const std::string& value) { value_ = value; }

private:
	std::string name_;
	std::string value_;
};
class EventScriptCode_SysVal : public EventScriptCode {
public:
	EventScriptCode_SysVal();
	std::string& GetName() { return name_; }
	void SetName(const std::string& name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(const std::string& value) { value_ = value; }
	bool IsGlobal() const { return bGlobal_; }
	void SetGlobal(bool b) { bGlobal_ = b; }

private:
	bool bGlobal_;
	std::string name_;
	std::string value_;
};
class EventScriptCode_Output : public EventScriptCode {
public:
	EventScriptCode_Output();
	std::string& GetValue() { return value_; }
	void SetValue(const std::string& value) { value_ = value; }

private:
	std::string value_;
};
class EventScriptCode_Image : public EventScriptCode {
public:
	enum {
		VALUE_INVALID = INT_MIN,
	};

public:
	EventScriptCode_Image();
	std::string GetObjectIdentifier() const { return idObject_; }
	void SetObjectIdentifier(const std::string& id) { idObject_ = id; }
	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path) { path_ = path; }
	int GetLayer() const { return layer_; }
	void SetLayer(int layer) { layer_ = layer; }
	std::string GetPriority() const { return pri_; }
	void SetPriority(const std::string& pri) { pri_ = pri; }
	std::string GetVisible() const { return bVisible_; }
	void SetVisible(const std::string& bVisible) { bVisible_ = bVisible_; }
	std::string GetLeftDestPoint() const { return posDestLeft_; }
	void SetLeftDestPoint(const std::string& left) { posDestLeft_ = left; }
	std::string GetTopDestPoint() const { return posDestTop_; }
	void SetTopDestPoint(const std::string& top) { posDestTop_ = top; }
	std::string GetTransition() const { return bTransition_; }
	void SetTransition(const std::string& bTrans) { bTransition_ = bTrans; }
	std::string GetWaitEnd() const { return bWaitEnd_; }
	void SetWaitEnd(const std::string& bWaitEnd) { bWaitEnd_ = bWaitEnd; }

protected:
	std::string idObject_;
	std::string path_;
	int layer_;
	std::string pri_;
	std::string bVisible_;
	std::string posDestLeft_;
	std::string posDestTop_;
	std::string bTransition_;
	std::string bWaitEnd_;
};
class EventScriptCode_Sound : public EventScriptCode {
public:
	EventScriptCode_Sound();
	int GetSoundType() const { return typeSound_; }
	void SetSoundType(int type) { typeSound_ = type; }
	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path) { path_ = path; }

private:
	int typeSound_;
	std::string path_;
};
class EventScriptCode_If : public EventScriptCode, public EventScriptBlock {
public:
	EventScriptCode_If();
	std::string GetParameter() const { return param_; }
	void SetParameter(const std::string& param) { param_ = param; }
	int GetNextElsePosition() const { return posNextElse_; }
	void SetNextElsePosition(int pos) { posNextElse_ = pos; }

protected:
	std::string param_;
	int posNextElse_;
};

class EventScriptCode_Jump : public EventScriptCode {
public:
	EventScriptCode_Jump();

	bool IsGoSub() const { return bGoSub_; }
	void SetGosub(bool b) { bGoSub_ = b; }

	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path) { path_ = path; }
	std::string GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

protected:
	bool bGoSub_;
	std::string path_;
	std::string name_;
};

class EventScriptCode_Script : public EventScriptCode {
public:
	EventScriptCode_Script();
	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path) { path_ = path; }
	std::string GetMethod() const { return method_; }
	void SetMethod(const std::string& method) { method_ = method; }
	std::string GetWaitEnd() const { return bWaitEnd_; }
	void SetWaitEnd(const std::string& bWait) { bWaitEnd_ = bWait; }
	std::string GetTargetId() const { return targetId_; }
	void SetTargetId(const std::string& target) { targetId_ = target; }
	std::string GetCode() const { return code_; }
	void SetCode(const std::string& code) { code_ = code; }
	std::string GetId() const { return id_; }
	void SetId(const std::string& id) { id_ = id; }
	std::vector<std::string> GetArgumentList() const { return arg_; }
	void SetArgument(int index, const std::string& arg)
	{
		if (index >= arg_.size())
			arg_.resize(index + 1);
		arg_[index] = arg;
	}

	bool IsEndScript() const { return bEndScript_; }
	void SetEndScript(bool bEnd) { bEndScript_ = bEnd; }

private:
	std::string path_;
	std::string method_;
	std::string bWaitEnd_;
	std::string targetId_;
	std::string code_;
	std::string id_;
	std::vector<std::string> arg_;
	bool bEndScript_;
};

class EventScriptCode_End : public EventScriptCode {
public:
	EventScriptCode_End();
	std::string GetArgument() const { return arg_; }
	void SetArgument(const std::string& arg) { arg_ = arg; }

protected:
	std::string arg_;
};

class EventScriptCode_Battle : public EventScriptCode {
public:
	EventScriptCode_Battle();
	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path) { path_ = path; }

protected:
	std::string path_;
};

/**********************************************************
//EventScriptCodeExecuter
**********************************************************/
class EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter(EventEngine* engine);
	virtual ~EventScriptCodeExecuter();
	virtual void Execute() = 0;
	bool IsEnd() const { return bEnd_; }

protected:
	EventEngine* engine_;
	bool bEnd_;

	int _GetElementInteger(const std::string& value);
	double _GetElementReal(const std::string& value);
	bool _GetElementBoolean(const std::string& value);
	std::string _GetElementString(const std::string& value);
	bool _IsValidElement(const std::string& value);
};

class EventScriptCodeExecuter_WaitClick : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitClick(EventEngine* engine)
		: EventScriptCodeExecuter(engine)
	{
	}
	void Execute() override;
};

class EventScriptCodeExecuter_WaitNextPage : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitNextPage(EventEngine* engine)
		: EventScriptCodeExecuter(engine)
	{
	}
	void Execute() override;
};

class EventScriptCodeExecuter_WaitTime : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitTime(EventEngine* engine, EventScriptCode_WaitTime* code);
	void Execute() override;

private:
	int count_;
	int time_;
	bool bSkipEnable_;
};

class EventScriptCodeExecuter_Transition : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Transition(EventEngine* engine, EventScriptCode_Transition* code);
	void Execute() override;

private:
	class DxScriptRenderObject_Transition : public DxScriptObjectBase {
	public:
		DxScriptRenderObject_Transition(TransitionEffect* effect)
		{
			effect_ = effect;
			priRender_ = 1.0;
		}
		void Render() override { effect_->Render(); }
		void SetRenderState() override {}

	private:
		TransitionEffect* effect_;
	};
	EventScriptCode_Transition* code_;
	gstd::ref_count_ptr<DxScriptForEvent> script_;
	gstd::ref_count_ptr<TransitionEffect> effect_;
	int frame_;
};

class EventScriptCodeExecuter_Image : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Image(EventEngine* engine, EventScriptCode_Image* code);
	~EventScriptCodeExecuter_Image() override;
	void Execute() override;

private:
	EventScriptCode_Image* code_;
	std::string pathImage_;
	gstd::ref_count_ptr<DxScriptObjectManager> objManager_;
	gstd::ref_count_ptr<DxScriptSpriteObject2D>::unsync nowSprite_;
	gstd::ref_count_ptr<DxScriptSpriteObject2D>::unsync oldSprite_;
	int frame_;
	bool bTrans_;
	double priority_;

	void _Initialize();
};

class EventScriptCodeExecuter_Script : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Script(EventEngine* engine, DxScriptForEvent* script);
	void Execute() override;

private:
	DxScriptForEvent* script_;
};

/**********************************************************
//EventWindowManager
**********************************************************/
class EventMouseCaptureLayer;
class EventTextWindow;
class EventNameWindow;
class EventLogWindow;
class EventWindowManager : public DxWindowManager {
public:
	EventWindowManager(EventEngine* engine);
	EventEngine* GetEngine() { return engine_; }
	virtual bool Initialize();
	void Work() override;
	void Render() override;

	gstd::ref_count_ptr<EventMouseCaptureLayer> GetMouseCaptureLayer() { return layerCapture_; }
	gstd::ref_count_ptr<EventTextWindow> GetTextWindow() { return wndText_; }
	gstd::ref_count_ptr<EventLogWindow> GetLogWindow() { return wndLog_; }
	gstd::ref_count_ptr<EventNameWindow> GetNameWindow() { return wndName_; }

	gstd::ref_count_ptr<DxButton> GetSaveButton() { return btnSave_; }
	gstd::ref_count_ptr<DxButton> GetLoadButton() { return btnLoad_; }

	bool IsTextVisible() const { return bVisibleText_; }
	void SetTextVisible(bool bVisible) { bVisibleText_ = bVisible; }

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

protected:
	EventEngine* engine_;
	gstd::ref_count_ptr<EventMouseCaptureLayer> layerCapture_;
	gstd::ref_count_ptr<EventTextWindow> wndText_;
	gstd::ref_count_ptr<EventLogWindow> wndLog_;
	gstd::ref_count_ptr<EventNameWindow> wndName_;

	gstd::ref_count_ptr<DxButton> btnSave_;
	gstd::ref_count_ptr<DxButton> btnLoad_;

	bool bVisibleText_;
};

class EventWindow : public DxWindow {
public:
	EventWindow() { bApplyVisibleText_ = true; }
	virtual bool IsWindowVisible();

protected:
	bool bApplyVisibleText_;
	EventWindowManager* _GetManager() { return (EventWindowManager*)manager_; }
};
class EventButton : public DxButton {
public:
	EventButton() { bApplyVisibleText_ = true; }
	virtual bool IsWindowVisible();

protected:
	bool bApplyVisibleText_;
	EventWindowManager* _GetManager() { return (EventWindowManager*)manager_; }
};
class EventMouseCaptureLayer : public EventWindow {
public:
	void AddedManager() override;
	void DispatchedEvent(gstd::ref_count_ptr<DxWindowEvent> event) override;
	void ClearEvent();
	gstd::ref_count_ptr<DxWindowEvent> GetEvent() { return event_; }

protected:
	gstd::ref_count_ptr<DxWindowEvent> event_;
};
class EventTextWindow : public EventWindow {
public:
	EventTextWindow();
	void AddedManager() override;
	void Render() override;
	bool IsWait();

protected:
	RECT rcMargin_;
	gstd::ref_count_ptr<DxText> dxText_;
};
class EventNameWindow : public EventWindow {
public:
	EventNameWindow();
	void Work() override;
	void Render() override;
	virtual void RenderText();
	std::wstring GetText() const { return text_->GetText(); }
	void SetText(const std::wstring& text) { text_->SetText(text); }

private:
	gstd::ref_count_ptr<DxText> text_;
};
class EventLogWindow : public EventWindow {
public:
	EventLogWindow();
	void AddedManager() override;
	void Work() override;
	void Render() override;

	gstd::ref_count_ptr<DxText> GetRenderer() { return dxText_; }
	void ResetPosition();

private:
	int posMin_; //最低表示位置
	int pos_;
	gstd::ref_count_ptr<DxText> dxText_;
};

/**********************************************************
//EventEngine
**********************************************************/
class EventScriptObjectManager : public DxScriptObjectManager, public gstd::Recordable {
public:
	virtual int AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj);

	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

private:
	enum {
		INDEX_FREE_START = 256,
	};
};
class EventText : public DxTextStepper {
public:
	bool IsVoiceText();
};
class EventLogText {
public:
	EventLogText(EventEngine* engine);
	virtual ~EventLogText();
	void Add(std::string text);

	int GetInfoCount() const { return listInfo_.size(); }
	gstd::ref_count_ptr<DxTextInfo> GetTextInfo(int pos) { return listInfo_[pos]; }

private:
	EventEngine* engine_;
	int max_;
	std::vector<gstd::ref_count_ptr<DxTextInfo>> listInfo_;
};
class EventValue : public gstd::Recordable {
	friend EventValueParser;

public:
	enum {
		TYPE_UNKNOWN,
		TYPE_REAL,
		TYPE_BOOLEAN,
		TYPE_STRING,
	};

public:
	EventValue() { type_ = TYPE_UNKNOWN; }
	~EventValue() override = default;
	int GetType() const { return type_; }
	double GetReal() const
	{
		double res = valueReal_;
		if (IsBoolean())
			res = valueBoolean_ ? 1.0 : 0.0;
		if (IsString())
			res = atof(valueString_.c_str());
		return res;
	}
	bool GetBoolean() const
	{
		bool res = valueBoolean_;
		if (IsReal())
			res = (valueReal_ != 0.0 ? true : false);
		if (IsString())
			res = (valueString_ == "true" ? true : false);
		return res;
	}
	std::string GetString() const
	{
		std::string res = valueString_;
		if (IsReal())
			res = gstd::StringUtility::Format("%f", valueReal_);
		if (IsBoolean())
			res = (valueBoolean_ ? "true" : "false");
		return res;
	}
	bool IsReal() const { return type_ == TYPE_REAL; }
	bool IsBoolean() const { return type_ == TYPE_BOOLEAN; }
	bool IsString() const { return type_ == TYPE_STRING; }

	void SetReal(double v)
	{
		type_ = TYPE_REAL;
		valueReal_ = v;
	}
	void SetBoolean(bool v)
	{
		type_ = TYPE_BOOLEAN;
		valueBoolean_ = v;
	}
	void SetString(const std::string& str)
	{
		type_ = TYPE_STRING;
		valueString_ = str;
	}

	gstd::TextParser::Result ConvertToTextParserResult();
	void Copy(const gstd::TextParser::Result& val);
	void Copy(const EventValue& val);

	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

protected:
	int type_;
	std::string valueString_;
	union {
		double valueReal_;
		bool valueBoolean_;
	};
};
class EventFrame {
public:
	EventFrame();
	virtual ~EventFrame();
	void SetActiveSource(gstd::ref_count_ptr<EventScriptSource> source) { sourceActive_ = source; }
	gstd::ref_count_ptr<EventScriptBlock> GetBlock() { return block_; }
	void SetBlock(gstd::ref_count_ptr<EventScriptBlock> block);
	gstd::ref_count_ptr<EventScriptSource> GetActiveSource() { return sourceActive_; }

	int GetCurrentPosition() const { return posCode_; }
	void SetCurrentPosition(int pos) { posCode_ = pos; }
	gstd::ref_count_ptr<EventScriptCode> NextCode();
	gstd::ref_count_ptr<EventScriptCode> GetCurrentCode();
	bool HasNextCode();
	void SetEnd() { bEnd_ = true; }
	bool IsEnd() const { return bEnd_; }
	gstd::ref_count_ptr<EventValue> GetValue(const std::string& key);
	void AddValue(const std::string& key, gstd::ref_count_ptr<EventValue> val);
	void SetValue(const std::string& key, gstd::ref_count_ptr<EventValue> val);

	int GetReturnPosition() const { return posReturn_; }
	void SetReturnPosition(int pos) { posReturn_ = pos; }
	bool IsInnerBlock();

	bool IsAutoGlobal() const { return bAutoGlobal_; }
	void SetAutoGlobal(bool bAuto) { bAutoGlobal_ = bAuto; }

	//保存
	void ReadRecord(gstd::RecordBuffer& record, EventEngine* engine);
	void WriteRecord(gstd::RecordBuffer& record, EventEngine* engine);

protected:
	bool bEnd_;
	int posCode_;
	int posReturn_;
	bool bAutoGlobal_;
	gstd::ref_count_ptr<EventScriptSource> sourceActive_;
	gstd::ref_count_ptr<EventScriptBlock> block_;
	std::map<std::string, gstd::ref_count_ptr<EventValue>> mapValue_;
};

class EventValueParser : public gstd::TextParser {
public:
	EventValueParser(EventEngine* engine);
	gstd::ref_count_ptr<EventValue> GetEventValue(const std::string& text);
protected:
	EventEngine* engine_;
	virtual Result _ParseIdentifer(std::vector<char>::iterator /*unused*/);
	std::vector<std::string> _GetFuctionArgument();

};

class EventImage : public gstd::Recordable {
public:
	enum {
		LAYER_FOREGROUND,
		LAYER_BACKGROUND,
		MAX_OBJECT = 256 * 4,
		INDEX_OLD_START = 100,
		ID_TRANSITION = 255,
	};

public:
	EventImage();
	~EventImage() override;
	void Render(int layer);
	int GetForegroundLayerIndex() const;
	int GetBackgroundLayerIndex() const;
	void SwapForeBackLayerIndex();
	gstd::ref_count_ptr<DxScriptObjectManager> GetObjectManager(int layer) { return objManager_[layer]; }

	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

protected:
	std::vector<gstd::ref_count_ptr<EventScriptObjectManager>> objManager_;
	int indexForeground_;
};

class EventKeyState {
public:
	EventKeyState(EventEngine* engine);
	virtual ~EventKeyState();
	void Work();

	void SetNextEnable(bool bEnable) { bNextEnable_ = bEnable; }
	bool IsNext() const;
	bool IsSkip() const;

protected:
	EventEngine* engine_;
	bool bNextEnable_;
};

class EventSound : public gstd::Recordable {
public:
	enum {
		TYPE_BGM,
		TYPE_SE,
	};

public:
	EventSound();
	~EventSound() override;

	void Play(int type, const std::string& path);
	void Delete(int type);

	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

private:
	gstd::ref_count_ptr<SoundPlayer> playerBgm_;
	gstd::ref_count_ptr<SoundPlayer> playerSe_;
};

class EventEngine : public gstd::TaskBase, public gstd::Recordable {
public:
	enum {
		STATE_RUN,
		STATE_LOG,
		STATE_HIDE_TEXT,
		STATE_STOP,
	};

public:
	EventEngine();
	~EventEngine() override;
	virtual bool Initialize();

	virtual void Work();
	virtual void Render();
	void SetSource(const std::wstring& path);

	gstd::ref_count_ptr<EventScriptSource> GetSource(const std::wstring& path);
	std::wstring GetSourcePath(gstd::ref_count_ptr<EventScriptSource> source);
	virtual bool IsEnd();

	gstd::ref_count_ptr<EventText> GetEventText() { return textEvent_; }
	gstd::ref_count_ptr<EventLogText> GetEventLogText() { return logEvent_; }
	gstd::ref_count_ptr<EventWindowManager> GetWindowManager() { return windowManager_; }

	gstd::ref_count_ptr<EventValue> GetEventValue(std::string key);
	gstd::ref_count_ptr<EventImage> GetEventImage() { return image_; }
	gstd::ref_count_ptr<EventKeyState> GetEventKeyState() { return keyState_; }

	gstd::ref_count_ptr<EventScriptCodeExecuter> GetActiveCodeExecuter() { return activeCodeExecuter_; }
	gstd::ref_count_ptr<EventFrame> GetGlobalFrame() { return frameGlobal_; }

	gstd::ref_count_ptr<gstd::ScriptEngineCache> GetScriptEngineCache() { return cacheScriptEngine_; }

	bool IsCriticalFrame() const { return bCriticalFrame_; }

	void SetState(int state);
	int GetState() const { return state_; }
	virtual void CheckStateChenge();

	bool IsSaveEnable();
	bool Load(const std::wstring& path);
	bool Load(gstd::RecordBuffer& record);
	bool Save(const std::wstring& path);
	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

protected:
	enum {
		RUN_RETURN_NONE,
		RUN_RETURN_DO,
	};

	int state_;
	gstd::ref_count_ptr<EventWindowManager> windowManager_;
	gstd::ref_count_ptr<EventText> textEvent_;
	gstd::ref_count_ptr<EventLogText> logEvent_;
	std::map<std::wstring, gstd::ref_count_ptr<EventScriptSource>> mapSource_;
	std::vector<gstd::ref_count_ptr<EventFrame>> frame_;
	gstd::ref_count_ptr<EventImage> image_;
	gstd::ref_count_ptr<EventKeyState> keyState_;
	gstd::ref_count_ptr<EventSound> sound_;
	bool bCriticalFrame_;
	gstd::ref_count_ptr<EventFrame> frameGlobal_; //グローバル変数用

	gstd::ref_count_ptr<EventScriptCodeExecuter> activeCodeExecuter_;
	std::list<gstd::ref_count_ptr<EventScriptCodeExecuter>> parallelCodeExecuter_;
	std::list<gstd::ref_count_ptr<DxScriptForEvent>> listScript_;
	gstd::ref_count_ptr<gstd::ScriptEngineCache> cacheScriptEngine_;

	void _RaiseError(std::wstring msg);
	virtual void _RunCode();
	virtual int _RunCode(gstd::ref_count_ptr<EventFrame> frameActive, gstd::ref_count_ptr<EventScriptCode> code) { return RUN_RETURN_NONE; }
	virtual void _RunScript();
	virtual void _WorkWindow();
	gstd::ref_count_ptr<EventScriptSource> _GetSource(const std::wstring& path);
};

/**********************************************************
//DxScriptForEvent
**********************************************************/
class DxScriptForEvent : public DxScript, public gstd::Recordable {
public:
	DxScriptForEvent(EventEngine* engine);
	~DxScriptForEvent() override;
	void Clear();
	bool SetSourceFromFile(const std::wstring& path) override;
	void SetSource(const std::string& source) override;
	virtual int AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj);
	void DeleteObject(int id) override;

	std::string GetMethod() const { return method_; }
	void SetMethod(const std::string& method) { method_ = method; }

	void EndScript() { bScriptEnd_ = true; }
	bool IsScriptEnd() const { return bScriptEnd_; }

	void SetTargetId(int target) { targetId_ = target; }
	std::string GetScriptId() const { return scriptId_; }
	void SetScriptId(const std::string& id) { scriptId_ = id; }
	std::string GetCode() const { return code_; }

	void AddArgumentValue(gstd::ref_count_ptr<EventValue> arg);

	void Read(gstd::RecordBuffer& record) override;
	void Write(gstd::RecordBuffer& record) override;

	//関数：スクリプト操作
	static gstd::value Func_EndScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetTarget(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv);

	//関数：キー入力
	static gstd::value Func_IsSkip(gstd::script_machine* machine, int argc, gstd::value const* argv);

private:
	EventEngine* engine_;
	bool bScriptEnd_;
	std::string method_;
	int targetId_;
	std::string scriptId_;
	std::set<int> listObj_;
	std::string code_;

	std::vector<char> _Include(std::vector<char>& source) override;
};

} // namespace directx

#endif
