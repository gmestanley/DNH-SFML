#ifndef __DIRECTX_DXTEXT__
#define __DIRECTX_DXTEXT__

#include "DxConstant.hpp"
#include "RenderObject.hpp"
#include "Texture.hpp"

namespace directx {

class DxChar;
class DxCharCache;
class DxCharCacheKey;
class DxTextRenderer;
class DxText;

/**********************************************************
//DxFont
**********************************************************/
class DxFont {
	friend DxCharCache;
	friend DxCharCacheKey;

public:
	enum {
		BORDER_NONE,
		BORDER_FULL,
		BORDER_SHADOW,
	};

public:
	DxFont();
	virtual ~DxFont();
	void SetLogFont(const LOGFONT& font) { info_ = font; }
	LOGFONT GetLogFont() const { return info_; }
	void SetTopColor(D3DCOLOR color) { colorTop_ = color; }
	D3DCOLOR GetTopColor() const { return colorTop_; }
	void SetBottomColor(D3DCOLOR color) { colorBottom_ = color; }
	D3DCOLOR GetBottomColor() const { return colorBottom_; }
	void SetBorderType(int type) { typeBorder_ = type; }
	int GetBorderType() const { return typeBorder_; }
	void SetBorderWidth(int width) { widthBorder_ = width; }
	int GetBorderWidth() const { return widthBorder_; }
	void SetBorderColor(D3DCOLOR color) { colorBorder_ = color; }
	D3DCOLOR GetBorderColor() const { return colorBorder_; }

protected:
	LOGFONT info_; //フォント種別
	D3DCOLOR colorTop_;
	D3DCOLOR colorBottom_;
	int typeBorder_; //縁取り
	int widthBorder_;
	D3DCOLOR colorBorder_; //縁取り色
};

/**********************************************************
//DxChar
//文字1文字のテクスチャ
**********************************************************/
class DxChar {
public:
	DxChar();
	virtual ~DxChar();
	bool Create(int code, gstd::Font& winFont, DxFont& dxFont);
	gstd::ref_count_ptr<Texture> GetTexture() { return texture_; }
	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	LOGFONT GetInfo() { return font_.GetLogFont(); }

private:
	gstd::ref_count_ptr<Texture> texture_;
	int code_;
	DxFont font_;

	int width_;
	int height_;
};

/**********************************************************
//DxCharCache
//文字キャッシュ
**********************************************************/
class DxCharCacheKey {
	friend DxCharCache;
	friend DxTextRenderer;

public:
	bool operator==(const DxCharCacheKey& key) const
	{
		bool res = true;
		res &= (code_ == key.code_);
		res &= (font_.colorTop_ == key.font_.colorTop_);
		res &= (font_.colorBottom_ == key.font_.colorBottom_);
		res &= (font_.typeBorder_ == key.font_.typeBorder_);
		res &= (font_.widthBorder_ == key.font_.widthBorder_);
		res &= (font_.colorBorder_ == key.font_.colorBorder_);
		if (!res)
			return res;
		res &= (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) == 0);
		return res;
	}
	bool operator<(const DxCharCacheKey& key) const
	{
		if (code_ != key.code_)
			return code_ < key.code_;
		if (font_.colorTop_ != key.font_.colorTop_)
			return font_.colorTop_ < key.font_.colorTop_;
		if (font_.colorBottom_ != key.font_.colorBottom_)
			return font_.colorBottom_ < key.font_.colorBottom_;
		// if (font_.typeBorder_ != key.font_.typeBorder_)
		// 	return font_.typeBorder_ != key.font_.typeBorder_;
		if (font_.widthBorder_ != key.font_.widthBorder_)
			return font_.widthBorder_ < key.font_.widthBorder_;
		if (font_.colorBorder_ != key.font_.colorBorder_)
			return font_.colorBorder_ < key.font_.colorBorder_;
		return (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) < 0);
	}

