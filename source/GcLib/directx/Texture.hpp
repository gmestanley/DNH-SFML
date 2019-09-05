#ifndef __DIRECTX_TEXTURE__
#define __DIRECTX_TEXTURE__

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"

namespace directx {

class TextureData;
class Texture;
class TextureManager;
class TextureInfoPanel;

/**********************************************************
//Texture
**********************************************************/
class TextureData {
	friend Texture;
	friend TextureManager;
	friend TextureInfoPanel;

public:
	enum {
		TYPE_TEXTURE,
		TYPE_RENDER_TARGET,
	};

public:
	TextureData();
	virtual ~TextureData();
	std::wstring GetName() const { return name_; }
	D3DXIMAGE_INFO GetImageInfo() const { return infoImage_; }

protected:
	int type_;
	TextureManager* manager_;
	IDirect3DTexture9* pTexture_;
	D3DXIMAGE_INFO infoImage_;
	std::wstring name_;
	volatile bool bLoad_;

	IDirect3DSurface9* lpRenderSurface_; //バックバッファ実体(レンダリングターゲット用)
	IDirect3DSurface9* lpRenderZ_; //バックバッファのZバッファ実体(レンダリングターゲット用)
};

class Texture : public gstd::FileManager::LoadObject {
	friend TextureData;
	friend TextureManager;
	friend TextureInfoPanel;

public:
	Texture();
	Texture(const Texture* texture);
	~Texture() override;
	void Release();

	std::wstring GetName();
	bool CreateFromFile(const std::wstring& _Path);
	bool CreateRenderTarget(const std::wstring& name);
	bool CreateFromFileInLoadThread(const std::wstring& _Path, bool bLoadImageInfo = false);

	void SetTexture(IDirect3DTexture9* pTexture);
	IDirect3DTexture9* GetD3DTexture();
	IDirect3DSurface9* GetD3DSurface();
	IDirect3DSurface9* GetD3DZBuffer();

	int GetWidth() const;
	int GetHeight() const;
	bool IsLoad() { return data_ != nullptr && data_->bLoad_; }

protected:
	gstd::ref_count_ptr<TextureData> data_;
	TextureData* _GetTextureData() { return data_.GetPointer(); }
	const TextureData* _GetTextureData() const { return data_.GetPointer(); }
};

/**********************************************************
//TextureManager
**********************************************************/
class TextureManager : public DirectGraphicsListener, public gstd::FileManager::LoadThreadListener {
	friend Texture;
	friend TextureData;
	friend TextureInfoPanel;
	static TextureManager* thisBase_;

public:
	static const std::wstring TARGET_TRANSITION;

public:
	TextureManager();
	~TextureManager() override;
	static TextureManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	virtual void Clear();
	virtual void Add(const std::wstring& name, gstd::ref_count_ptr<Texture> texture); //テクスチャの参照を保持します
	virtual void Release(const std::wstring& name); //保持している参照を解放します
	virtual bool IsDataExists(const std::wstring& name);

	void ReleaseDirectGraphics() override { ReleaseDxResource(); }
	void RestoreDirectGraphics() override { RestoreDxResource(); }
	void ReleaseDxResource();
	void RestoreDxResource();

	gstd::ref_count_ptr<TextureData> GetTextureData(const std::wstring& name);
	gstd::ref_count_ptr<Texture> CreateFromFile(const std::wstring& _Path); //テクスチャを読み込みます。TextureDataは保持しますが、Textureは保持しません。
	gstd::ref_count_ptr<Texture> CreateRenderTarget(const std::wstring& name);
	gstd::ref_count_ptr<Texture> GetTexture(const std::wstring& name); //作成済みのテクスチャを取得します
	gstd::ref_count_ptr<Texture> CreateFromFileInLoadThread(const std::wstring& _Path, bool bLoadImageInfo = false);
	void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event) override;

	void SetInfoPanel(gstd::ref_count_ptr<TextureInfoPanel> panel) { panelInfo_ = panel; }

protected:
	mutable gstd::CriticalSection lock_;
	std::map<std::wstring, gstd::ref_count_ptr<Texture>> mapTexture_;
	std::map<std::wstring, gstd::ref_count_ptr<TextureData>> mapTextureData_;
	gstd::ref_count_ptr<TextureInfoPanel> panelInfo_;

	void _ReleaseTextureData(const std::wstring& name);
	bool _CreateFromFile(const std::wstring& path);
	bool _CreateRenderTarget(const std::wstring& name);
};

/**********************************************************
//TextureInfoPanel
**********************************************************/
class TextureInfoPanel : public gstd::WindowLogger::Panel, public gstd::Thread {
public:
	TextureInfoPanel();
	~TextureInfoPanel() override;
	void LocateParts() override;
	virtual void Update(TextureManager* manager);

protected:
	enum {
		ROW_ADDRESS,
		ROW_NAME,
		ROW_FULLNAME,
		ROW_COUNT_REFFRENCE,
		ROW_WIDTH_IMAGE,
		ROW_HEIGHT_IMAGE,
	};
	int timeUpdateInterval_;
	gstd::WListView wndListView_;
	bool _AddedLogger(HWND hTab) override;
	void _Run();
};

} // namespace directx

#endif
