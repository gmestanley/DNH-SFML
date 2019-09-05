#include "EventScript.hpp"
#include "DirectGraphics.hpp"
#include "DirectInput.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//EventScriptSource
//コンパイルされたイベントスクリプトコード
**********************************************************/
EventScriptSource::EventScriptSource() = default;
EventScriptSource::~EventScriptSource()
{
	code_.clear();
	event_.clear();
}
void EventScriptSource::AddCode(gstd::ref_count_ptr<EventScriptCode> code)
{
	code_.push_back(code);
}
gstd::ref_count_ptr<EventScriptBlock_Main> EventScriptSource::GetEventBlock(const std::string& name)
{
	auto eventItr = event_.find(name);
	if (eventItr != event_.end())
		return eventItr->second;
	return nullptr;
}

/**********************************************************
//EventScriptScanner
**********************************************************/
constexpr int EventScriptScanner::TOKEN_TAG_START = EventScriptToken::TK_OPENB;
constexpr int EventScriptScanner::TOKEN_TAG_END = EventScriptToken::TK_CLOSEB;
const std::string EventScriptScanner::TAG_START = "[";
const std::string EventScriptScanner::TAG_END = "]";
const std::string EventScriptScanner::TAG_NEW_LINE = "r";
const std::string EventScriptScanner::TAG_RUBY = "ruby";
const std::string EventScriptScanner::TAG_FONT = "font";
constexpr char CHAR_TAG_START = '[';
constexpr char CHAR_TAG_END = ']';
EventScriptScanner::EventScriptScanner(const char* str, int size)
{
	std::vector<char> buf;
	buf.resize(size);
	if (!buf.empty()) {
		memcpy(&buf[0], str, size);
	}

	buf.push_back('\0');
	this->EventScriptScanner::EventScriptScanner(buf);
}
EventScriptScanner::EventScriptScanner(const std::string& str)
{
	std::vector<char> buf;
	buf.resize(str.size() + 1);
	memcpy(&buf[0], str.c_str(), str.size() + 1);
	this->EventScriptScanner::EventScriptScanner(buf);
}
EventScriptScanner::EventScriptScanner(const std::vector<char>& buf)
{
	buffer_ = buf;
	pointer_ = buffer_.begin();

	line_ = 1;
	bTagScan_ = false;
}
EventScriptScanner::~EventScriptScanner() = default;

char EventScriptScanner::_NextChar()
{
	if (!HasNext()) {
		_RaiseError(L"_NextChar:すでに文字列終端です");
	}

	if (IsDBCSLeadByte(*pointer_) != 0)
		pointer_ += 2;
	else
		++pointer_;

	if (IsDBCSLeadByte(*pointer_) == 0 && *pointer_ == '\n')
		++line_;
	return *pointer_;
}
void EventScriptScanner::_SkipComment()
{
	while (true) {
		auto posStart = pointer_;
		_SkipSpace();

		char ch = *pointer_;

		if (ch == '/') { //コメントアウト処理
			auto tPos = pointer_;
			ch = _NextChar();
			if (ch == '/') { // "//"
				while (true) {
					ch = _NextChar();
					if (IsDBCSLeadByte(ch))
						continue;
					if (ch == '\r' || ch == '\n')
						break;
				}
			} else if (ch == '*') { // "/*"-"*/"
				while (true) {
					ch = _NextChar();
					if (ch == '*') {
						ch = _NextChar();
						if (ch == '/')
							break;
					}
				}
				ch = _NextChar();
			} else {
				pointer_ = tPos;
				ch = '/';
			}
		}

		//スキップも空白飛ばしも無い場合、終了
		if (posStart == pointer_)
			break;
	}
}
void EventScriptScanner::_SkipSpace()
{
	while (*pointer_ == L' ' || *pointer_ == L'\t') {
		_NextChar();
	}
}
void EventScriptScanner::_RaiseError(const std::wstring& str)
{
	throw gstd::wexception(str);
}
bool EventScriptScanner::_IsTextStartSign()
{
	if (bTagScan_ != false)
		return false;

	bool res = false;
	char ch = *pointer_;

	if (false && ch == '\\') {
		auto pos = pointer_;
		ch = _NextChar(); //次のタグまで進める
		bool bDBSSLeadByte = IsDBCSLeadByte(ch) != 0;
		bool bLess = (!bDBSSLeadByte && ch == CHAR_TAG_START);
		if (!bLess) {
			res = false;
			SetCurrentPointer(pos);
		} else {
			res = bDBSSLeadByte || !bLess;
		}
	} else {
		bool bDBSSLeadByte = IsDBCSLeadByte(ch) != 0;
		bool bLess = (!bDBSSLeadByte && ch == CHAR_TAG_START);
		res = bDBSSLeadByte || !bLess;
	}

	return res;
}
bool EventScriptScanner::_IsTextScan()
{
	bool res = false;
	char ch = _NextChar();
	if (!HasNext()) {
		return false;
	}
	if (ch == '/') {
		ch = *(pointer_ + 1);
		if (ch == '/' || ch == '*')
			res = false;
	} else if (false && ch == '\\') {
		ch = _NextChar(); //次のタグまで進める
		res = true;
	} else {
		bool bGreater = (IsDBCSLeadByte(ch) == 0 && ch == CHAR_TAG_END);
		if (bGreater) {
			_RaiseError(L"テキスト中にタグ終了文字が存在しました");
		}
		bool bNotLess = !(IsDBCSLeadByte(ch) == 0 && ch == CHAR_TAG_START);
		res = bNotLess;
	}
	return res;
}
EventScriptToken& EventScriptScanner::GetToken()
{
	return token_;
}
EventScriptToken& EventScriptScanner::Next()
{
	if (!HasNext()) {
		_RaiseError(L"Next:すでに終端です");
	}

	_SkipComment(); //コメントをとばします

	char ch = *pointer_;
	if (ch == '\0') {
		token_ = EventScriptToken(EventScriptToken::TK_EOF, "\0");
		return token_;
	}

	EventScriptToken::Type type = EventScriptToken::TK_UNKNOWN;
	auto posStart = pointer_; //先頭を保存

	if (_IsTextStartSign()) {
		ch = *pointer_;

		posStart = pointer_;
		while (_IsTextScan()) {
		}

		ch = *pointer_;
		if (IsDBCSLeadByte(*pointer_) == 0 && ch == CHAR_TAG_START) {
		} else if (!HasNext()) {
		}
		// else _RaiseError("Next:すでに文字列終端です");

		type = EventScriptToken::TK_TEXT;
		std::string text = std::string(posStart, pointer_);
		text = StringUtility::ReplaceAll(text, "\\", "");
		token_ = EventScriptToken(type, text);
	} else {
		switch (ch) {
		case '\0':
			type = EventScriptToken::TK_EOF;
			break; //終端
		case ',':
			_NextChar();
			type = EventScriptToken::TK_COMMA;
			break;
		case '=':
			_NextChar();
			type = EventScriptToken::TK_EQUAL;
			break;
		case '(':
			_NextChar();
			type = EventScriptToken::TK_OPENP;
			break;
		case ')':
			_NextChar();
			type = EventScriptToken::TK_CLOSEP;
			break;
		case '[':
			_NextChar();
			type = EventScriptToken::TK_OPENB;
			break;
		case ']':
			_NextChar();
			type = EventScriptToken::TK_CLOSEB;
			break;
		case '{':
			_NextChar();
			type = EventScriptToken::TK_OPENC;
			break;
		case '}':
			_NextChar();
			type = EventScriptToken::TK_CLOSEC;
			break;
		case '*':
			_NextChar();
			type = EventScriptToken::TK_ASTERISK;
			break;
		case '/':
			_NextChar();
			type = EventScriptToken::TK_SLASH;
			break;
		case ':':
			_NextChar();
			type = EventScriptToken::TK_COLON;
			break;
		case ';':
			_NextChar();
			type = EventScriptToken::TK_SEMICOLON;
			break;
		case '~':
			_NextChar();
			type = EventScriptToken::TK_TILDE;
			break;
		case '!':
			_NextChar();
			type = EventScriptToken::TK_EXCLAMATION;
			break;
		case '#':
			_NextChar();
			type = EventScriptToken::TK_SHARP;
			break;
		case '<':
			_NextChar();
			type = EventScriptToken::TK_LESS;
			break;
		case '>':
			_NextChar();
			type = EventScriptToken::TK_GREATER;
			break;

		case '"': {
			ch = _NextChar(); //1つ進めて
			char pre = ch;
			while (true) {
				if (ch == '"' && pre != '\\')
					break;
				pre = ch;
				ch = _NextChar(); //次のダブルクオーテーションまで進める
			}

			if (ch == '"')
				_NextChar(); //ダブルクオーテーションだったら1つ進める
			else
				_RaiseError(L"Next:すでに文字列終端です");
			type = EventScriptToken::TK_STRING;
			break;
		}

		case '\r':
		case '\n': //改行
			//改行がいつまでも続くようなのも1つの改行として扱う
			while (ch == '\r' || ch == '\n')
				ch = _NextChar();
			type = EventScriptToken::TK_NEWLINE;
			break;

		case '+':
		case '-': {
			if (ch == '+') {
				ch = _NextChar();
				type = EventScriptToken::TK_PLUS;

			} else if (ch == '-') {
				ch = _NextChar();
				type = EventScriptToken::TK_MINUS;
			}

			if (!isdigit(ch))
				break; //次が数字でないなら抜ける
		}

		default: {
			if (isdigit(ch)) {
				//整数か実数
				while (isdigit(ch))
					ch = _NextChar(); //数字だけの間ポインタを進める
				type = EventScriptToken::TK_INT;
				if (ch == '.') {
					//実数か整数かを調べる。小数点があったら実数
					ch = _NextChar();
					while (isdigit(ch))
						ch = _NextChar(); //数字だけの間ポインタを進める
					type = EventScriptToken::TK_REAL;
				}

				if (ch == 'E' || ch == 'e') {
					//1E-5みたいなケース
					auto pos = pointer_;
					ch = _NextChar();
					while (isdigit(ch) || ch == '-')
						ch = _NextChar(); //数字だけの間ポインタを進める
					type = EventScriptToken::TK_REAL;
				}

			} else if (isalpha(ch) || ch == '_') {
				//たぶん識別子
				while (isalpha(ch) || isdigit(ch) || ch == '_')
					ch = _NextChar(); //たぶん識別子な間ポインタを進める
				type = EventScriptToken::TK_ID;
			} else {
				_NextChar();
				type = EventScriptToken::TK_UNKNOWN;
			}
			break;
		}
		}
		if (type == EventScriptScanner::TOKEN_TAG_START)
			bTagScan_ = true;
		else if (type == EventScriptScanner::TOKEN_TAG_END)
			bTagScan_ = false;

		if (type == EventScriptToken::TK_STRING) {
			//\を除去
			std::string str = StringUtility::ReplaceAll(std::string(posStart, pointer_), "\\\"", "\"");
			token_ = EventScriptToken(type, str);
		} else {
			token_ = EventScriptToken(type, std::string(posStart, pointer_));
		}
	}

	return token_;
}
bool EventScriptScanner::HasNext()
{
	return pointer_ != buffer_.end()
		&& *pointer_ != '\0'
		&& token_.GetType() != EventScriptToken::TK_EOF;
}
void EventScriptScanner::CheckType(const EventScriptToken& tok, int type)
{
	if (tok.type_ != type) {
		std::wstring str = StringUtility::Format(L"CheckType error[%s]:", tok.element_.c_str());
		_RaiseError(str);
	}
}
void EventScriptScanner::CheckIdentifer(const EventScriptToken& tok, const std::string& id)
{
	if (tok.type_ != EventScriptToken::TK_ID || tok.GetIdentifier() != id) {
		std::wstring str = StringUtility::Format(L"CheckID error[%s]:", tok.element_.c_str());
		_RaiseError(str);
	}
}
int EventScriptScanner::GetCurrentLine() const
{
	return line_;
}
int EventScriptScanner::SearchCurrentLine()
{
	int line = 1;
	const char* pbuf = &(*buffer_.begin());
	const char* ebuf = &(*pointer_);
	while (pbuf < ebuf) {
		if (*pbuf == '\n')
			line++;
		if (IsDBCSLeadByte(*pbuf))
			pbuf += 2;
		else
			pbuf++;
	}
	return line;
}
std::vector<char>::iterator EventScriptScanner::GetCurrentPointer() const
{
	return pointer_;
}
void EventScriptScanner::SetCurrentPointer(std::vector<char>::iterator pos)
{
	pointer_ = pos;
}
int EventScriptScanner::GetCurrentPosition() const
{
	if (buffer_.empty())
		return 0;
	const char* pos = &*pointer_;
	return pos - &buffer_[0];
}

//EventScriptToken
std::string EventScriptToken::GetIdentifier() const
{
	if (type_ != TK_ID) {
		throw gstd::wexception(L"EventScriptToken::GetIdentifier:データのタイプが違います");
	}
	return element_;
}
std::string EventScriptToken::GetString() const
{
	if (type_ != TK_STRING) {
		throw gstd::wexception(L"EventScriptToken::GetString:データのタイプが違います");
	}
	return element_.substr(1, element_.size() - 2);
}
int EventScriptToken::GetInteger() const
{
	if (type_ != TK_INT) {
		throw gstd::wexception(L"EventScriptToken::GetInterger:データのタイプが違います");
	}
	return atoi(element_.c_str());
}
double EventScriptToken::GetReal() const
{
	if (type_ != TK_REAL && type_ != TK_INT) {
		throw gstd::wexception(L"EventScriptToken::GetReal:データのタイプが違います");
	}
	return atof(element_.c_str());
}
bool EventScriptToken::GetBoolean() const
{
	if (type_ == TK_REAL && type_ == TK_INT) {
		return GetReal() == 1;
	} else {
		return element_ == "true";
	}
}