private:
	int code_;
	DxFont font_;
};
class DxCharCache {
	friend DxTextRenderer;

public:
	DxCharCache();
	~DxCharCache();
	void Clear();
	int GetCacheCount() const { return mapCache_.size(); }

	gstd::ref_count_ptr<DxChar> GetChar(DxCharCacheKey& key);
	void AddChar(DxCharCacheKey& key, gstd::ref_count_ptr<DxChar> value);

private:
	int sizeMax_;
	int countPri_;
	std::map<DxCharCacheKey, gstd::ref_count_ptr<DxChar>> mapCache_;
	std::map<int, DxCharCacheKey> mapPriKey_;
	std::map<DxCharCacheKey, int> mapKeyPri_;

	void _arrange();
};

/**********************************************************
//DxTextScanner
**********************************************************/
class DxTextScanner;
class DxTextToken {
	friend DxTextScanner;

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
	DxTextToken() { type_ = TK_UNKNOWN; }
	DxTextToken(Type type, const std::wstring& element)
	{
		type_ = type;
		element_ = element;
	}

	virtual ~DxTextToken() = default;
	Type GetType() const { return type_; }
	std::wstring& GetElement() { return element_; }

	int GetInteger();
	double GetReal();
	bool GetBoolean();
	std::wstring GetString();
	std::wstring GetIdentifier() const;

protected:
	Type type_;
	std::wstring element_;
};

class DxTextScanner {
public:
	const static int TOKEN_TAG_START;
	const static int TOKEN_TAG_END;
	const static std::wstring TAG_START;
	const static std::wstring TAG_END;
	const static std::wstring TAG_NEW_LINE;
	const static std::wstring TAG_RUBY;
	const static std::wstring TAG_FONT;

public:
	DxTextScanner(wchar_t* str, int charCount);
	DxTextScanner(const std::wstring& str);
	DxTextScanner(const std::vector<wchar_t>& buf);
	virtual ~DxTextScanner();

	DxTextToken& GetToken(); //現在のトークンを取得
	DxTextToken& Next();
	bool HasNext();
	void CheckType(const DxTextToken& tok, int type);
	void CheckIdentifer(const DxTextToken& tok, const std::wstring& id);
	int GetCurrentLine() const;
	int SearchCurrentLine();

	std::vector<wchar_t>::iterator GetCurrentPointer() const;
	void SetCurrentPointer(std::vector<wchar_t>::iterator pos);
	void SetPointerBegin() { pointer_ = buffer_.begin(); }
	int GetCurrentPosition() const;
	void SetTagScanEnable(bool bEnable) { bTagScan_ = bEnable; }

protected:
	int line_;
	std::vector<wchar_t> buffer_;
	std::vector<wchar_t>::iterator pointer_; //今の位置
	DxTextToken token_; //現在のトークン
	bool bTagScan_;

	wchar_t _NextChar(); //ポインタを進めて次の文字を調べる
	virtual void _SkipComment(); //コメントをとばす
	virtual void _SkipSpace(); //空白をとばす
	virtual void _RaiseError(const std::wstring& str); //例外を投げます
	bool _IsTextStartSign();
	bool _IsTextScan();
};

/**********************************************************
//DxTextRenderer
//テキスト描画エンジン
**********************************************************/
class DxTextLine;
class DxTextInfo;
class DxTextRenderer;
class DxTextTag;
class DxTextTag {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_RUBY,
		TYPE_FONT
	};

public:
	DxTextTag()
	{
		indexTag_ = 0;
		typeTag_ = TYPE_UNKNOWN;
	}
	virtual ~DxTextTag() = default;
	int GetTagType() const { return typeTag_; }
	int GetTagIndex() const { return indexTag_; }
	void SetTagIndex(int index) { indexTag_ = index; }