/**********************************************************
//EventScriptCompiler
**********************************************************/
EventScriptCompiler::EventScriptCompiler() = default;
EventScriptCompiler::~EventScriptCompiler() = default;
void EventScriptCompiler::_ParseBlock(ref_count_ptr<EventScriptCode> blockStartCode)
{
	while (scan_->HasNext()) {
		EventScriptToken& tok = scan_->Next();
		int typeToken = tok.GetType();
		if (typeToken == EventScriptToken::TK_TEXT) {
			std::string element = tok.GetElement();
			element = StringUtility::ReplaceAll(element, "\r", "");
			element = StringUtility::ReplaceAll(element, "\n", "");
			element = StringUtility::ReplaceAll(element, "\t", "");
			if (element.empty())
				continue;
			ref_count_ptr<EventScriptCode_Text> code = new EventScriptCode_Text();
			code->SetLine(scan_->GetCurrentLine());
			code->SetText(element);
			source_->AddCode(code);
		} else if (typeToken == EventScriptScanner::TOKEN_TAG_START) {
			scan_->Next();
			ref_count_ptr<EventScriptCode> res = _ParseTag(blockStartCode);
			if (res != nullptr) {
				source_->AddCode(res);
			}
		}

		/*
		for (const auto& code : source_->code_) {
			std::string log = StringUtility::Format("code:type[%d] line[%d]", code->GetType(), code->GetLine());
			Logger::WriteTop(log);
		}
		Logger::WriteTop("");
		*/
	}
}
ref_count_ptr<EventScriptCode> EventScriptCompiler::_ParseTag(ref_count_ptr<EventScriptCode> blockStartCode)
{
	ref_count_ptr<EventScriptCode> res = nullptr;
	EventScriptToken& tok = scan_->GetToken();
	std::string element = tok.GetElement();
	if (element == "event_block_start") {
		int pos = source_->GetCodeCount();
		ref_count_ptr<EventScriptBlock_Main> block = new EventScriptBlock_Main();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "name") {
				std::string name = value.GetString();
				block->SetName(name);
			}
		}
		block->SetStartPosition(pos);
		std::vector<ref_count_ptr<EventScriptBlock>> blocks;
		blocks.push_back(block);
		block_.push_back(blocks);
	} else if (element == "event_block_end") {
		int pos = source_->GetCodeCount();
		std::vector<ref_count_ptr<EventScriptBlock>> blocks = *block_.rbegin();
		block_.pop_back();
		ref_count_ptr<EventScriptBlock_Main> block = ref_count_ptr<EventScriptBlock_Main>::DownCast(blocks[0]);

		block->SetEndPosition(pos - 1);
		block->SetReturnPosition(EventScriptBlock::POS_NULL);

		std::string name = block->GetName();
		source_->event_[name] = block;
	} else if (element == EventScriptScanner::TAG_NEW_LINE) {
		res = new EventScriptCode_NextLine();
	} else if (element == EventScriptCode::TAG_WAIT_CLICK) {
		res = new EventScriptCode_WaitClick();
	} else if (element == EventScriptCode::TAG_WAIT_NEXT_PAGE) {
		res = new EventScriptCode_WaitNextPage();
	} else if (element == EventScriptCode::TAG_WAIT_TIME) {
		auto* code = new EventScriptCode_WaitTime();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "frame") {
				code->SetTime(value.GetElement());
			} else if (key == "skip") {
				code->SetSkipEnable(value.GetBoolean());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_CLEAR_MESSAGE) {
		res = new EventScriptCode_ClearMessage();
	} else if (element == EventScriptCode::TAG_NAME) {
		auto* code = new EventScriptCode_Name();
		auto pos = scan_->GetCurrentPointer();
		EventScriptToken tok = scan_->Next();
		int type = tok.GetType();
		if (type == EventScriptScanner::TOKEN_TAG_END) {
			// scan_->SetCurrentPointer(pos);
		} else if (type == EventScriptToken::TK_STRING || type == EventScriptToken::TK_ID) {
			std::string name = tok.GetElement();
			code->SetName(name);
		} else {
			std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
			for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
				std::string key = itr->first;
				EventScriptToken value = itr->second;
				if (key == "name") {
					code->SetName(value.GetElement());
				}
			}
		}

		res = code;
	} else if (element == EventScriptCode::TAG_TRANSITION) {
		auto* code = new EventScriptCode_Transition();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "type") {
				int type = EventScriptCode_Transition::TRANS_NONE;
				std::string strTrans = value.GetElement();
				if (strTrans == "fade")
					type = EventScriptCode_Transition::TRANS_FADE;
				code->SetTransType(type);
			} else if (key == "frame") {
				code->SetFrame(value.GetElement());
			} else if (key == "path") {
				code->SetPath(value.GetElement());
				code->SetTransType(EventScriptCode_Transition::TRANS_SCRIPT);
			} else if (key == "method") {
				code->SetMethod(value.GetElement());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_VISIBLE_TEXT) {
		auto* code = new EventScriptCode_VisibleText();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "visible") {
				code->SetVisible(value.GetBoolean());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_VAR) {
		auto* code = new EventScriptCode_Var();
		std::string name = scan_->Next().GetElement();
		scan_->CheckType(scan_->Next(), EventScriptToken::TK_EQUAL);
		std::string value = _GetNextTagElement();

		if (value[0] == '"' && value[value.size() - 1] == '"')
			value = StringUtility::ReplaceAll(value, "\"", "\\\"", 1, value.size() - 1);
		scan_->CheckType(scan_->Next(), EventScriptScanner::TOKEN_TAG_END);
		code->SetName(name);
		code->SetValue(value);
		res = code;
	} else if (element == EventScriptCode::TAG_EVAL) {
		auto* code = new EventScriptCode_Eval();
		std::string name = scan_->Next().GetElement();
		scan_->CheckType(scan_->Next(), EventScriptToken::TK_EQUAL);
		std::string value = _GetNextTagElement();
		scan_->CheckType(scan_->Next(), EventScriptScanner::TOKEN_TAG_END);
		code->SetName(name);
		code->SetValue(value);
		res = code;
	} else if (element == EventScriptCode::TAG_SYSVAL) {
		auto* code = new EventScriptCode_SysVal();
		std::string name = scan_->Next().GetElement();
		scan_->CheckType(scan_->Next(), EventScriptToken::TK_EQUAL);
		std::string value = _GetNextTagElement();
		scan_->CheckType(scan_->Next(), EventScriptScanner::TOKEN_TAG_END);
		code->SetName(name);
		code->SetValue(value);
		res = code;
	} else if (element == EventScriptCode::TAG_OUTPUT) {
		auto* code = new EventScriptCode_Output();
		scan_->CheckIdentifer(scan_->Next(), "msg");
		scan_->CheckType(scan_->Next(), EventScriptToken::TK_EQUAL);
		std::string value = _GetNextTagElement();
		scan_->CheckType(scan_->Next(), EventScriptScanner::TOKEN_TAG_END);
		code->SetValue(value);
		res = code;
	} else if (element == EventScriptCode::TAG_IMAGE) {
		auto* code = new EventScriptCode_Image();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "id")
				code->SetObjectIdentifier(value.GetElement());
			else if (key == "path") {
				if (value.GetType() == EventScriptToken::TK_STRING)
					code->SetPath(value.GetString());
				else
					code->SetPath(value.GetElement());
			} else if (key == "layer") {
				int layer = 0;
				std::string strLayer = value.GetElement();
				if (strLayer == "fore")
					layer = EventImage::LAYER_FOREGROUND;
				else if (strLayer == "back")
					layer = EventImage::LAYER_BACKGROUND;
				else
					layer = value.GetInteger();
				code->SetLayer(layer);
			} else if (key == "pri")
				code->SetPriority(value.GetElement());
			else if (key == "visible")
				code->SetVisible(value.GetElement());
			else if (key == "trans")
				code->SetTransition(value.GetElement());
			else if (key == "left_dest")
				code->SetLeftDestPoint(value.GetElement());
			else if (key == "top_dest")
				code->SetTopDestPoint(value.GetElement());
			else if (key == "wait")
				code->SetWaitEnd(value.GetElement());
			else if (key == "delete") {
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_BGM || element == EventScriptCode::TAG_SE) {
		auto* code = new EventScriptCode_Sound();
		if (element == EventScriptCode::TAG_BGM)
			code->SetSoundType(EventSound::TYPE_BGM);
		else
			code->SetSoundType(EventSound::TYPE_SE);

		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "path") {
				if (value.GetType() == EventScriptToken::TK_STRING)
					code->SetPath(value.GetString());
				else
					code->SetPath(value.GetElement());
			} else if (key == "delete") {
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_IF || element == EventScriptCode::TAG_ELSEIF) {
		ref_count_ptr<EventScriptCode_If> code = new EventScriptCode_If();
		int pos = source_->GetCodeCount();
		code->SetStartPosition(pos + 1);
		std::string param = _GetNextTagElement();
		if (!param.empty()) {
			code->SetParameter(param);
		}

		if (element == EventScriptCode::TAG_IF) {
			std::vector<ref_count_ptr<EventScriptBlock>> blocks;
			blocks.push_back(code);
			block_.push_back(blocks);
		} else if (element == EventScriptCode::TAG_ELSEIF) {
			std::vector<ref_count_ptr<EventScriptBlock>>& blocks = *block_.rbegin();
			ref_count_ptr<EventScriptCode_If> preIf = ref_count_ptr<EventScriptCode_If>::DownCast(*blocks.rbegin());
			preIf->SetNextElsePosition(pos);

			blocks.push_back(code);
		}
		res = code;
	} else if (element == EventScriptCode::TAG_ENDIF) {
		int pos = source_->GetCodeCount();
		std::vector<ref_count_ptr<EventScriptBlock>>& blocks = *block_.rbegin();
		for (int iIf = 0; iIf < blocks.size(); iIf++) {
			ref_count_ptr<EventScriptCode_If> block = ref_count_ptr<EventScriptCode_If>::DownCast(blocks[iIf]);
			int posEnd = pos;
			if (iIf < blocks.size() - 1) {
				ref_count_ptr<EventScriptCode_If> next = ref_count_ptr<EventScriptCode_If>::DownCast(blocks[iIf + 1]);
				posEnd = next->GetStartPosition() - 2;
			} else {
				posEnd = pos - 1;
			}
			block->SetEndPosition(posEnd);
			block->SetReturnPosition(pos);
		}
		block_.pop_back();
	} else if (element == EventScriptCode::TAG_GOSUB || element == EventScriptCode::TAG_GOTO) {
		auto* code = new EventScriptCode_Jump();
		code->SetGosub(element == EventScriptCode::TAG_GOSUB);

		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "path") {
				code->SetPath(value.GetElement());
			} else if (key == "name") {
				code->SetName(value.GetElement());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_SCRIPT) {
		auto* code = new EventScriptCode_Script();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "path") {
				code->SetPath(value.GetElement());
			} else if (key == "method") {
				code->SetMethod(value.GetElement());
			} else if (key == "wait") {
				if (value.GetType() == EventScriptToken::TK_STRING)
					code->SetWaitEnd(value.GetString());
				else
					code->SetWaitEnd(value.GetElement());
			} else if (key == "target") {
				if (value.GetType() == EventScriptToken::TK_STRING)
					code->SetTargetId(value.GetString());
				else
					code->SetTargetId(value.GetElement());
			} else if (key == "code") {
				code->SetCode(value.GetElement());
			} else if (key == "id") {
				code->SetId(value.GetString());
			} else if (key == "delete") {
				code->SetEndScript(true);
			} else if (key.find("arg") != std::string::npos) {
				if (key == "arg")
					code->SetArgument(0, value.GetElement());
				else {
					std::string str = StringUtility::ReplaceAll(key, "arg", "");
					int index = StringUtility::ToInteger(str);
					code->SetArgument(index, value.GetElement());
				}
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_SCRIPT_END) {
		auto* code = new EventScriptCode_Script();
		code->SetEndScript(true);
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "path") {
				code->SetPath(value.GetElement());
			} else if (key == "method") {
				code->SetMethod(value.GetElement());
			} else if (key == "id") {
				code->SetId(value.GetString());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_END) {
		auto* code = new EventScriptCode_End();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "arg") {
				code->SetArgument(value.GetString());
			}
		}
		res = code;
	} else if (element == EventScriptCode::TAG_BATTLE) {
		auto* code = new EventScriptCode_Battle();
		std::map<std::string, EventScriptToken> mapElement = _GetAllTagElement();
		for (auto itr = mapElement.begin(); itr != mapElement.end(); ++itr) {
			std::string key = itr->first;
			EventScriptToken value = itr->second;
			if (key == "path") {
				if (value.GetType() == EventScriptToken::TK_STRING)
					code->SetPath(value.GetString());
				else
					code->SetPath(value.GetElement());
			}
		}
		res = code;
	} else {
		std::string text = EventScriptScanner::TAG_START + element + " ";
		ref_count_ptr<EventScriptCode_Text> code = new EventScriptCode_Text();
		text += _GetNextTagElement();
		scan_->CheckType(scan_->Next(), EventScriptScanner::TOKEN_TAG_END);
		text += EventScriptScanner::TAG_END;
		code->SetText(text);
		res = code;
	}

	if (res != nullptr) {
		int line = scan_->GetCurrentLine();
		res->SetLine(line);
	}

	return res;
}
std::map<std::string, EventScriptToken> EventScriptCompiler::_GetAllTagElement()
{
	std::map<std::string, EventScriptToken> res;
	auto pos = scan_->GetCurrentPointer();
	while (true) {
		pos = scan_->GetCurrentPointer();
		EventScriptToken& tok = scan_->Next();
		if (tok.GetType() == EventScriptScanner::TOKEN_TAG_END)
			break;
		std::string key = tok.GetElement();
		tok = scan_->Next();
		if (tok.GetType() != EventScriptToken::TK_EQUAL) {
			res[key] = EventScriptToken();
			if (tok.GetType() == EventScriptScanner::TOKEN_TAG_END)
				break;
			continue;
		}

		tok = scan_->Next();
		res[key] = tok;
	}
	scan_->SetCurrentPointer(pos);
	scan_->SetTagScanEnable(true);
	return res;
}
std::string EventScriptCompiler::_GetNextTagElement()
{
	return _GetNextTagElement(EventScriptScanner::TOKEN_TAG_END);
}
std::string EventScriptCompiler::_GetNextTagElement(int type)
{
	std::string res;
	auto pos = scan_->GetCurrentPointer();
	while (true) {
		pos = scan_->GetCurrentPointer();
		EventScriptToken& tok = scan_->Next();
		if (tok.GetType() == type)
			break;
		if (tok.GetType() == EventScriptToken::TK_NEWLINE)
			continue;
		if (tok.GetType() == EventScriptScanner::TOKEN_TAG_END)
			break;
		res += tok.GetElement();
	}
	scan_->SetCurrentPointer(pos);
	scan_->SetTagScanEnable(true);
	return res;
}
gstd::ref_count_ptr<EventScriptSource> EventScriptCompiler::Compile()
{
	FileManager* fileManager = FileManager::GetBase();
	ref_count_ptr<FileReader> reader = fileManager->GetFileReader(path_);
	if (reader == nullptr || !reader->Open()) {
		Logger::WriteTop(ErrorUtility::GetFileNotFoundErrorMessage(path_));
		return false;
	}

	int size = reader->GetFileSize();
	std::vector<char> text;
	text.resize(size + 1);
	reader->Read(&text[0], size);
	text[size] = '\0';

	scan_ = new EventScriptScanner(text);
	source_ = new EventScriptSource();

	try {
		_ParseBlock(nullptr);
	} catch (gstd::wexception& e) {
		int line = scan_->GetCurrentLine();
		throw gstd::wexception(StringUtility::Format(L"%s line[%d]", e.what(), line));
	}

	return source_;
}

/**********************************************************
//EventScriptBlock
**********************************************************/
const std::string EventScriptBlock::BLOCK_GLOBAL = "global";

/**********************************************************
//EventScriptCode
**********************************************************/
const std::string EventScriptCode::TAG_WAIT_CLICK = "l";
const std::string EventScriptCode::TAG_WAIT_NEXT_PAGE = "p";
const std::string EventScriptCode::TAG_WAIT_TIME = "wait";
const std::string EventScriptCode::TAG_CLEAR_MESSAGE = "cm";
const std::string EventScriptCode::TAG_NAME = "nm";
const std::string EventScriptCode::TAG_TRANSITION = "trans";
const std::string EventScriptCode::TAG_VISIBLE_TEXT = "vw";
const std::string EventScriptCode::TAG_VAR = "var";
const std::string EventScriptCode::TAG_EVAL = "eval";
const std::string EventScriptCode::TAG_SYSVAL = "sysval";
const std::string EventScriptCode::TAG_OUTPUT = "output";
const std::string EventScriptCode::TAG_IMAGE = "image";
const std::string EventScriptCode::TAG_BGM = "bgm";
const std::string EventScriptCode::TAG_SE = "se";
const std::string EventScriptCode::TAG_IF = "if";
const std::string EventScriptCode::TAG_ELSEIF = "elseif";
const std::string EventScriptCode::TAG_ENDIF = "endif";
const std::string EventScriptCode::TAG_GOSUB = "gosub";
const std::string EventScriptCode::TAG_GOTO = "goto";
const std::string EventScriptCode::TAG_SCRIPT = "script";
const std::string EventScriptCode::TAG_SCRIPT_END = "script_end";
const std::string EventScriptCode::TAG_END = "end";
const std::string EventScriptCode::TAG_BATTLE = "battle";

const std::string EventScriptCode::STRING_INVALID = "__VALUE_INVALID__";

EventScriptCode::EventScriptCode()
{
	line_ = -1;
	type_ = TYPE_UNKNOWN;
}
EventScriptCode::~EventScriptCode() = default;

//EventScriptCode_Text
EventScriptCode_Text::EventScriptCode_Text()
{
	type_ = TYPE_TEXT;
}

//EventScriptCode_NextLine
EventScriptCode_NextLine::EventScriptCode_NextLine()
{
	type_ = TYPE_NEXT_LINE;
}
std::string EventScriptCode_NextLine::GetCodeText() const
{
	return EventScriptScanner::TAG_START + EventScriptScanner::TAG_NEW_LINE + EventScriptScanner::TAG_END;
}

//EventScriptCode_WaitClick
EventScriptCode_WaitClick::EventScriptCode_WaitClick()
{
	type_ = TYPE_WAIT_CLICK;
}

//EventScriptCode_WaitTime
EventScriptCode_WaitTime::EventScriptCode_WaitTime()
{
	type_ = TYPE_WAIT_TIME;
	time_ = "0";
	bSkipEnable_ = true;
}

//EventScriptCode_WaitNextPage
EventScriptCode_WaitNextPage::EventScriptCode_WaitNextPage()
{
	type_ = TYPE_WAIT_NEXT_PAGE;
}

//EventScriptCode_ClearMessage
EventScriptCode_ClearMessage::EventScriptCode_ClearMessage()
{
	type_ = TYPE_CLEAR_MESSAGE;
}

//EventScriptCode_Name
EventScriptCode_Name::EventScriptCode_Name()
{
	type_ = TYPE_NAME;
}

//EventScriptCode_Transition
EventScriptCode_Transition::EventScriptCode_Transition()
{
	type_ = TYPE_TRANSITION;
	typeTrans_ = TRANS_NONE;
	frame_ = "30";
}

//EventScriptCode_VisibleText
EventScriptCode_VisibleText::EventScriptCode_VisibleText()
{
	type_ = TYPE_VISIBLE_TEXT;
	bVisible_ = true;
}

//EventScriptCode_Var
EventScriptCode_Var::EventScriptCode_Var()
{
	type_ = TYPE_VAR;
}

//EventScriptCode_Eval
EventScriptCode_Eval::EventScriptCode_Eval()
{
	type_ = TYPE_EVAL;
}

//EventScriptCode_SysVal
EventScriptCode_SysVal::EventScriptCode_SysVal()
{
	type_ = TYPE_SYSVAL;
	bGlobal_ = false;
}

//EventScriptCode_Output
EventScriptCode_Output::EventScriptCode_Output()
{
	type_ = TYPE_OUTPUT;
}

//EventScriptCode_Image
EventScriptCode_Image::EventScriptCode_Image()
{
	type_ = TYPE_IMAGE;
	idObject_ = STRING_INVALID;
	layer_ = EventImage::LAYER_FOREGROUND;
	pri_ = STRING_INVALID;
	bVisible_ = "true";
	posDestLeft_ = STRING_INVALID;
	posDestTop_ = STRING_INVALID;
	bTransition_ = "true";
	bWaitEnd_ = "true";
}

//EventScriptCode_Sound
EventScriptCode_Sound::EventScriptCode_Sound()
{
	type_ = TYPE_SOUND;
}

//EventScriptCode_If
EventScriptCode_If::EventScriptCode_If()
{
	type_ = TYPE_IF;
	bInner_ = true;
	param_ = "true";
	posNextElse_ = -1;
}

//EventScriptCode_Jump
EventScriptCode_Jump::EventScriptCode_Jump()
{
	type_ = TYPE_JUMP;
	bGoSub_ = false;
}

//EventScriptCode_Script
EventScriptCode_Script::EventScriptCode_Script()
{
	type_ = TYPE_SCRIPT;
	bWaitEnd_ = "true";
	bEndScript_ = false;
	targetId_ = StringUtility::Format("%d", DxScript::ID_INVALID);
}

//EventScriptCode_End
EventScriptCode_End::EventScriptCode_End()
{
	type_ = TYPE_END;
}

//EventScriptCode_Battle
EventScriptCode_Battle::EventScriptCode_Battle()
{
	type_ = TYPE_BATTLE;
}
/**********************************************************
//EventScriptCodeExecuter
**********************************************************/
EventScriptCodeExecuter::EventScriptCodeExecuter(EventEngine* engine)
{
	engine_ = engine;
	bEnd_ = false;
}
EventScriptCodeExecuter::~EventScriptCodeExecuter() = default;
int EventScriptCodeExecuter::_GetElementInteger(const std::string& value)
{
	EventValueParser parser(engine_);
	ref_count_ptr<EventValue> res = parser.GetEventValue(value);
	return floorl(res->GetReal() + 0.5);
}
double EventScriptCodeExecuter::_GetElementReal(const std::string& value)
{
	EventValueParser parser(engine_);
	ref_count_ptr<EventValue> res = parser.GetEventValue(value);
	return res->GetReal();
}
bool EventScriptCodeExecuter::_GetElementBoolean(const std::string& value)
{
	EventValueParser parser(engine_);
	ref_count_ptr<EventValue> res = parser.GetEventValue(value);
	return res->GetBoolean();
}
std::string EventScriptCodeExecuter::_GetElementString(const std::string& value)
{
	EventValueParser parser(engine_);
	ref_count_ptr<EventValue> res = parser.GetEventValue(value);
	return res->GetString();
}
bool EventScriptCodeExecuter::_IsValidElement(const std::string& value)
{
	return value != EventScriptCode::STRING_INVALID;
}
//EventScriptCodeExecuter_WaitClick
void EventScriptCodeExecuter_WaitClick::Execute()
{
	gstd::ref_count_ptr<EventKeyState> keyState = engine_->GetEventKeyState();
	bool bkeyNext = keyState->IsNext();
	bool bKeySkip = keyState->IsSkip();

	if (bkeyNext || bKeySkip) {
		bEnd_ = true;
	}
}

//EventScriptCodeExecuter_WaitNextPage
void EventScriptCodeExecuter_WaitNextPage::Execute()
{
	gstd::ref_count_ptr<EventKeyState> keyState = engine_->GetEventKeyState();
	bool bkeyNext = keyState->IsNext();
	bool bKeySkip = keyState->IsSkip();

	if (bkeyNext || bKeySkip) {
		bEnd_ = true;
	}
}

//EventScriptCodeExecuter_WaitTime
EventScriptCodeExecuter_WaitTime::EventScriptCodeExecuter_WaitTime(EventEngine* engine, EventScriptCode_WaitTime* code)
	: EventScriptCodeExecuter(engine)
{
	count_ = 0;
	time_ = _GetElementInteger(code->GetTime());
	bSkipEnable_ = code->IsSkipEnable();
}
void EventScriptCodeExecuter_WaitTime::Execute()
{
	if (count_ >= time_)
		bEnd_ = true;
	if (bSkipEnable_) {
		gstd::ref_count_ptr<EventKeyState> keyState = engine_->GetEventKeyState();
		bool bKeySkip = keyState->IsSkip();
		if (bKeySkip)
			bEnd_ = true;
	}
	++count_;
}

//EventScriptCodeExecuter_Transition
EventScriptCodeExecuter_Transition::EventScriptCodeExecuter_Transition(EventEngine* engine, EventScriptCode_Transition* code)
	: EventScriptCodeExecuter(engine)
{
	code_ = code;
	frame_ = 0;
}
void EventScriptCodeExecuter_Transition::Execute()
{
	if (frame_ == 0) {
		DirectGraphics* graphics = DirectGraphics::GetBase();
		TextureManager* manager = TextureManager::GetBase();
		gstd::ref_count_ptr<EventImage> image = engine_->GetEventImage();

		ref_count_ptr<Texture> texture = manager->GetTexture(TextureManager::TARGET_TRANSITION);
		graphics->SetRenderTarget(texture);
		graphics->ClearRenderTarget();
		graphics->BeginScene();

		//背景は黒で塗りつぶす
		{
			Sprite2D back;
			RECT_D rcDest = { 0., 0., (double)graphics->GetScreenWidth(), (double)graphics->GetScreenHeight() };
			back.SetColorRGB(D3DCOLOR_ARGB(255, 0, 0, 0));
			back.SetDestinationRect(rcDest);
			back.Render();
		}

		image->Render(image->GetForegroundLayerIndex());

		graphics->EndScene();
		graphics->SetRenderTarget(nullptr);
		image->SwapForeBackLayerIndex();

		int layer = image->GetForegroundLayerIndex();
		gstd::ref_count_ptr<DxScriptObjectManager> objManager = image->GetObjectManager(layer);
		DxScriptObjectBase* obj = nullptr;
		int type = code_->GetTransType();
		switch (type) {
		case EventScriptCode_Transition::TRANS_NONE:
			break;
		case EventScriptCode_Transition::TRANS_FADE: {
			int frame = _GetElementInteger(code_->GetFrame());
			auto* effect = new TransitionEffect_FadeOut();
			effect->Initialize(frame, texture);
			effect_ = effect;
			obj = new DxScriptRenderObject_Transition(effect_.GetPointer());

			objManager->AddObject(EventImage::ID_TRANSITION, obj);
			break;
		}
		case EventScriptCode_Transition::TRANS_SCRIPT: {
			RECT_D rect = { 0., 0., (double)graphics->GetScreenWidth(), (double)graphics->GetScreenHeight() };
			auto* sprite = new DxScriptSpriteObject2D();
			sprite->SetTexture(texture);
			sprite->SetRenderPriority(1.0);
			sprite->GetSpritePointer()->SetDestinationRect(rect);
			sprite->GetSpritePointer()->SetSourceRect(rect);
			obj = sprite;
			objManager->AddObject(EventImage::ID_TRANSITION, obj);

			std::string path = _GetElementString(code_->GetPath());
			std::string method = _GetElementString(code_->GetMethod());
			std::wstring wPath = StringUtility::ConvertMultiToWide(path);
			script_ = new DxScriptForEvent(engine_);
			script_->SetTargetId(EventImage::ID_TRANSITION);
			script_->SetMethod(method);
			script_->SetSourceFromFile(wPath);
			script_->Compile();
			script_->Run(method);

			break;
		}
		}
	}

	if (effect_ == nullptr && script_ == nullptr) {
		bEnd_ = true;
		return;
	}

	if (effect_ != nullptr)
		effect_->Work();
	else if (script_ != nullptr)
		script_->Run("MainLoop");

	gstd::ref_count_ptr<EventKeyState> keyState = engine_->GetEventKeyState();
	bool bkeyNext = keyState->IsNext();
	bool bKeySkip = keyState->IsSkip();

	if (effect_ != nullptr)
		bEnd_ = effect_->IsEnd();
	else if (script_ != nullptr)
		bEnd_ = script_->IsScriptEnd();

	if (bEnd_ || bkeyNext || bKeySkip) {
		gstd::ref_count_ptr<EventImage> image = engine_->GetEventImage();
		int layer = image->GetForegroundLayerIndex();
		gstd::ref_count_ptr<DxScriptObjectManager> objManager = image->GetObjectManager(layer);
		objManager->DeleteObject(EventImage::ID_TRANSITION);
	}
	++frame_;
}