protected:
	int typeTag_;
	int indexTag_;
};
class DxTextTag_Ruby : public DxTextTag {
public:
	DxTextTag_Ruby()
	{
		typeTag_ = TYPE_RUBY;
		leftMargin_ = 0;
	}
	int GetLeftMargin() const { return leftMargin_; }
	void SetLeftMargin(int left) { leftMargin_ = left; }

	std::wstring& GetText() { return text_; }
	void SetText(std::wstring text) { text_ = text; }
	std::wstring GetRuby() { return ruby_; }
	void SetRuby(std::wstring ruby) { ruby_ = ruby; }

	gstd::ref_count_ptr<DxText> GetRenderText() { return dxText_; }
	void SetRenderText(gstd::ref_count_ptr<DxText> text) { dxText_ = text; }

private:
	int leftMargin_;
	std::wstring text_;
	std::wstring ruby_;
	gstd::ref_count_ptr<DxText> dxText_;
};
class DxTextTag_Font : public DxTextTag {
private:
	DxFont font_;

public:
	DxTextTag_Font() { typeTag_ = TYPE_FONT; }
	void SetFont(DxFont font) { font_ = font; }
	DxFont& GetFont() { return font_; }

};
class DxTextLine {
	friend DxTextRenderer;

public:
	DxTextLine()
	{
		width_ = 0;
		height_ = 0;
		sidePitch_ = 0;
	}
	virtual ~DxTextLine() = default;
	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	int GetSidePitch() const { return sidePitch_; }
	void SetSidePitch(int pitch) { sidePitch_ = pitch; }
	std::vector<int>& GetTextCodes() { return code_; }
	int GetTextCodeCount() const { return code_.size(); }
	int GetTagCount() const { return tag_.size(); }
	gstd::ref_count_ptr<DxTextTag> GetTag(int index) { return tag_[index]; }

protected:
	int width_;
	int height_;
	int sidePitch_;
	std::vector<int> code_;
	std::vector<gstd::ref_count_ptr<DxTextTag>> tag_;
};
class DxTextInfo {
	friend DxTextRenderer;

public:
	DxTextInfo()
	{
		totalWidth_ = 0;
		totalHeight_ = 0;
		lineValidStart_ = 1;
		lineValidEnd_ = 0;
		bAutoIndent_ = false;
	}
	virtual ~DxTextInfo() = default;
	int GetTotalWidth() const { return totalWidth_; }
	int GetTotalHeight() const { return totalHeight_; }
	int GetValidStartLine() const { return lineValidStart_; }
	int GetValidEndLine() const { return lineValidEnd_; }
	void SetValidStartLine(int line) { lineValidStart_ = line; }
	void SetValidEndLine(int line) { lineValidEnd_ = line; }
	bool IsAutoIndent() const { return bAutoIndent_; }
	void SetAutoIndent(bool bEnable) { bAutoIndent_ = bEnable; }

	int GetLineCount() const { return textLine_.size(); }
	void AddTextLine(gstd::ref_count_ptr<DxTextLine> text) { textLine_.push_back(text), lineValidEnd_++; }
	gstd::ref_count_ptr<DxTextLine> GetTextLine(int pos) { return textLine_[pos]; }

protected:
	int totalWidth_;
	int totalHeight_;
	int lineValidStart_;
	int lineValidEnd_;
	bool bAutoIndent_;
	std::vector<gstd::ref_count_ptr<DxTextLine>> textLine_;
};

class DxTextRenderObject {
private:
	struct ObjectData {
		POINT bias;
		gstd::ref_count_ptr<Sprite2D> sprite;
	};

public:
	DxTextRenderObject();
	virtual ~DxTextRenderObject() = default;

	void Render();
	void AddRenderObject(gstd::ref_count_ptr<Sprite2D> obj);
	void AddRenderObject(gstd::ref_count_ptr<DxTextRenderObject> obj, POINT bias);