//EventScriptCodeExecuter_Image
EventScriptCodeExecuter_Image::EventScriptCodeExecuter_Image(EventEngine* engine, EventScriptCode_Image* code)
	: EventScriptCodeExecuter(engine)
{
	code_ = code;
	frame_ = 0;
	bTrans_ = false;

	EventValueParser parser(engine);
	std::string strPri = code_->GetPriority();
	ref_count_ptr<EventValue> val = parser.GetEventValue(strPri);
	priority_ = val->GetReal();
}
EventScriptCodeExecuter_Image::~EventScriptCodeExecuter_Image()
{
	objManager_ = nullptr;
	nowSprite_ = nullptr;
	oldSprite_ = nullptr;
}
void EventScriptCodeExecuter_Image::_Initialize()
{
	ref_count_ptr<EventImage> imageManager = engine_->GetEventImage();
	int layer = code_->GetLayer();
	if (layer == EventImage::LAYER_FOREGROUND)
		layer = imageManager->GetForegroundLayerIndex();
	else if (layer == EventImage::LAYER_BACKGROUND)
		layer = imageManager->GetBackgroundLayerIndex();
	objManager_ = imageManager->GetObjectManager(layer);

	std::string path = _GetElementString(code_->GetPath());
	std::wstring wPath = StringUtility::ConvertMultiToWide(path);
	int idObj = _GetElementInteger(code_->GetObjectIdentifier());
	nowSprite_ = ref_count_ptr<DxScriptSpriteObject2D>::unsync::DownCast(objManager_->GetObject(idObj));
	if (nowSprite_ == nullptr && !path.empty()) {
		nowSprite_ = new DxScriptSpriteObject2D();
		nowSprite_->SetRenderPriority(priority_);
		objManager_->AddObject(idObj, nowSprite_);
		bTrans_ = true;
	}

	//パス確認
	if (nowSprite_ != nullptr) {
		ref_count_ptr<Texture> texture = nowSprite_->GetObjectPointer()->GetTexture();
		if (texture == nullptr) {
			texture = new Texture();
			// texture->CreateFromFile(path);
			texture->CreateFromFileInLoadThread(wPath, true);
			nowSprite_->SetTexture(texture);
			Sprite2D* sprite = nowSprite_->GetSpritePointer();

			RECT_D rect = { 0., 0., (double)texture->GetWidth(), (double)texture->GetHeight() };
			sprite->SetDestinationRect(rect);
			sprite->SetVertex(rect, rect, D3DCOLOR_ARGB(255, 255, 255, 255));
			bTrans_ = true;
		}

		if (!path.empty()) {
			if (texture->GetName() != wPath) {
				//画像変更
				int idObjOld = EventImage::INDEX_OLD_START + idObj;
				oldSprite_ = new DxScriptSpriteObject2D();
				oldSprite_->Copy(nowSprite_.GetPointer());
				objManager_->AddObject(idObjOld, oldSprite_);

				Sprite2D* sprite = nowSprite_->GetSpritePointer();
				texture = new Texture();
				// texture->CreateFromFile(path);
				texture->CreateFromFileInLoadThread(wPath, true);
				nowSprite_->SetTexture(texture);
				RECT_D rect = { 0., 0., (double)texture->GetWidth(), (double)texture->GetHeight() };
				sprite->SetDestinationRect(rect);
				sprite->SetVertex(rect, rect, D3DCOLOR_ARGB(255, 255, 255, 255));
				bTrans_ = true;
			}
		} else {
			int idObjOld = EventImage::INDEX_OLD_START + idObj;
			oldSprite_ = new DxScriptSpriteObject2D();
			oldSprite_->Copy(nowSprite_.GetPointer());
			objManager_->AddObject(idObjOld, oldSprite_);

			objManager_->DeleteObject(idObj);
			nowSprite_ = nullptr;
			bTrans_ = true;
		}
	}

	if (nowSprite_ != nullptr) {
		if (_IsValidElement(code_->GetLeftDestPoint()))
			nowSprite_->SetX(_GetElementInteger(code_->GetLeftDestPoint()));
		if (_IsValidElement(code_->GetTopDestPoint()))
			nowSprite_->SetY(_GetElementInteger(code_->GetTopDestPoint()));
		if (_IsValidElement(code_->GetPriority()))
			nowSprite_->SetRenderPriority(_GetElementReal(code_->GetPriority()));
	}

	if (_IsValidElement(code_->GetTransition()))
		bTrans_ &= _GetElementBoolean(code_->GetTransition());
}
void EventScriptCodeExecuter_Image::Execute()
{
	const int frameTrans = 10;
	if (frame_ == 0)
		_Initialize();

	gstd::ref_count_ptr<EventKeyState> keyState = engine_->GetEventKeyState();
	bool bkeyNext = keyState->IsNext();
	bool bKeySkip = keyState->IsSkip();

	if (frame_ >= frameTrans || !bTrans_ || bkeyNext || bKeySkip) {
		if (oldSprite_ != nullptr) {
			objManager_->DeleteObject(oldSprite_->GetObjectID());
		}
		if (nowSprite_ != nullptr)
			nowSprite_->SetAlpha(255);
		bEnd_ = true;
		return;
	}

	double dAlpha = (double)255 / (double)frameTrans;

	if (nowSprite_ != nullptr)
		nowSprite_->SetAlpha(dAlpha * frame_);
	if (oldSprite_ != nullptr) {
		oldSprite_->SetAlpha(255 - dAlpha * frame_);
	}

	++frame_;
}

//EventScriptCodeExecuter_Script
EventScriptCodeExecuter_Script::EventScriptCodeExecuter_Script(EventEngine* engine, DxScriptForEvent* script)
	: EventScriptCodeExecuter(engine)
{
	script_ = script;
}
void EventScriptCodeExecuter_Script::Execute()
{
	if (script_->IsScriptEnd())
		bEnd_ = true;
}
/**********************************************************
//EventWindowManager
**********************************************************/
EventWindowManager::EventWindowManager(EventEngine* engine)
{
	engine_ = engine;
	bVisibleText_ = true;
}
bool EventWindowManager::Initialize()
{
	//セーブボタン

	//ロードボタン

	//キャプチャレイヤ
	layerCapture_ = new EventMouseCaptureLayer();
	AddWindow(layerCapture_);

	//テキストウィンドウ
	wndText_ = new EventTextWindow();
	AddWindow(wndText_);

	//ログウィンドウ
	wndLog_ = new EventLogWindow();
	AddWindow(wndLog_);
	wndLog_->SetWindowEnable(false);
	wndLog_->SetWindowVisible(false);

	//名前ウィンドウ
	wndName_ = new EventNameWindow();

	return true;
}
void EventWindowManager::Work()
{
	layerCapture_->ClearEvent();
	DxWindowManager::Work();
}
void EventWindowManager::Render()
{
	DxWindowManager::Render();
}
void EventWindowManager::Read(gstd::RecordBuffer& record)
{
	//名前欄
	std::wstring name = record.GetRecordAsStringW("name");
	wndName_->SetText(name);
}
void EventWindowManager::Write(gstd::RecordBuffer& record)
{
	//名前欄
	std::wstring name = wndName_->GetText();
	record.SetRecordAsStringW("name", name);
}

//EventMouseCaptureLayer
void EventMouseCaptureLayer::AddedManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	RECT rect = { 0, 0, graphics->GetScreenWidth(), graphics->GetScreenHeight() };
	SetWindowRect(rect);
}
void EventMouseCaptureLayer::DispatchedEvent(gstd::ref_count_ptr<DxWindowEvent> event)
{
	gstd::ref_count_ptr<DxWindow> window = event->GetSourceWindow();
	if (window != nullptr && window->GetID() == GetID()) {
		if (event->HasEventType(DxWindowEvent::TYPE_MOUSE_LEFT_CLICK)) {
			event_ = event;
		}
	}
}
void EventMouseCaptureLayer::ClearEvent()
{
	event_ = nullptr;
}

//EventWindow
bool EventWindow::IsWindowVisible()
{
	bool res = DxWindow::IsWindowVisible();
	if (bApplyVisibleText_) {
		res &= _GetManager()->IsTextVisible();
	}
	return res;
}

//EventWindow
bool EventButton::IsWindowVisible()
{
	bool res = DxButton::IsWindowVisible();
	if (bApplyVisibleText_) {
		res &= _GetManager()->IsTextVisible();
	}
	return res;
}

//EventTextWindow
EventTextWindow::EventTextWindow()
{
	dxText_ = new DxText();
	dxText_->SetFontType(Font::MINCHOH);
	dxText_->SetFontColorTop(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxText_->SetFontColorBottom(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxText_->SetFontBorderType(directx::DxFont::BORDER_SHADOW);
	dxText_->SetFontBorderColor(D3DCOLOR_ARGB(255, 32, 32, 32));
	dxText_->SetFontBorderWidth(2);
	dxText_->SetLinePitch(20);
	dxText_->SetFontSize(24);
	dxText_->SetFontBold(true);

	ZeroMemory(&rcMargin_, sizeof(RECT));
	rcMargin_.left = 16;
	rcMargin_.top = 16;
	rcMargin_.right = 16;
}
void EventTextWindow::AddedManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	RECT rect = { 0, 440, graphics->GetScreenWidth(), 600 };
	SetWindowRect(rect);

	ref_count_ptr<Sprite2D> sprite = new Sprite2D();
	sprite->SetColorRGB(D3DCOLOR_ARGB(255, 64, 64, 64));
	sprite->SetAlpha(192);
	SetFrameSprite(sprite);
}
void EventTextWindow::Render()
{
	_RenderFrame();

	EventEngine* engine = _GetManager()->GetEngine();
	gstd::ref_count_ptr<EventText> textEvent = engine->GetEventText();
	RECT rect = GetWindowRect();
	dxText_->SetPosition(rect.left + rcMargin_.left, rect.top + rcMargin_.top);
	dxText_->SetMaxWidth(rect.right - rect.left - rcMargin_.left - rcMargin_.right);
	dxText_->SetMaxHeight(INT_MAX);
	dxText_->SetText(textEvent->GetText());
	gstd::ref_count_ptr<DxTextInfo> textInfo = dxText_->GetTextInfo();

	bool bVoice = textEvent->IsVoiceText();
	textInfo->SetAutoIndent(bVoice);

	int maxHeight = rect.bottom - rect.top - rcMargin_.top - rcMargin_.bottom;
	dxText_->SetMaxHeight(maxHeight);
	int countLine = textInfo->GetLineCount();
	int heightTotal = 0;
	int lineEnd = countLine;
	int lineStart = lineEnd;
	for (int iLine = countLine - 1; iLine >= 0; iLine--) {
		gstd::ref_count_ptr<DxTextLine> lineText = textInfo->GetTextLine(iLine);
		heightTotal += lineText->GetHeight() + dxText_->GetLinePitch();
		if (heightTotal > maxHeight - 8)
			break;
		--lineStart;
	}
	textInfo->SetValidStartLine(max(lineStart, 1));
	textInfo->SetValidEndLine(lineEnd);
	dxText_->Render(textInfo);
}
bool EventTextWindow::IsWait()
{
	EventEngine* engine = _GetManager()->GetEngine();
	gstd::ref_count_ptr<EventScriptCodeExecuter> executer = engine->GetActiveCodeExecuter();
	return dynamic_cast<EventScriptCodeExecuter_WaitClick*>(executer.GetPointer()) != nullptr;
}
//EventNameWindow
EventNameWindow::EventNameWindow()
{
	text_ = new DxText();
	text_->SetFontType(Font::MINCHOH);
	text_->SetFontColorTop(D3DCOLOR_ARGB(255, 255, 255, 255));
	text_->SetFontColorBottom(D3DCOLOR_ARGB(255, 255, 255, 255));
	text_->SetFontBorderType(directx::DxFont::BORDER_SHADOW);
	text_->SetFontBorderColor(D3DCOLOR_ARGB(255, 32, 32, 32));
	text_->SetFontBorderWidth(2);
	text_->SetLinePitch(20);
	text_->SetFontSize(28);
	text_->SetFontBold(true);
	SetAlpha(0);
}
void EventNameWindow::Work()
{
	DxWindow::Work();
	int alpha = GetAlpha();
	if (text_->GetText().empty()) {
		alpha = max(0, alpha - 8);
	} else
		alpha = 255;
	SetAlpha(alpha);
}
void EventNameWindow::Render()
{
	DxWindow::Render();
	if (!text_->GetText().empty())
		RenderText();
}
void EventNameWindow::RenderText()
{
	int sizeFont = text_->GetFontSize();
	RECT rect = GetAbsoluteWindowRect();
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	text_->SetMaxWidth(width);
	text_->SetHorizontalAlignment(DxText::ALIGNMENT_CENTER);
	text_->SetPosition(rect.left, rect.top + (height - sizeFont) / 2 - 2);
	text_->Render();
}

//EventLogWindow
EventLogWindow::EventLogWindow()
{
	pos_ = 0;
	dxText_ = new DxText();
	dxText_->SetFontType(Font::MINCHOH);
	dxText_->SetFontColorTop(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxText_->SetFontColorBottom(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxText_->SetFontBorderType(directx::DxFont::BORDER_SHADOW);
	dxText_->SetFontBorderColor(D3DCOLOR_ARGB(255, 32, 32, 32));
	dxText_->SetFontBorderWidth(2);
	dxText_->SetLinePitch(20);
	dxText_->SetFontSize(24);
	dxText_->SetFontBold(true);
}
void EventLogWindow::AddedManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	RECT rect = { 20, 20, graphics->GetScreenWidth() - 20, 580 };
	SetWindowRect(rect);

	ref_count_ptr<Sprite2D> sprite = new Sprite2D();
	sprite->SetColorRGB(D3DCOLOR_ARGB(255, 64, 64, 64));
	sprite->SetAlpha(192);
	SetFrameSprite(sprite);
}
void EventLogWindow::Work()
{
	if (!IsWindowEnable())
		return;

	EventEngine* engine = _GetManager()->GetEngine();
	DirectInput* input = DirectInput::GetBase();
	gstd::ref_count_ptr<EventLogText> log = engine->GetEventLogText();
	int count = log->GetInfoCount();

	bool bChange = false;
	int mouseZ = input->GetMouseMoveZ();
	if (mouseZ > 0)
		pos_ = min(count - 1, pos_ + 1);
	else if (mouseZ < 0) {
		if (pos_ == posMin_ || count == 0)
			bChange = true;
		else
			pos_ = max(posMin_, pos_ - 1);
	}

	if (input->GetMouseState(DI_MOUSE_RIGHT) == KEY_PULL)
		bChange = true;

	if (bChange)
		engine->SetState(EventEngine::STATE_RUN);
}
void EventLogWindow::Render()
{
	_RenderFrame();

	EventEngine* engine = _GetManager()->GetEngine();
	gstd::ref_count_ptr<EventLogText> log = engine->GetEventLogText();
	if (log->GetInfoCount() == 0)
		return;

	int countInfo = log->GetInfoCount();
	RECT rect = GetWindowRect();
	int left = rect.left + 16;
	int top = rect.top + 16;
	int maxHeight = rect.bottom - rect.top - 32;

	std::list<int> listHeight;
	std::list<gstd::ref_count_ptr<DxTextInfo>> listInfo;
	int heightTotal = 0;
	for (int iInfo = pos_; iInfo >= 0; iInfo--) {
		gstd::ref_count_ptr<DxTextInfo> textInfo = log->GetTextInfo(iInfo);

		int height = 0;
		int countLine = textInfo->GetLineCount();
		for (int iLine = 0; iLine < countLine; ++iLine) {
			gstd::ref_count_ptr<DxTextLine> lineText = textInfo->GetTextLine(iLine);
			height += lineText->GetHeight() + dxText_->GetLinePitch();
		}

		listHeight.push_back(height);
		listInfo.push_back(textInfo);

		heightTotal += height;
		if (heightTotal > dxText_->GetMaxHeight())
			break;
	}

	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetViewPort(left, top, rect.right - rect.left, maxHeight);
	auto itrHeight = listHeight.begin();
	auto itrInfo = listInfo.begin();
	for (; itrInfo != listInfo.end(); ++itrInfo, ++itrHeight) {
		gstd::ref_count_ptr<DxTextInfo> textInfo = (*itrInfo);
		dxText_->SetPosition(left, top);
		dxText_->SetMaxHeight(maxHeight);
		dxText_->Render(textInfo);
		top += *itrHeight;
	}
	graphics->ResetViewPort();
}
void EventLogWindow::ResetPosition()
{
	//1ページ以下のときの位置を調べる
	EventEngine* engine = _GetManager()->GetEngine();
	gstd::ref_count_ptr<EventLogText> log = engine->GetEventLogText();

	RECT rect = GetWindowRect();
	int left = rect.left + 16;
	int top = rect.top + 16;
	dxText_->SetMaxWidth(rect.right - rect.left - 32);

	int countInfo = log->GetInfoCount();
	posMin_ = 0;
	int heightTotal = 0;
	for (int iInfo = 0; iInfo < countInfo; ++iInfo) {
		gstd::ref_count_ptr<DxTextInfo> textInfo = log->GetTextInfo(iInfo);

		int height = 0;
		int countLine = textInfo->GetLineCount();
		for (int iLine = 0; iLine < countLine; ++iLine) {
			gstd::ref_count_ptr<DxTextLine> lineText = textInfo->GetTextLine(iLine);
			height += lineText->GetHeight() + dxText_->GetLinePitch();
		}

		heightTotal += height;
		if (heightTotal > dxText_->GetMaxHeight())
			break;
		posMin_++;
	}

	posMin_ = min(posMin_ - 1, countInfo - 1);
	pos_ = posMin_;
}
/**********************************************************
//EventEngine
**********************************************************/
//EventScriptObjectManager
int EventScriptObjectManager::AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj)
{
	int res = DxScript::ID_INVALID;
	if (!listUnusedIndex_.empty()) {
		for (auto itr = listUnusedIndex_.begin(); itr != listUnusedIndex_.end(); ++itr) {
			int index = (*itr);
			if (index >= INDEX_FREE_START) {
				res = index;
				listUnusedIndex_.erase(itr);
				break;
			}
		}

		obj_[res] = obj;
		obj->SetActive(true);
		listActiveObject_.push_back(obj);
		_SetObjectID(obj.GetPointer(), res);
	}
	return res;
}
void EventScriptObjectManager::Read(gstd::RecordBuffer& record)
{
	if (!record.IsExists("count"))
		return;

	//有効なID数
	int countObj = record.GetRecordAsInteger("count");
	std::vector<int> listValidId;
	listValidId.resize(countObj);
	record.GetRecord("ids", &listValidId[0], listValidId.size() * sizeof(int));

	//オブジェクト
	for (int iObj = 0; iObj < listValidId.size(); ++iObj) {
		auto* obj = new DxScriptSpriteObject2D();
		Sprite2D* sprite = obj->GetSpritePointer();

		RecordBuffer recObj;
		record.GetRecordAsRecordBuffer(StringUtility::Format("vertex%d", iObj), recObj);

		int pri = recObj.GetRecordAsDouble("pri");
		std::wstring pathTexture = recObj.GetRecordAsStringW("texture");
		pathTexture = PathProperty::GetModuleDirectory() + pathTexture;
		ref_count_ptr<Texture> texture = new Texture();
		texture->CreateFromFile(pathTexture);
		sprite->SetTexture(texture);

		int countVertex = recObj.GetRecordAsInteger("countVertex");
		D3DXVECTOR3 pos;
		D3DXVECTOR3 angle;
		D3DXVECTOR3 scale;
		sprite->SetVertexCount(countVertex);
		VERTEX_TLX* pVertex = sprite->GetVertex(0);
		recObj.GetRecord("vertex", pVertex, countVertex * sizeof(VERTEX_TLX));
		recObj.GetRecord("pos", &pos, sizeof(D3DXVECTOR3));
		recObj.GetRecord("angle", &angle, sizeof(D3DXVECTOR3));
		recObj.GetRecord("scale", &scale, sizeof(D3DXVECTOR3));
		obj->SetPosition(pos);
		obj->SetAngle(angle);
		obj->SetScale(scale);
		int index = listValidId[iObj];
		obj_[index] = obj;
	}
}
void EventScriptObjectManager::Write(gstd::RecordBuffer& record)
{
	int iObj = 0;
	std::vector<int> listValidId;
	for (iObj = 0; iObj < EventImage::INDEX_OLD_START; ++iObj) {
		if (obj_[iObj] != nullptr)
			listValidId.push_back(iObj);
	}
	if (listValidId.empty())
		return;

	//有効なID数
	record.SetRecordAsInteger("count", listValidId.size());
	record.SetRecord("ids", &listValidId[0], listValidId.size() * sizeof(int));

	//オブジェクト
	for (iObj = 0; iObj < listValidId.size(); ++iObj) {
		int index = listValidId[iObj];
		gstd::ref_count_ptr<DxScriptSpriteObject2D>::unsync obj = gstd::ref_count_ptr<DxScriptSpriteObject2D>::unsync::DownCast(obj_[index]);
		Sprite2D* sprite = obj->GetSpritePointer();

		RecordBuffer recObj;
		recObj.SetRecordAsDouble("pri", obj->GetRenderPriority());
		gstd::ref_count_ptr<Texture> texture = sprite->GetTexture();
		std::wstring pathTexture = texture->GetName();
		pathTexture = PathProperty::GetPathWithoutModuleDirectory(pathTexture);
		recObj.SetRecordAsStringW("texture", pathTexture);

		int countVertex = sprite->GetVertexCount();
		D3DXVECTOR3 pos = obj->GetPosition();
		D3DXVECTOR3 angle = obj->GetAngle();
		D3DXVECTOR3 scale = obj->GetScale();
		recObj.SetRecordAsInteger("countVertex", countVertex);
		VERTEX_TLX* pVertex = sprite->GetVertex(0);
		recObj.SetRecord("vertex", pVertex, countVertex * sizeof(VERTEX_TLX));
		recObj.SetRecord("pos", &pos, sizeof(D3DXVECTOR3));
		recObj.SetRecord("angle", &angle, sizeof(D3DXVECTOR3));
		recObj.SetRecord("scale", &scale, sizeof(D3DXVECTOR3));
		record.SetRecordAsRecordBuffer(StringUtility::Format("vertex%d", iObj), recObj);
	}
}

//EventText
bool EventText::IsVoiceText()
{
	if (text_.size() >= 2) {
		std::wstring str = text_.substr(0, 2);
		return str.find(L"「") != std::wstring::npos;
	}
	return false;
}

//EventLogText
EventLogText::EventLogText(EventEngine* engine)
{
	max_ = 100;
	engine_ = engine;
}
EventLogText::~EventLogText() = default;
void EventLogText::Add(std::string text)
{
	text += EventScriptScanner::TAG_START + EventScriptScanner::TAG_NEW_LINE + EventScriptScanner::TAG_END + "--------------------------------";
	EventLogWindow* wnd = engine_->GetWindowManager()->GetLogWindow().GetPointer();
	// RECT rect = wnd->GetWindowRect();
	ref_count_ptr<DxText> renderer = wnd->GetRenderer();

	DxText dxText;
	dxText.Copy(*renderer.GetPointer());
	/*
	dxText.SetFont(renderer->GetFont());
	// dxText.SetFontBorderType(directx::DxFont::BORDER_FULL);
	// dxText.SetFontBorderWidth(2);
	dxText.SetLinePitch(renderer->GetLinePitch());
	dxText.SetFontSize(renderer->GetFontSize());
	dxText.SetFontBold(false);
	dxText.SetPosition(renderer->GetPosition());
	dxText.SetMaxWidth(renderer->GetMaxWidth());
	*/
	dxText.SetMaxHeight(INT_MAX);

	std::wstring wText = StringUtility::ConvertMultiToWide(text);
	dxText.SetText(wText);

	gstd::ref_count_ptr<DxTextInfo> info = dxText.GetTextInfo();
	listInfo_.insert(listInfo_.begin(), info);

	if (listInfo_.size() >= max_)
		listInfo_.pop_back();
}

//EventValue
gstd::TextParser::Result EventValue::ConvertToTextParserResult()
{
	TextParser::Result res;
	if (IsReal()) {
		res.SetReal(GetReal());
	} else if (IsBoolean()) {
		res.SetBoolean(GetBoolean());
	} else if (IsString()) {
		std::wstring wstr = StringUtility::ConvertMultiToWide(GetString());
		res.SetString(wstr);
	}
	return res;
}
void EventValue::Copy(const gstd::TextParser::Result& val)
{
	if (val.IsReal()) {
		type_ = EventValue::TYPE_REAL;
		valueReal_ = val.GetReal();
	} else if (val.IsBoolean()) {
		type_ = EventValue::TYPE_BOOLEAN;
		valueBoolean_ = val.GetBoolean();
	} else if (val.IsString()) {
		type_ = EventValue::TYPE_STRING;
		std::string str = StringUtility::ConvertWideToMulti(val.GetString());
		valueString_ = str;
	}
}
void EventValue::Copy(const EventValue& val)
{
	*this = val;
}
void EventValue::Read(gstd::RecordBuffer& record)
{
	type_ = record.GetRecordAsInteger("type");
	if (type_ == TYPE_STRING) {
		std::string str = record.GetRecordAsStringA("strValue");
		std::wstring home = PathProperty::GetModuleDirectory();
		std::string sHome = StringUtility::ConvertWideToMulti(home);
		str = StringUtility::ReplaceAll(str, ".\\", sHome);
		valueString_ = str;
	} else
		valueReal_ = record.GetRecordAsDouble("value");
}
void EventValue::Write(gstd::RecordBuffer& record)
{
	record.SetRecordAsInteger("type", type_);
	if (type_ == TYPE_STRING) {
		std::string str = valueString_;
		std::wstring home = PathProperty::GetModuleDirectory();
		std::string sHome = StringUtility::ConvertWideToMulti(home);
		str = StringUtility::ReplaceAll(str, sHome, ".\\");

		record.SetRecordAsStringA("strValue", str);
	} else
		record.SetRecordAsDouble("value", valueReal_);
}

//EventFrame
EventFrame::EventFrame()
{
	posCode_ = 0;
	bEnd_ = false;
	bAutoGlobal_ = false;
}
EventFrame::~EventFrame() = default;
void EventFrame::SetBlock(gstd::ref_count_ptr<EventScriptBlock> block)
{
	block_ = block;
	posCode_ = block->GetStartPosition();
	if (block->GetReturnPosition() > 0) {
		posReturn_ = block->GetReturnPosition();
	}
}
gstd::ref_count_ptr<EventScriptCode> EventFrame::NextCode()
{
	++posCode_;
	return GetCurrentCode();
}
gstd::ref_count_ptr<EventScriptCode> EventFrame::GetCurrentCode()
{
	return sourceActive_->GetCode(posCode_);
}
bool EventFrame::HasNextCode()
{
	if (block_ != nullptr) {
		return posCode_ < block_->GetEndPosition();
	} else {
		int count = sourceActive_->GetCodeCount();
		return posCode_ < count - 1;
	}
}
gstd::ref_count_ptr<EventValue> EventFrame::GetValue(const std::string& key)
{
	auto valueItr = mapValue_.find(key);
	if (valueItr != mapValue_.end())
		return valueItr->second;
	return nullptr;
}
void EventFrame::AddValue(const std::string& key, gstd::ref_count_ptr<EventValue> val)
{
	// if(mapValue_.find(key) != mapValue_.end())
	// 	return NULL;
	mapValue_[key] = val;
}
void EventFrame::SetValue(const std::string& key, gstd::ref_count_ptr<EventValue> val)
{
	mapValue_[key] = val;
}
bool EventFrame::IsInnerBlock()
{
	if (block_ != nullptr) {
		return block_->IsInner();
	}
	return false;
}
void EventFrame::ReadRecord(gstd::RecordBuffer& record, EventEngine* engine)
{
	//位置
	posCode_ = record.GetRecordAsInteger("posCode_");
	posReturn_ = record.GetRecordAsInteger("posReturn_");

	//ソース
	std::wstring pathSource = record.GetRecordAsStringW("pathSource");
	pathSource = PathProperty::GetModuleDirectory() + pathSource;
	sourceActive_ = engine->GetSource(pathSource);

	//ブロックの位置
	if (record.IsExists("BlockIndex")) {
		//if
		int pos = record.GetRecordAsInteger("BlockIndex");
		block_ = ref_count_ptr<EventScriptBlock>::DownCast(sourceActive_->GetCode(pos));
	} else if (record.IsExists("BlockName")) {
		std::string name = record.GetRecordAsStringA("BlockName");
		block_ = sourceActive_->GetEventBlock(name);
	} else
		throw gstd::wexception(L"ブロックがない?");

	//変数
	int countValue = record.GetRecordAsInteger("countValue");
	for (int iValue = 0; iValue < countValue; ++iValue) {
		std::string keyName = StringUtility::Format("valueName%d", iValue);
		std::string keyValue = StringUtility::Format("value%d", iValue);

		std::string name = record.GetRecordAsStringA(keyName);
		RecordBuffer rcValue;
		record.GetRecordAsRecordBuffer(keyValue, rcValue);
		auto* value = new EventValue();
		value->Read(rcValue);
		mapValue_[name] = value;
	}
}
void EventFrame::WriteRecord(gstd::RecordBuffer& record, EventEngine* engine)
{
	//位置
	record.SetRecordAsInteger("posCode_", posCode_);
	record.SetRecordAsInteger("posReturn_", posReturn_);

	//ソース
	std::wstring wPathSource = engine->GetSourcePath(sourceActive_);
	wPathSource = PathProperty::GetPathWithoutModuleDirectory(wPathSource);

	record.SetRecordAsStringW("pathSource", wPathSource);

	//ブロックの位置を記録
	if (gstd::ref_count_ptr<EventScriptCode_If>::DownCast(block_) != nullptr) {
		//ifの場合はコードの位置
		int index = -1;
		int codeCount = sourceActive_->GetCodeCount();
		for (int iCode = 0; iCode < codeCount; ++iCode) {
			void* pCode = sourceActive_->GetCode(iCode).GetPointer();
			void* pBlock = block_.GetPointer();
			if (pCode == pBlock) {
				index = iCode;
				break;
			}
		}

		if (index < 0)
			throw gstd::wexception(L"ifブロックが不正で保存できません");

		record.SetRecordAsInteger("BlockIndex", index);
	} else if (gstd::ref_count_ptr<EventScriptBlock_Main>::DownCast(block_) != nullptr) {
		//その他はブロック名称
		gstd::ref_count_ptr<EventScriptBlock_Main> mainBlock = gstd::ref_count_ptr<EventScriptBlock_Main>::DownCast(block_);
		std::string name = mainBlock->GetName();
		record.SetRecordAsStringA("BlockName", name);
	} else
		throw gstd::wexception(L"保存できないブロック?");

	//変数
	int iCountValue = 0;
	int countValue = mapValue_.size();
	record.SetRecordAsInteger("countValue", countValue);
	for (auto& itrValue : mapValue_) {
		std::string name = itrValue.first;
		gstd::ref_count_ptr<EventValue> value = itrValue.second;
		record.SetRecordAsStringA(StringUtility::Format("valueName%d", iCountValue), name);

		RecordBuffer rcValue;
		value->Write(rcValue);
		record.SetRecordAsRecordBuffer(StringUtility::Format("value%d", iCountValue), rcValue);

		++iCountValue;
	}
}

//EventValueParser
EventValueParser::EventValueParser(EventEngine* engine)
{
	engine_ = engine;
}
gstd::TextParser::Result EventValueParser::_ParseIdentifer(std::vector<char>::iterator /*unused*/)
{
	Result res;
	Token& tok = scan_->GetToken();
	std::wstring id = tok.GetElement();
	if (id == L"true") {
		res.SetBoolean(true);
	} else if (id == L"false") {
		res.SetBoolean(false);
	} else if (id == L"FormatReal") {
		try {
			std::vector<std::string> args = _GetFuctionArgument();
			EventValueParser parser(engine_);
			ref_count_ptr<EventValue> val1 = parser.GetEventValue(args[0]);
			ref_count_ptr<EventValue> val2 = parser.GetEventValue(args[1]);
			std::string str = StringUtility::Format((char*)val1->GetString().c_str(), val2->GetReal());

			std::wstring wstr = StringUtility::ConvertMultiToWide(str);
			res.SetString(wstr);
		} catch (...) {
			_RaiseError(L"FormatReal");
		}
	} else if (id == L"GetHomeDirectory") {
		std::vector<std::string> args = _GetFuctionArgument();
		std::wstring dirModule = PathProperty::GetModuleDirectory();
		res.SetString(dirModule);
	} else {
		std::string sId = StringUtility::ConvertWideToMulti(id);
		ref_count_ptr<EventValue> val = engine_->GetEventValue(sId);
		if (val != nullptr) {
			res = val->ConvertToTextParserResult();
		}
	}
	return res;
}
std::vector<std::string> EventValueParser::_GetFuctionArgument()
{
	std::vector<std::string> res;
	std::string arg;
	scan_->CheckType(scan_->Next(), Token::TK_OPENP);
	while (true) {
		Token& tok = scan_->Next();
		if (tok.GetType() == Token::TK_CLOSEP) {
			if (!arg.empty())
				res.push_back(arg);
			break;
		}
		if (tok.GetType() == Token::TK_COMMA) {
			res.push_back(arg);
			arg = "";
		} else {
			std::wstring wstr = tok.GetElement();
			std::string str = StringUtility::ConvertWideToMulti(wstr);
			arg += str;
		}
	}
	return res;
}
gstd::ref_count_ptr<EventValue> EventValueParser::GetEventValue(const std::string& text)
{
	SetSource(text);
	gstd::ref_count_ptr<EventValue> res = new EventValue();
	if (text.empty())
		return res;

	TextParser::Result value = GetResult();
	res->Copy(value);
	return res;
}

//EventImage
EventImage::EventImage()
{
	objManager_.resize(2);
	for (auto& iManager : objManager_) {
		auto* manager = new EventScriptObjectManager();
		manager->SetMaxObject(MAX_OBJECT);
		iManager = manager;
	}
	indexForeground_ = 0;
}
EventImage::~EventImage() = default;
void EventImage::Render(int layer)
{
	objManager_[layer]->RenderObject();
}
int EventImage::GetForegroundLayerIndex() const
{
	return indexForeground_;
}
int EventImage::GetBackgroundLayerIndex() const
{
	return 1 - indexForeground_;
}
void EventImage::SwapForeBackLayerIndex()
{
	indexForeground_ = 1 - indexForeground_;
}
void EventImage::Read(gstd::RecordBuffer& record)
{
	//前景インデックス
	indexForeground_ = record.GetRecordAsInteger("fore");

	//オブジェクト管理
	for (int iManager = 0; iManager < objManager_.size(); ++iManager) {
		RecordBuffer recManager;
		record.GetRecordAsRecordBuffer(StringUtility::Format("manager%d", iManager), recManager);
		objManager_[iManager]->Read(recManager);
	}
}
void EventImage::Write(gstd::RecordBuffer& record)
{
	//前景インデックス
	record.SetRecordAsInteger("fore", indexForeground_);

	//オブジェクト管理
	for (int iManager = 0; iManager < objManager_.size(); ++iManager) {
		RecordBuffer recManager;
		objManager_[iManager]->Write(recManager);

		record.SetRecordAsRecordBuffer(StringUtility::Format("manager%d", iManager), recManager);
	}
}

//EventKeyState
EventKeyState::EventKeyState(EventEngine* engine)
{
	engine_ = engine;
	bNextEnable_ = true;
}
EventKeyState::~EventKeyState() = default;
void EventKeyState::Work()
{
	bNextEnable_ = true;
}
bool EventKeyState::IsNext() const
{
	if (!bNextEnable_)
		return false;

	const DirectInput* input = DirectInput::GetBase();

	auto mngWindow = engine_->GetWindowManager();
	auto wndEvent = mngWindow->GetMouseCaptureLayer()->GetEvent();

	return (wndEvent != nullptr && wndEvent->HasEventType(DxWindowEvent::TYPE_MOUSE_LEFT_CLICK))
		|| (input->GetKeyState(DIK_Z) == KEY_PUSH)
		|| (input->GetKeyState(DIK_RETURN) == KEY_PUSH)
		|| (input->GetMouseMoveZ() < 0);
}
bool EventKeyState::IsSkip() const
{
	const DirectInput* input = DirectInput::GetBase();
	return (input->GetKeyState(DIK_LCONTROL) == KEY_HOLD);
}

//EventSound
EventSound::EventSound() = default;
EventSound::~EventSound()
{
	if (playerBgm_ != nullptr)
		playerBgm_->Delete();
	if (playerSe_ != nullptr)
		playerSe_->Delete();
}
void EventSound::Play(int type, const std::string& path)
{
	DirectSoundManager* manager = DirectSoundManager::GetBase();
	gstd::ref_count_ptr<SoundPlayer> player = type == TYPE_BGM ? playerBgm_ : playerSe_;
	if (player != nullptr) {
		if (type == TYPE_BGM) {
			player->SetFadeDelete(-10);
			playerBgm_ = nullptr;
		} else {
			player->Stop();
			player->Delete();
			playerSe_ = nullptr;
		}
	}

	std::wstring wPath = StringUtility::ConvertMultiToWide(path);
	player = manager->GetPlayer(wPath);
	if (player != nullptr) {
		bool bLoop = type == TYPE_BGM;
		SoundPlayer::PlayStyle style;
		style.SetLoopEnable(bLoop);
		player->Play(style);
		if (type == TYPE_BGM)
			playerBgm_ = player;
		else
			playerSe_ = player;
	}
}
void EventSound::Delete(int type)
{
	gstd::ref_count_ptr<SoundPlayer> player = (type == TYPE_BGM ? playerBgm_ : playerSe_);
	if (player != nullptr) {
		if (type == TYPE_BGM) {
			player->SetFadeDelete(-20);
			playerBgm_ = nullptr;
		} else {
			player->Stop();
			player->Delete();
			playerSe_ = nullptr;
		}
	}
}
void EventSound::Read(gstd::RecordBuffer& record)
{
	if (!record.IsExists("path"))
		return;
	std::string dirModule = StringUtility::ConvertWideToMulti(PathProperty::GetModuleDirectory());
	std::string path = record.GetRecordAsStringA("path");
	path = dirModule + path;
	Play(TYPE_BGM, path);
}
void EventSound::Write(gstd::RecordBuffer& record)
{
	if (playerBgm_ == nullptr)
		return;
	std::wstring path = playerBgm_->GetPath();
	path = PathProperty::GetPathWithoutModuleDirectory(path);

	std::string sPath = StringUtility::ConvertWideToMulti(path);
	record.SetRecordAsStringA("path", sPath);
}

//EventEngine
EventEngine::EventEngine()
{
	state_ = STATE_RUN;
}
EventEngine::~EventEngine()
{
	FileManager::GetBase()->WaitForThreadLoadComplete();

	listScript_.clear();
	windowManager_ = nullptr;
	textEvent_ = nullptr;
	image_ = nullptr;
	keyState_ = nullptr;
	frameGlobal_ = nullptr;
}
bool EventEngine::Initialize()
{
	windowManager_ = new EventWindowManager(this);
	windowManager_->Initialize();
	textEvent_ = new EventText();
	logEvent_ = new EventLogText(this);
	image_ = new EventImage();
	keyState_ = new EventKeyState(this);
	sound_ = new EventSound();
	bCriticalFrame_ = false;
	frameGlobal_ = new EventFrame();
	cacheScriptEngine_ = new ScriptEngineCache();
	return true;
}
void EventEngine::_RaiseError(std::wstring msg)
{
	ref_count_ptr<EventFrame> frame = *frame_.rbegin();
	if (frame != nullptr) {
		gstd::ref_count_ptr<EventScriptCode> code = frame->GetCurrentCode();
		std::wstring path = GetSourcePath(frame->GetActiveSource());
		if (code != nullptr) {
			msg += StringUtility::Format(L"line[%d] path[%s]", code->GetLine(), path.c_str());
		}
	}
	throw gstd::wexception(msg);
}
void EventEngine::_RunCode()
{
	if (frame_.empty())
		return;

	bCriticalFrame_ = false;
	_WorkWindow();
	keyState_->Work();

	while (true) {
		if (activeCodeExecuter_ != nullptr) {
			activeCodeExecuter_->Execute();
			for (auto itr = parallelCodeExecuter_.begin(); itr != parallelCodeExecuter_.end();) {
				(*itr)->Execute();
				if ((*itr)->IsEnd()) {
					itr = parallelCodeExecuter_.erase(itr);
				} else
					++itr;
			}

			bool bEndCode = activeCodeExecuter_->IsEnd() && parallelCodeExecuter_.empty();
			if (bEndCode) {
				activeCodeExecuter_ = nullptr;
			} else {
				break;
			}
		} else if (textEvent_->HasNext()) {
			//表示中テキストが残っている場合
			bool bNext = keyState_->IsNext();
			bool bSkip = keyState_->IsSkip();
			if (!bNext && !bSkip) {
				textEvent_->Next();
			} else {
				textEvent_->NextSkip();
			}
			break;
		} else {
			keyState_->SetNextEnable(false);
			windowManager_->GetMouseCaptureLayer()->ClearEvent();

			//テキストが残っていない場合、次のコードを読み込む
			ref_count_ptr<EventFrame> frameActive = *frame_.rbegin();
			while (frameActive->IsEnd()) {
				//フレーム終了
				int next = frameActive->GetReturnPosition();
				frame_.pop_back();
				if (frame_.empty())
					return;
				// if(frameActive->GetBlock()->IsGlobal())
				// 	return;
				if (frameActive->IsAutoGlobal())
					return; //自動グローバル処理の場合は抜ける
				frameActive = *frame_.rbegin();
				if (next == EventScriptBlock::POS_NULL) {
					//返る位置が指定されて無い場合、
					//その次のコードから継続する
					next = frameActive->GetCurrentPosition() + 1;
				}
				frameActive->SetCurrentPosition(next);
				if (!frameActive->HasNextCode())
					frameActive->SetEnd();
			}

			gstd::ref_count_ptr<EventScriptCode> code = frameActive->GetCurrentCode();
			int typeCode = code->GetType();
			if (typeCode == EventScriptCode::TYPE_TEXT) {
				auto* codeText = (EventScriptCode_Text*)code.GetPointer();

				std::wstring wText = StringUtility::ConvertMultiToWide(codeText->GetText());
				textEvent_->SetSource(wText);
			} else if (typeCode == EventScriptCode::TYPE_WAIT_CLICK) {
				activeCodeExecuter_ = new EventScriptCodeExecuter_WaitClick(this);
			} else if (typeCode == EventScriptCode::TYPE_WAIT_NEXT_PAGE) {
				activeCodeExecuter_ = new EventScriptCodeExecuter_WaitNextPage(this);
			} else if (typeCode == EventScriptCode::TYPE_WAIT_TIME) {
				auto* codeWaitTime = (EventScriptCode_WaitTime*)code.GetPointer();
				activeCodeExecuter_ = new EventScriptCodeExecuter_WaitTime(this, codeWaitTime);
			} else if (typeCode == EventScriptCode::TYPE_CLEAR_MESSAGE) {
				std::string text = StringUtility::ConvertWideToMulti(textEvent_->GetText());
				logEvent_->Add(text);
				textEvent_->Clear();
			} else if (typeCode == EventScriptCode::TYPE_NAME) {
				auto* codeName = (EventScriptCode_Name*)code.GetPointer();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeName->GetName());
				std::string name = val->GetString();
				std::wstring wName = StringUtility::ConvertMultiToWide(name);

				gstd::ref_count_ptr<EventNameWindow> wndName = windowManager_->GetNameWindow();
				if (wndName != nullptr)
					wndName->SetText(wName);
			} else if (typeCode == EventScriptCode::TYPE_TRANSITION) {
				auto* codeTrans = (EventScriptCode_Transition*)code.GetPointer();
				auto* executer = new EventScriptCodeExecuter_Transition(this, codeTrans);
				activeCodeExecuter_ = executer;
			} else if (typeCode == EventScriptCode::TYPE_VISIBLE_TEXT) {
				auto* codeVisibleText = (EventScriptCode_VisibleText*)code.GetPointer();
				bool bVisible = codeVisibleText->IsVisible();
				windowManager_->SetTextVisible(bVisible);
			} else if (typeCode == EventScriptCode::TYPE_VAR) {
				auto* codeVar = (EventScriptCode_Var*)code.GetPointer();
				std::string& name = codeVar->GetName();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeVar->GetValue());
				if (frameActive->GetBlock()->IsGlobal()) {
					frameGlobal_->AddValue(name, val);
				} else {
					frameActive->AddValue(name, val);
				}
			} else if (typeCode == EventScriptCode::TYPE_EVAL) {
				auto* codeEval = (EventScriptCode_Eval*)code.GetPointer();
				std::string& name = codeEval->GetName();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeEval->GetValue());
				ref_count_ptr<EventValue> dest = frameActive->GetValue(name);
				dest->Copy(*val.GetPointer());
				if ((*frame_.begin())->GetBlock()->IsGlobal()) {
					frameGlobal_->SetValue(name, val);
				} else {
					frameActive->SetValue(name, dest);
				}
			} else if (typeCode == EventScriptCode::TYPE_SYSVAL) {
				auto* codeSysVal = (EventScriptCode_SysVal*)code.GetPointer();
				std::string& name = codeSysVal->GetName();
				bool bGlobal = codeSysVal->IsGlobal();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeSysVal->GetValue());

				std::string syskey = bGlobal ? SystemValueManager::RECORD_SYSTEM_GLOBAL : SystemValueManager::RECORD_SYSTEM;
				SystemValueManager* svm = SystemValueManager::GetBase();
				ref_count_ptr<RecordBuffer> record = svm->GetRecordBuffer(syskey);

				int typeVal = val->GetType();
				switch (typeVal) {
				case EventValue::TYPE_BOOLEAN:
					record->SetRecordAsBoolean(name, val->GetBoolean());
					break;
				case EventValue::TYPE_REAL:
					record->SetRecordAsDouble(name, val->GetReal());
					break;
				case EventValue::TYPE_STRING:
					record->SetRecordAsStringA(name, val->GetString());
					break;
				}

			} else if (typeCode == EventScriptCode::TYPE_OUTPUT) {
				auto* codeOut = (EventScriptCode_Output*)code.GetPointer();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeOut->GetValue());

				std::wstring wText = StringUtility::ConvertMultiToWide(val->GetString());
				textEvent_->SetSource(wText);
			} else if (typeCode == EventScriptCode::TYPE_IMAGE) {
				auto* codeImage = (EventScriptCode_Image*)code.GetPointer();
				auto* executer = new EventScriptCodeExecuter_Image(this, codeImage);

				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeImage->GetWaitEnd());
				bool bWaitEnd = val->GetBoolean();
				if (bWaitEnd)
					activeCodeExecuter_ = executer;
				else
					parallelCodeExecuter_.emplace_back(executer);
			} else if (typeCode == EventScriptCode::TYPE_SOUND) {
				auto* codeSound = (EventScriptCode_Sound*)code.GetPointer();
				EventValueParser parser(this);
				ref_count_ptr<EventValue> val = parser.GetEventValue(codeSound->GetPath());
				std::string path = val->GetString();
				if (!path.empty()) {
					sound_->Play(codeSound->GetSoundType(), path);
				} else {
					sound_->Delete(codeSound->GetSoundType());
				}
			} else if (typeCode == EventScriptCode::TYPE_IF) {
				while (true) {
					ref_count_ptr<EventScriptCode_If> codeIf = ref_count_ptr<EventScriptCode_If>::DownCast(code);
					std::string param = codeIf->GetParameter();
					EventValueParser parser(this);
					ref_count_ptr<EventValue> val = parser.GetEventValue(param);
					if (val->GetBoolean()) {
						ref_count_ptr<EventFrame> pFrame = new EventFrame();
						pFrame->SetActiveSource(frameActive->GetActiveSource());

						ref_count_ptr<EventScriptBlock> block = codeIf;
						pFrame->SetBlock(block);

						frame_.push_back(pFrame);
						break;
					}

					int posNext = codeIf->GetNextElsePosition();
					if (posNext > 0) {
						frameActive->SetCurrentPosition(posNext);
						code = frameActive->GetCurrentCode();
					} else {
						//if該当なし
						frameActive->SetCurrentPosition(codeIf->GetEndPosition());
						break;
					}
				}
				continue;
			} else if (typeCode == EventScriptCode::TYPE_JUMP) {
				ref_count_ptr<EventScriptCode_Jump> codeJump = ref_count_ptr<EventScriptCode_Jump>::DownCast(code);
				EventValueParser parser(this);
				std::string path = parser.GetEventValue(codeJump->GetPath())->GetString();
				std::string name = parser.GetEventValue(codeJump->GetName())->GetString();

				ref_count_ptr<EventScriptBlock> block;
				ref_count_ptr<EventFrame> frameJump = new EventFrame();
				if (path.empty()) {
					//自スクリプト
					block = frameActive->GetActiveSource()->GetEventBlock(name);
					frameJump->SetActiveSource(frameActive->GetActiveSource());
				} else {
					//別ファイルスクリプト
					std::wstring wPath = StringUtility::ConvertMultiToWide(path);
					gstd::ref_count_ptr<EventScriptSource> source = _GetSource(wPath);
					frameJump->SetActiveSource(source);
					block = source->GetEventBlock(name);
				}

				if (block != nullptr) {
					if (codeJump->IsGoSub()) {
						int current = frameActive->GetCurrentPosition() + 1;
						frameJump->SetBlock(block);
						frame_.push_back(frameJump);
						frameJump->SetReturnPosition(current);
					} else {
						int current = frameActive->GetCurrentPosition() + 1;
						frameJump->SetBlock(block);
						frame_.clear();
						frame_.push_back(frameJump);
						frameJump->SetReturnPosition(current);
					}
				} else {
					_RaiseError(L"存在しないブロックが指定されました");
				}

				continue;
			} else if (typeCode == EventScriptCode::TYPE_SCRIPT) {
				ref_count_ptr<EventScriptCode_Script> codeScript = ref_count_ptr<EventScriptCode_Script>::DownCast(code);
				EventValueParser parser(this);
				std::string path = parser.GetEventValue(codeScript->GetPath())->GetString();
				std::string method = parser.GetEventValue(codeScript->GetMethod())->GetString();
				std::string code = parser.GetEventValue(codeScript->GetCode())->GetString();
				code = StringUtility::ReplaceAll(code, "\\\"", "\"");
				std::string scriptId = parser.GetEventValue(codeScript->GetId())->GetString();
				bool bWait = parser.GetEventValue(codeScript->GetWaitEnd())->GetBoolean();
				int target = parser.GetEventValue(codeScript->GetTargetId())->GetReal();
				std::vector<std::string> listArg = codeScript->GetArgumentList();

				if (method.empty())
					_RaiseError(L"methodのないスクリプトを実行しようとしました");

				std::wstring wPath = StringUtility::ConvertMultiToWide(path);
				auto* script = new DxScriptForEvent(this);
				script->SetTargetId(target);
				script->SetMethod(method);
				if (!path.empty())
					script->SetSourceFromFile(wPath);
				else if (!code.empty()) {
					script->SetSource(code);

					std::wstring path = GetSourcePath(frameActive->GetActiveSource());
					script->SetPath(wPath);
				}

				script->SetScriptId(scriptId);
				script->Compile();
				if (!listArg.empty()) {
					int countArg = listArg.size();
					for (int iArg = 0; iArg < countArg; iArg++) {
						ref_count_ptr<EventValue> eArg = parser.GetEventValue(listArg[iArg]);
						script->AddArgumentValue(eArg);
					}
				}
				script->Run(method);

				if (codeScript->IsEndScript()) {
					std::list<gstd::ref_count_ptr<DxScriptForEvent>>::iterator itr;
					for (itr = listScript_.begin(); itr != listScript_.end(); itr++) {
						gstd::ref_count_ptr<DxScriptForEvent> script = (*itr);
						if (scriptId == script->GetScriptId() || (wPath == script->GetPath() && method == script->GetMethod())) {
							script->EndScript();
						}
					}
				} else {
					if (bWait) {
						auto* executer = new EventScriptCodeExecuter_Script(this, script);
						activeCodeExecuter_ = executer;
					}
					listScript_.emplace_back(script);
				}
			} else {
				//不明なタグや、テキストレンダラにそのままわたすタグ
				int res = _RunCode(frameActive, code);
				if (res == RUN_RETURN_NONE) {
					std::wstring wText = StringUtility::ConvertMultiToWide(code->GetCodeText());
					textEvent_->SetSource(wText);
				}
			}

			if (frameActive->HasNextCode())
				frameActive->NextCode();
			else
				frameActive->SetEnd();
			bCriticalFrame_ = true;
		}
	}

	//スキップ中の場合は、強制更新不要
	if (keyState_->IsSkip())
		bCriticalFrame_ = false;
}
void EventEngine::_RunScript()
{
	for (auto itrScript = listScript_.begin(); itrScript != listScript_.end();) {
		gstd::ref_count_ptr<DxScriptForEvent>& script = (*itrScript);
		if (script->IsScriptEnd()) {
			script->Clear();
			itrScript = listScript_.erase(itrScript);
		} else {
			script->Run("MainLoop");
			++itrScript;
		}
	}
}
void EventEngine::_WorkWindow()
{
	windowManager_->Work();
}
gstd::ref_count_ptr<EventScriptSource> EventEngine::_GetSource(const std::wstring& path)
{
	gstd::ref_count_ptr<EventScriptSource> res = nullptr;
	auto sourceItr = mapSource_.find(path);
	if (sourceItr != mapSource_.end()) {
		res = sourceItr->second;
	} else {
		EventScriptCompiler compiler;
		compiler.SetPath(path);
		res = compiler.Compile();

		if (res == nullptr) {
			throw gstd::wexception(L"コンパイル失敗");
		}

		ref_count_ptr<EventScriptBlock> block = res->GetEventBlock(EventScriptBlock::BLOCK_GLOBAL);
		if (block != NULL) {
			frameGlobal_->SetActiveSource(res);
			frameGlobal_->SetBlock(block);

			ref_count_ptr<EventFrame> frame = new EventFrame();
			frame->SetAutoGlobal(true);
			frame->SetActiveSource(res);
			frame->SetBlock(block);
			frame_.push_back(frame);
			_RunCode();
		}

		mapSource_[path] = res;
	}
	return res;
}