	POINT GetPosition() { return position_; }
	void SetPosition(POINT pos)
	{
		position_.x = pos.x;
		position_.y = pos.y;
	}
	void SetPosition(int x, int y)
	{
		position_.x = x;
		position_.y = y;
	}
	void SetVertexColor(D3DCOLOR color) { color_ = color; }

	void SetAngle(const D3DXVECTOR3& angle) { angle_ = angle; }
	void SetScale(const D3DXVECTOR3& scale) { scale_ = scale; }
	void SetTransCenter(const D3DXVECTOR2& center) { center_ = center; }
	void SetAutoCenter(bool bAuto) { bAutoCenter_ = bAuto; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }

	gstd::ref_count_ptr<Shader> GetShader() { return shader_; }
	void SetShader(gstd::ref_count_ptr<Shader> shader) { shader_ = shader; }

protected:
	POINT position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
	D3DCOLOR color_;
	std::list<ObjectData> listData_;
	D3DXVECTOR2 center_; //座標変換の中心
	bool bAutoCenter_;
	bool bPermitCamera_;
	gstd::ref_count_ptr<Shader> shader_;
};

class DxTextRenderer {
private:
	static DxTextRenderer* thisBase_;

public:
	enum {
		ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT,
		ALIGNMENT_TOP,
		ALIGNMENT_BOTTOM,
		ALIGNMENT_CENTER,
	};

public:
	DxTextRenderer();
	virtual ~DxTextRenderer();
	static DxTextRenderer* GetBase() { return thisBase_; }
	bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	void ClearCache() { cache_.Clear(); }
	void SetFont(LOGFONT& logFont) { winFont_.CreateFontIndirect(logFont); }
	void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }
	gstd::ref_count_ptr<DxTextInfo> GetTextInfo(DxText* dxText);

	gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject(DxText* dxText, gstd::ref_count_ptr<DxTextInfo> textInfo);

	void Render(DxText* dxText);
	void Render(DxText* dxText, gstd::ref_count_ptr<DxTextInfo> textInfo);

	int GetCacheCount() const { return cache_.GetCacheCount(); }

	bool AddFontFromFile(std::wstring path);

protected:
	DxCharCache cache_;
	gstd::Font winFont_;
	D3DCOLOR colorVertex_;
	gstd::CriticalSection lock_;

	SIZE _GetTextSize(HDC hDC, const wchar_t* pText);
	gstd::ref_count_ptr<DxTextLine> _GetTextInfoSub(const std::wstring& text, const DxText* dxText, DxTextInfo* textInfo, gstd::ref_count_ptr<DxTextLine> textLine, HDC& hDC, int& totalWidth, int& totalHeight);
	void _CreateRenderObject(gstd::ref_count_ptr<DxTextRenderObject> objRender, POINT pos, DxFont& dxFont, gstd::ref_count_ptr<DxTextLine> textLine);
	std::wstring _ReplaceRenderText(std::wstring& text);
};

/**********************************************************
//DxText
//テキスト描画
**********************************************************/
class DxText {
	friend DxTextRenderer;

public:
	enum {
		ALIGNMENT_LEFT = DxTextRenderer::ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT = DxTextRenderer::ALIGNMENT_RIGHT,
		ALIGNMENT_TOP = DxTextRenderer::ALIGNMENT_TOP,
		ALIGNMENT_BOTTOM = DxTextRenderer::ALIGNMENT_BOTTOM,
		ALIGNMENT_CENTER = DxTextRenderer::ALIGNMENT_CENTER,
	};
public:
	DxText();
	virtual ~DxText();
	void Copy(const DxText& src);
	virtual void Render();
	void Render(gstd::ref_count_ptr<DxTextInfo> textInfo);

	gstd::ref_count_ptr<DxTextInfo> GetTextInfo();
	gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject();
	gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject(gstd::ref_count_ptr<DxTextInfo> textInfo);

	DxFont GetFont() const { return dxFont_; }
	void SetFont(const DxFont& font) { dxFont_ = font; }
	void SetFont(const LOGFONT& logFont) { dxFont_.SetLogFont(logFont); }