void EventEngine::Work()
{
	try {
		//状態変更
		CheckStateChenge();

		//実行
		if (state_ == STATE_RUN) {
			_RunCode();
			_RunScript();
		} else if (state_ == STATE_LOG) {
			_WorkWindow();
		} else if (state_ == STATE_HIDE_TEXT) {
			_WorkWindow();
			_RunScript();
		}
	} catch (gstd::wexception& e) {
		_RaiseError(e.what());
	}
}
void EventEngine::Render()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<DxCamera2D> camera = graphics->GetCamera2D();

	camera->SetEnable(true);
	image_->Render(image_->GetForegroundLayerIndex());
	camera->SetEnable(false);

	windowManager_->Render();
}

void EventEngine::SetSource(const std::wstring& path)
{
	ref_count_ptr<EventScriptSource> source = _GetSource(path);
	ref_count_ptr<EventFrame> frame = new EventFrame();
	frame->SetActiveSource(source);

	ref_count_ptr<EventScriptBlock> block = source->GetEventBlock("main");
	frame->SetBlock(block);

	frame_.push_back(frame);
}
gstd::ref_count_ptr<EventScriptSource> EventEngine::GetSource(const std::wstring& path)
{
	auto sourceItr = mapSource_.find(path);
	if (sourceItr != mapSource_.end())
		return sourceItr->second;
	return nullptr;
}
std::wstring EventEngine::GetSourcePath(gstd::ref_count_ptr<EventScriptSource> source)
{
	for (auto itrSource = mapSource_.begin(); itrSource != mapSource_.end(); ++itrSource) {
		auto tSource = (*itrSource).second;
		if (source == tSource) {
			return (*itrSource).first;
		}
	}
	return std::wstring();
}
bool EventEngine::IsEnd()
{
	return frame_.empty();
}
gstd::ref_count_ptr<EventValue> EventEngine::GetEventValue(std::string key)
{
	gstd::ref_count_ptr<EventValue> res = nullptr;
	bool bInner = true;
	int count = frame_.size();
	do {
		ref_count_ptr<EventFrame> frame = frame_[--count];
		res = frame->GetValue(key);
		bInner = frame->IsInnerBlock();
	} while (count > 0 && res == nullptr && bInner);

	if (res == nullptr) {
		res = frameGlobal_->GetValue(key);
	}

	if (res == nullptr) {
		SystemValueManager* svm = SystemValueManager::GetBase();
		ref_count_ptr<RecordBuffer> record = svm->GetRecordBuffer(SystemValueManager::RECORD_SYSTEM);
		if (record == nullptr)
			record = svm->GetRecordBuffer(SystemValueManager::RECORD_SYSTEM_GLOBAL);
		if (record != nullptr) {
			if (record->IsExists(key)) {
				res = new EventValue();
				int type = record->GetEntryType(key);
				switch (type) {
				case RecordEntry::TYPE_BOOLEAN:
					res->SetBoolean(record->GetRecordAsBoolean(key));
					break;
				case RecordEntry::TYPE_DOUBLE:
					res->SetReal(record->GetRecordAsDouble(key));
					break;
				case RecordEntry::TYPE_STRING_A:
					res->SetString(record->GetRecordAsStringA(key));
					break;
				}
			}
		}
	}

	return res;
}
void EventEngine::SetState(int state)
{
	if (state_ == state)
		return;

	EventWindowManager* windowManager = GetWindowManager().GetPointer();
	EventTextWindow* wndText = windowManager->GetTextWindow().GetPointer();
	EventNameWindow* wndName = windowManager->GetNameWindow().GetPointer();
	EventLogWindow* wndLog = windowManager->GetLogWindow().GetPointer();
	bool bTextVisible = windowManager->IsTextVisible();
	if (!bTextVisible)
		state = STATE_RUN;

	state_ = state;
	if (state == STATE_LOG) {
		wndText->SetWindowVisible(false);
		wndName->SetWindowVisible(false);
		wndLog->ResetPosition();
		wndLog->SetWindowEnable(true);
		wndLog->SetWindowVisible(true);
	} else if (state == STATE_RUN) {
		wndText->SetWindowVisible(true);
		wndName->SetWindowVisible(true);
		wndLog->SetWindowEnable(false);
		wndLog->SetWindowVisible(false);
	} else if (state == STATE_HIDE_TEXT) {
		wndText->SetWindowVisible(false);
		wndName->SetWindowVisible(false);
		wndLog->SetWindowEnable(false);
		wndLog->SetWindowVisible(false);
	}

	DxButton* btnSave = windowManager->GetSaveButton().GetPointer();
	DxButton* btnLoad = windowManager->GetLoadButton().GetPointer();
	if (btnSave != nullptr) {
		btnSave->SetWindowVisible(state == STATE_RUN);
		btnSave->SetWindowEnable(state == STATE_RUN);
	}
	if (btnLoad != nullptr) {
		btnLoad->SetWindowVisible(state == STATE_RUN);
		btnLoad->SetWindowEnable(state == STATE_RUN);
	}

	DirectInput* input = DirectInput::GetBase();
	input->ResetMouseState();
}
void EventEngine::CheckStateChenge()
{
	DirectInput* input = DirectInput::GetBase();
	if (state_ == STATE_RUN) {
		bool bChangeLOG = input->GetMouseMoveZ() > 0;
		bool bChangeHideText = input->GetMouseState(DI_MOUSE_RIGHT) == KEY_PULL;

		if (bChangeLOG)
			SetState(STATE_LOG);
		else if (bChangeHideText)
			SetState(STATE_HIDE_TEXT);
	} else if (state_ == STATE_HIDE_TEXT) {
		if (keyState_->IsSkip()
			|| (input->GetMouseState(DI_MOUSE_LEFT) == KEY_PULL)
			|| (input->GetMouseState(DI_MOUSE_RIGHT) == KEY_PULL)
		) {
			SetState(STATE_RUN);
		}
	}
}

bool EventEngine::IsSaveEnable()
{
	if (activeCodeExecuter_ == nullptr)
		return false;
	return (ref_count_ptr<EventScriptCodeExecuter_WaitClick>::DownCast(activeCodeExecuter_) != nullptr);
}
bool EventEngine::Load(const std::wstring& path)
{
	RecordBuffer record;
	record.ReadFromFile(path);

	return Load(record);
}
bool EventEngine::Load(gstd::RecordBuffer& record)
{
	Read(record);

	//とりあえずクリック待ち
	textEvent_->NextSkip();
	activeCodeExecuter_ = new EventScriptCodeExecuter_WaitClick(this);

	SetState(STATE_RUN);

	return true;
}
bool EventEngine::Save(const std::wstring& path)
{
	RecordBuffer record;
	Write(record);
	record.WriteToFile(path);
	return true;
}
void EventEngine::Read(gstd::RecordBuffer& record)
{
	//テキストウィンドウ
	std::wstring textDisp = record.GetRecordAsStringW("DispText");
	textEvent_->SetSource(textDisp);

	//スクリプトパス
	int countSource = record.GetRecordAsInteger("SourceCount");
	for (int iSource = 0; iSource < countSource; ++iSource) {
		std::wstring path = record.GetRecordAsStringW(StringUtility::Format("SourcePath%d", iSource));
		path = PathProperty::GetModuleDirectory() + path;

		EventScriptCompiler compiler;
		compiler.SetPath(path);
		gstd::ref_count_ptr<EventScriptSource> source = compiler.Compile();

		mapSource_[path] = source;
	}

	//グローバル変数フレーム
	gstd::RecordBuffer recGlobalFrame;
	record.GetRecordAsRecordBuffer("GlobalFrame", recGlobalFrame);
	frameGlobal_->ReadRecord(recGlobalFrame, this);

	//フレーム
	int countFrame = record.GetRecordAsInteger("FrameCount");
	frame_.resize(countFrame);
	for (int iFrame = 0; iFrame < countFrame; ++iFrame) {
		gstd::RecordBuffer recFrame;
		record.GetRecordAsRecordBuffer(StringUtility::Format("Frame%d", iFrame), recFrame);
		auto* frame = new EventFrame();
		frame->ReadRecord(recFrame, this);

		frame_[iFrame] = frame;
	}

	//表示中画像
	RecordBuffer recImage;
	record.GetRecordAsRecordBuffer("image", recImage);
	image_->Read(recImage);

	//ウィンドウ
	RecordBuffer recWindow;
	record.GetRecordAsRecordBuffer("window", recWindow);
	windowManager_->Read(recWindow);

	//音声
	RecordBuffer recSound;
	record.GetRecordAsRecordBuffer("sound", recSound);
	sound_->Read(recSound);

	//実行中スクリプト
	int countScript = record.GetRecordAsInteger("ScriptCount");
	for (int iScript = 0; iScript < countScript; ++iScript) {
		RecordBuffer recScript;
		record.GetRecordAsRecordBuffer(StringUtility::Format("Script%d", iScript), recScript);

		auto* script = new DxScriptForEvent(this);
		script->Read(recScript);
		script->Compile();
		script->Run(script->GetMethod());

		listScript_.emplace_back(script);
	}
}
void EventEngine::Write(gstd::RecordBuffer& record)
{
	//テキストウィンドウ
	std::wstring wTextDisp = textEvent_->GetText();
	record.SetRecordAsStringW("DispText", wTextDisp);

	//スクリプトパス
	int iSource = 0;
	record.SetRecordAsInteger("SourceCount", mapSource_.size());
	for (auto itrSource = mapSource_.begin(); itrSource != mapSource_.end(); ++itrSource) {
		std::wstring path = (*itrSource).first;
		path = PathProperty::GetPathWithoutModuleDirectory(path);

		record.SetRecordAsStringW(StringUtility::Format("SourcePath%d", iSource), path);
		++iSource;
	}

	//グローバル変数フレーム
	gstd::RecordBuffer recGlobalFrame;
	frameGlobal_->WriteRecord(recGlobalFrame, this);
	record.SetRecordAsRecordBuffer("GlobalFrame", recGlobalFrame);

	//フレーム
	int countFrame = frame_.size();
	record.SetRecordAsInteger("FrameCount", countFrame);
	for (int iFrame = 0; iFrame < countFrame; ++iFrame) {
		gstd::RecordBuffer recFrame;
		frame_[iFrame]->WriteRecord(recFrame, this);
		record.SetRecordAsRecordBuffer(StringUtility::Format("Frame%d", iFrame), recFrame);
	}

	//表示中画像
	RecordBuffer recImage;
	image_->Write(recImage);
	record.SetRecordAsRecordBuffer("image", recImage);

	//ウィンドウ
	RecordBuffer recWindow;
	windowManager_->Write(recWindow);
	record.SetRecordAsRecordBuffer("window", recWindow);

	//音声
	RecordBuffer recSound;
	sound_->Write(recSound);
	record.SetRecordAsRecordBuffer("sound", recSound);

	//実行中スクリプト
	int countScript = 0;
	for (auto& script : listScript_) {
		if (script == nullptr)
			continue;
		if (script->IsScriptEnd())
			continue;

		RecordBuffer recScript;
		script->Write(recScript);
		record.SetRecordAsRecordBuffer(StringUtility::Format("Script%d", countScript), recScript);

		++countScript;
	}
	record.SetRecordAsInteger("ScriptCount", countScript);
}
/**********************************************************
//DxScriptForEvent
**********************************************************/
function const eventFunction[] = {
	//関数：スクリプト操作
	{ "EndScript", DxScriptForEvent::Func_EndScript, 0 },
	{ "GetTarget", DxScriptForEvent::Func_GetTarget, 0 },
	{ "GetEventValue", DxScriptForEvent::Func_GetEventValue, 1 },
	{ "SetEventValue", DxScriptForEvent::Func_SetEventValue, 2 },

	//関数：キー入力
	{ "IsSkip", DxScriptForEvent::Func_IsSkip, 0 },

	//定数
	{ "ID_TRANSITION", constant<EventImage::ID_TRANSITION>::func, 0 },
};
DxScriptForEvent::DxScriptForEvent(EventEngine* engine)
{
	_AddFunction(eventFunction, sizeof(eventFunction) / sizeof(function));
	bScriptEnd_ = false;
	targetId_ = ID_INVALID;
	engine_ = engine;

	gstd::ref_count_ptr<EventImage> image = engine->GetEventImage();
	int layer = image->GetForegroundLayerIndex();
	SetObjectManager(image->GetObjectManager(layer));
}
DxScriptForEvent::~DxScriptForEvent()
{
	Clear();
}
std::vector<char> DxScriptForEvent::_Include(std::vector<char>& source)
{
	std::vector<char> res = ScriptClientBase::_Include(source);
	std::string strMain = "@MainLoop{yield;}";

	int posResEnd = res.size();
	res.resize(res.size() + strMain.size());
	memcpy(&res[posResEnd], &strMain[0], strMain.size());

	return res;
}
void DxScriptForEvent::Clear()
{
	for (int objectId : listObj_) {
		objManager_->DeleteObject(objectId);
	}
}
bool DxScriptForEvent::SetSourceFromFile(const std::wstring& path)
{
	SetScriptEngineCache(engine_->GetScriptEngineCache());
	return ScriptClientBase::SetSourceFromFile(path);
}
void DxScriptForEvent::SetSource(const std::string& source)
{
	ScriptClientBase::SetSource(source);
	code_ = source;
}
int DxScriptForEvent::AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj)
{
	int res = DxScript::AddObject(obj);
	if (res != ID_INVALID)
		listObj_.insert(res);
	return res;
}
void DxScriptForEvent::DeleteObject(int id)
{
	if (id == ID_INVALID)
		return;
	DxScript::DeleteObject(id);
	auto objectItr = listObj_.find(id);
	if (objectItr != listObj_.end())
		listObj_.erase(objectItr);
}
void DxScriptForEvent::AddArgumentValue(gstd::ref_count_ptr<EventValue> arg)
{
	int type = arg->GetType();
	gstd::value vArg;
	if (type == EventValue::TYPE_REAL)
		vArg = value(machine_->get_engine()->get_real_type(), (long double)arg->GetReal());
	else if (type == EventValue::TYPE_BOOLEAN)
		vArg = value(machine_->get_engine()->get_boolean_type(), arg->GetBoolean());
	else if (type == EventValue::TYPE_STRING)
		vArg = value(machine_->get_engine()->get_string_type(), to_wide(arg->GetString()));
	ScriptClientBase::AddArgumentValue(vArg);
}
void DxScriptForEvent::Read(gstd::RecordBuffer& record)
{
	//ファイルパス
	std::wstring path = record.GetRecordAsStringW("path");
	path = PathProperty::GetModuleDirectory() + path;

	//実行メソッド
	method_ = record.GetRecordAsStringA("method");

	//ターゲット
	targetId_ = record.GetRecordAsInteger("target");

	//コード
	code_ = record.GetRecordAsStringA("code");

	if (!code_.empty()) {
		SetSource(code_);
		SetPath(path);
	} else {
		SetSourceFromFile(path);
	}
}
void DxScriptForEvent::Write(gstd::RecordBuffer& record)
{
	//ファイルパス
	std::wstring path = GetPath();
	path = PathProperty::GetPathWithoutModuleDirectory(path);
	record.SetRecordAsStringW("path", path);

	//実行メソッド
	record.SetRecordAsStringA("method", method_);

	//ターゲット
	record.SetRecordAsInteger("target", targetId_);

	//コード
	record.SetRecordAsStringA("code", code_);
}