	void SetFontType(const wchar_t* type);
	int GetFontSize() const { return dxFont_.GetLogFont().lfHeight; }
	void SetFontSize(int size)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfHeight = size;
		SetFont(info);
	}
	void SetFontBold(bool bBold)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
		SetFont(info);
	}
	void SetFontItalic(bool bItalic)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfItalic = static_cast<BYTE>(bItalic);
		SetFont(info);
	}
	void SetFontUnderLine(bool bLine)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfUnderline = static_cast<BYTE>(bLine);
		SetFont(info);
	}

	void SetFontColorTop(D3DCOLOR color) { dxFont_.SetTopColor(color); }
	void SetFontColorBottom(D3DCOLOR color) { dxFont_.SetBottomColor(color); }
	void SetFontBorderWidth(int width) { dxFont_.SetBorderWidth(width); }
	void SetFontBorderType(int type) { dxFont_.SetBorderType(type); }
	void SetFontBorderColor(D3DCOLOR color) { dxFont_.SetBorderColor(color); }

	POINT GetPosition() const { return pos_; }
	void SetPosition(int x, int y)
	{
		pos_.x = x;
		pos_.y = y;
	}
	void SetPosition(POINT pos) { pos_ = pos; }
	int GetMaxWidth() const { return widthMax_; }
	void SetMaxWidth(int width) { widthMax_ = width; }
	int GetMaxHeight() const { return heightMax_; }
	void SetMaxHeight(int height) { heightMax_ = height; }
	int GetSidePitch() const { return sidePitch_; }
	void SetSidePitch(int pitch) { sidePitch_ = pitch; }
	int GetLinePitch() const { return linePitch_; }
	void SetLinePitch(int pitch) { linePitch_ = pitch; }
	RECT GetMargin() const { return margin_; }
	void SetMargin(RECT margin) { margin_ = margin; }
	int GetHorizontalAlignment() const { return alignmentHorizontal_; }
	void SetHorizontalAlignment(int value) { alignmentHorizontal_ = value; }
	int GetVerticalAlignment() const { return alignmentVertical_; }
	void SetVerticalAlignment(int value) { alignmentVertical_ = value; }

	D3DCOLOR GetVertexColor() const { return colorVertex_; }
	void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }
	bool IsPermitCamera() const { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
	bool IsSyntacticAnalysis() const { return bSyntacticAnalysis_; }
	void SetSyntacticAnalysis(bool bEnable) { bSyntacticAnalysis_ = bEnable; }

	std::wstring& GetText() { return text_; }
	const std::wstring& GetText() const { return text_; }
	void SetText(const std::wstring& text) { text_ = text; }

	gstd::ref_count_ptr<Shader> GetShader() { return shader_; }
	void SetShader(gstd::ref_count_ptr<Shader> shader) { shader_ = shader; }

protected:
	DxFont dxFont_;
	POINT pos_;
	int widthMax_;
	int heightMax_;
	int sidePitch_;
	int linePitch_;
	RECT margin_;
	int alignmentHorizontal_;
	int alignmentVertical_;
	D3DCOLOR colorVertex_;
	bool bPermitCamera_;
	bool bSyntacticAnalysis_;

	gstd::ref_count_ptr<Shader> shader_;
	std::wstring text_;
};

/**********************************************************
//DxTextStepper
**********************************************************/
class DxTextStepper {
public:
	DxTextStepper();
	virtual ~DxTextStepper();
	void Clear();

	std::wstring GetText() const { return text_; }
	void Next();
	void NextSkip();
	bool HasNext() const;
	void SetSource(const std::wstring& text);

protected:
	int posNext_;
	std::wstring text_;
	std::wstring source_;
	int framePerSec_;
	int countNextPerFrame_;
	double countFrame_;
	void _Next();
};

} // namespace directx

#endif