//関数：スクリプト操作
gstd::value DxScriptForEvent::Func_EndScript(script_machine* machine, int argc, value const* argv)
{
	auto* script = (DxScriptForEvent*)machine->data;
	script->bScriptEnd_ = true;
	return gstd::value();
}
gstd::value DxScriptForEvent::Func_GetTarget(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (DxScriptForEvent*)machine->data;
	return value(machine->get_engine()->get_real_type(), (long double)script->targetId_);
}
gstd::value DxScriptForEvent::Func_GetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (DxScriptForEvent*)machine->data;
	std::wstring wName = argv[0].as_string();
	std::string name = to_mbcs(wName);
	gstd::ref_count_ptr<EventValue> eValue = script->engine_->GetEventValue(name);
	if (eValue == nullptr)
		throw gstd::wexception(StringUtility::Format(L"存在しない変数:%s", name.c_str()));

	int type = eValue->GetType();
	if (type == EventValue::TYPE_REAL)
		return value(machine->get_engine()->get_real_type(), (long double)eValue->GetReal());
	if (type == EventValue::TYPE_BOOLEAN)
		return value(machine->get_engine()->get_boolean_type(), eValue->GetBoolean());
	else if (type == EventValue::TYPE_STRING)
		return value(machine->get_engine()->get_string_type(), to_wide(eValue->GetString()));
	return gstd::value();
}
gstd::value DxScriptForEvent::Func_SetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (DxScriptForEvent*)machine->data;
	std::wstring wName = argv[0].as_string();
	std::string name = to_mbcs(wName);
	gstd::ref_count_ptr<EventValue> eValue = script->engine_->GetEventValue(name);
	if (eValue == nullptr)
		throw gstd::wexception(StringUtility::Format(L"存在しない変数:%s", name.c_str()));

	if (argv[1].get_type() == machine->get_engine()->get_real_type()) {
		eValue->SetReal(argv[1].as_real());
	} else if (argv[1].get_type() == machine->get_engine()->get_boolean_type()) {
		eValue->SetBoolean(argv[1].as_boolean());
	} else if (argv[1].get_type() == machine->get_engine()->get_string_type()) {
		eValue->SetString(to_mbcs(argv[1].as_string()));
	}
	return gstd::value();
}

//関数：キー入力
gstd::value DxScriptForEvent::Func_IsSkip(gstd::script_machine* machine, int argc, gstd::value const* argv)
{
	auto* script = (DxScriptForEvent*)machine->data;
	EventEngine* engine = script->engine_;
	gstd::ref_count_ptr<EventKeyState> key = engine->GetEventKeyState();

	return gstd::value(machine->get_engine()->get_boolean_type(), key->IsSkip() || key->IsNext());
}
