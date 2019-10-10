#ifndef __DIRECTX_RENDEROBJECT__
#define __DIRECTX_RENDEROBJECT__

#include "DxConstant.hpp"
#include "DxUtility.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace directx {

class RenderObjectBase;
class RenderManager;
/**********************************************************
//FVF頂点フォーマット
//http://msdn.microsoft.com/ja-jp/library/cc324487.aspx
**********************************************************/
struct VERTEX_TL {
	//座標3D変換済み、ライティング済み
	VERTEX_TL() = default;
	VERTEX_TL(D3DXVECTOR4 pos, D3DCOLOR dcol)
		: position(pos)
		, diffuse_color(dcol)
	{
	}
	D3DXVECTOR4 position;
	D3DCOLOR diffuse_color;
	enum { fvf = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE) };
};

struct VERTEX_TLX {
	//座標3D変換済み、ライティング済み、テクスチャ有り
	VERTEX_TLX() = default;
	VERTEX_TLX(D3DXVECTOR4 pos, D3DCOLOR diffcol, D3DXVECTOR2 tex)
		: position(pos)
		, diffuse_color(diffcol)
		, texcoord(tex)
	{
	}
	D3DXVECTOR4 position;
	D3DCOLOR diffuse_color;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) };
};

struct VERTEX_L {
	//ライティング済み
	VERTEX_L() = default;
	VERTEX_L(D3DXVECTOR3 pos, D3DCOLOR col)
		: position(pos)
		, diffuse_color(col)
	{
	}
	D3DXVECTOR3 position;
	D3DCOLOR diffuse_color;
	enum { fvf = (D3DFVF_XYZ | D3DFVF_DIFFUSE) };
};

struct VERTEX_LX {
	//ライティング済み、テクスチャ有り
	VERTEX_LX() = default;
	VERTEX_LX(D3DXVECTOR3 pos, D3DCOLOR diffcol, D3DXVECTOR2 tex)
		: position(pos)
		, diffuse_color(diffcol)
		, texcoord(tex)
	{
	}
	D3DXVECTOR3 position;
	D3DCOLOR diffuse_color;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1) };
};

struct VERTEX_N {
	//未ライティング
	VERTEX_N() = default;
	VERTEX_N(D3DXVECTOR3 pos, D3DXVECTOR3 n)
		: position(pos)
		, normal(n)
	{
	}
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	enum { fvf = (D3DFVF_XYZ | D3DFVF_NORMAL) };
};

struct VERTEX_NX {
	//未ライティング、テクスチャ有り
	VERTEX_NX() = default;
	VERTEX_NX(D3DXVECTOR3 pos, D3DXVECTOR3 n, D3DXVECTOR2 tc)
		: position(pos)
		, normal(n)
		, texcoord(tc)
	{
	}
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) };
};

struct VERTEX_NXG {
	VERTEX_NXG() = default;
	VERTEX_NXG(D3DXVECTOR3& pos, D3DXVECTOR3& n, D3DXVECTOR2& tc)
		: position(pos)
		, normal(n)
		, texcoord(tc)
	{
	}
	D3DXVECTOR3 position;
	float blend[3];
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_TEX1) };
};

struct VERTEX_B1NX {
	//未ライティング、テクスチャ有り、頂点ブレンド1
	VERTEX_B1NX() = default;
	VERTEX_B1NX(D3DXVECTOR3 pos, DWORD bi, D3DXVECTOR3 n, D3DXVECTOR2 tc)
		: position(pos)
		, normal(n)
		, texcoord(tc)
	{
		blendIndex = bi;
	}
	D3DXVECTOR3 position;
	DWORD blendIndex;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZB1 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1) };
};

struct VERTEX_B2NX {
	//未ライティング、テクスチャ有り、頂点ブレンド2
	VERTEX_B2NX() = default;
	VERTEX_B2NX(D3DXVECTOR3 pos, float rate, BYTE index1, BYTE index2, D3DXVECTOR3 n, D3DXVECTOR2 tc)
		: position(pos)
		, normal(n)
		, texcoord(tc)
	{
		blendRate = rate;
		gstd::BitAccess::SetByte(blendIndex, 0, index1);
		gstd::BitAccess::SetByte(blendIndex, 8, index2);
	}
	D3DXVECTOR3 position;
	float blendRate;
	DWORD blendIndex;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZB2 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1) };
};

struct VERTEX_B4NX {
	//未ライティング、テクスチャ有り、頂点ブレンド4
	VERTEX_B4NX() = default;
	VERTEX_B4NX(D3DXVECTOR3 pos, const float rate[3], BYTE index[4], D3DXVECTOR3 n, D3DXVECTOR2 tc)
		: position(pos)
		, normal(n)
		, texcoord(tc)
	{
		for (int iRate = 0; iRate < 3; ++iRate)
			blendRate[iRate] = rate[iRate];
		for (int iIndex = 0; iIndex < 4; ++iIndex)
			gstd::BitAccess::SetByte(blendIndex, 8 * iIndex, index[iIndex]);
	}
	D3DXVECTOR3 position;
	float blendRate[3];
	DWORD blendIndex;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	enum { fvf = (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1) };
};

class RenderStateFunction;
class RenderBlock;
class RenderObject;

/**********************************************************
//RenderBlock
**********************************************************/
class RenderBlock {
public:
	RenderBlock();
	virtual ~RenderBlock();
	void SetRenderFunction(gstd::ref_count_ptr<RenderStateFunction> func) { func_ = func; }
	virtual void Render();

	virtual void CalculateZValue() = 0;
	float GetZValue() const { return posSortKey_; }
	void SetZValue(float pos) { posSortKey_ = pos; }
	virtual bool IsTranslucent() = 0; //Zソート対象に使用

	void SetRenderObject(gstd::ref_count_ptr<RenderObject> obj) { obj_ = obj; }
	gstd::ref_count_ptr<RenderObject> GetRenderObject() { return obj_; }
	void SetPosition(D3DXVECTOR3& pos) { position_ = pos; }
	void SetAngle(D3DXVECTOR3& angle) { angle_ = angle; }
	void SetScale(D3DXVECTOR3& scale) { scale_ = scale; }

protected:
	float posSortKey_;
	gstd::ref_count_ptr<RenderStateFunction> func_;
	gstd::ref_count_ptr<RenderObject> obj_;

	D3DXVECTOR3 position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
};

class RenderBlocks {
public:
	RenderBlocks() = default;
	virtual ~RenderBlocks() = default;
	void Add(gstd::ref_count_ptr<RenderBlock> block) { listBlock_.push_back(block); }
	std::list<gstd::ref_count_ptr<RenderBlock>>& GetList() { return listBlock_; }

protected:
	std::list<gstd::ref_count_ptr<RenderBlock>> listBlock_;
};

/**********************************************************
//RenderManager
//レンダリング管理
//3D不透明オブジェクト
//3D半透明オブジェクトZソート順
//2Dオブジェクト
//順に描画する
**********************************************************/
class RenderManager {
public:
	RenderManager();
	virtual ~RenderManager();
	virtual void Render();
	void AddBlock(gstd::ref_count_ptr<RenderBlock> block);
	void AddBlock(gstd::ref_count_ptr<RenderBlocks> blocks);

protected:
	std::list<gstd::ref_count_ptr<RenderBlock>> listBlockOpaque_;
	std::list<gstd::ref_count_ptr<RenderBlock>> listBlockTranslucent_;
};

/**********************************************************
//RenderStateFunction
**********************************************************/
class RenderStateFunction {
	friend RenderObjectBase;

public:
	RenderStateFunction();
	virtual ~RenderStateFunction();
	void CallRenderStateFunction();

	//レンダリングステート設定(RenderManager用)
	void SetLightingEnable(bool bEnable); //ライティング
	void SetCullingMode(DWORD mode); //カリング
	void SetZBufferEnable(bool bEnable); //Zバッファ参照
	void SetZWriteEnalbe(bool bEnable); //Zバッファ書き込み
	void SetBlendMode(DWORD mode, int stage = 0);
	void SetTextureFilter(DWORD mode, int stage = 0);

private:
	enum FUNC_TYPE {
		FUNC_LIGHTING,
		FUNC_CULLING,
		FUNC_ZBUFFER_ENABLE,
		FUNC_ZBUFFER_WRITE_ENABLE,
		FUNC_BLEND,
		FUNC_TEXTURE_FILTER,
	};

	std::map<FUNC_TYPE, gstd::ref_count_ptr<gstd::ByteBuffer>> mapFuncRenderState_;
};

class Matrices {
public:
	Matrices() = default;
	virtual ~Matrices() = default;
	void SetSize(int size)
	{
		matrix_.resize(size);
		for (int iMat = 0; iMat < size; ++iMat) {
			D3DXMatrixIdentity(&matrix_[iMat]);
		}
	}
	int GetSize() const { return matrix_.size(); }
	void SetMatrix(int index, D3DXMATRIX& mat) { matrix_[index] = mat; }
	D3DXMATRIX& GetMatrix(int index) { return matrix_[index]; }

private:
	std::vector<D3DXMATRIX> matrix_;
};

/**********************************************************
//RenderObject
//レンダリングオブジェクト
//描画の最小単位
//RenderManagerに登録して描画してもらう
//(直接描画も可能)
**********************************************************/
class RenderObject {
public:
	RenderObject();
	virtual ~RenderObject();
	virtual void Render() = 0;
	virtual void InitializeVertexBuffer() {}
	virtual void CalculateWeightCenter() {}
	D3DXVECTOR3 GetWeightCenter() const { return posWeightCenter_; }
	gstd::ref_count_ptr<Texture> GetTexture(int pos = 0) { return texture_[pos]; }

	void SetRalativeMatrix(D3DXMATRIX mat) { matRelative_ = mat; }

	//頂点設定
	void SetPrimitiveType(D3DPRIMITIVETYPE type) { typePrimitive_ = type; }
	virtual void SetVertexCount(int count)
	{
		vertex_.SetSize(count * strideVertexStreamZero_);
		ZeroMemory(vertex_.GetPointer(), vertex_.GetSize());
	}
	virtual int GetVertexCount() const { return vertex_.GetSize() / strideVertexStreamZero_; }
	void SetVertexIndicies(const std::vector<short>& indices) { vertexIndices_ = indices; }
	gstd::ByteBuffer* GetVertexPointer() { return &vertex_; }

	//描画用設定
	void SetPosition(const D3DXVECTOR3& pos) { position_ = pos; }
	void SetPosition(float x, float y, float z)
	{
		position_.x = x;
		position_.y = y;
		position_.z = z;
	}
	void SetX(float x) { position_.x = x; }
	void SetY(float y) { position_.y = y; }
	void SetZ(float z) { position_.z = z; }
	void SetAngle(const D3DXVECTOR3& angle) { angle_ = angle; }
	void SetAngleXYZ(float angx = 0.0F, float angy = 0.0F, float angz = 0.0F)
	{
		angle_.x = angx;
		angle_.y = angy;
		angle_.z = angz;
	}
	void SetScale(const D3DXVECTOR3& scale) { scale_ = scale; }
	void SetScaleXYZ(float sx = 1.0F, float sy = 1.0F, float sz = 1.0F)
	{
		scale_.x = sx;
		scale_.y = sy;
		scale_.z = sz;
	}
	void SetTexture(const Texture* texture, int stage = 0); //テクスチャ設定
	void SetTexture(gstd::ref_count_ptr<Texture> texture, int stage = 0); //テクスチャ設定

	bool IsCoordinate2D() const { return bCoordinate2D_; }
	void SetCoordinate2D(bool b) { bCoordinate2D_ = b; }

	gstd::ref_count_ptr<Shader> GetShader() { return shader_; }
	void SetShader(gstd::ref_count_ptr<Shader> shader) { shader_ = shader; }
	void BeginShader();
	void EndShader();

protected:
	D3DPRIMITIVETYPE typePrimitive_; //
	int strideVertexStreamZero_; //1頂点のサイズ
	gstd::ByteBuffer vertex_; //頂点
	std::vector<short> vertexIndices_;
	std::vector<gstd::ref_count_ptr<Texture>> texture_; //テクスチャ
	D3DXVECTOR3 posWeightCenter_; //重心

	//シェーダ用
	IDirect3DVertexDeclaration9* pVertexDecl_;
	IDirect3DVertexBuffer9* pVertexBuffer_;
	IDirect3DIndexBuffer9* pIndexBuffer_;

	D3DXVECTOR3 position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
	D3DXMATRIX matRelative_; //関係行列
	bool bCoordinate2D_; //2D座標指定
	gstd::ref_count_ptr<Shader> shader_;

	virtual void _ReleaseVertexBuffer();
	virtual void _RestoreVertexBuffer();
	virtual void _CreateVertexDeclaration() {}

	int _GetPrimitiveCount() const;
	void _SetTextureStageCount(int count)
	{
		texture_.resize(count);
		for (auto& texture : texture_)
			texture = nullptr;
	}
	virtual D3DXMATRIX _CreateWorldTransformMaxtrix(); //position_,angle_,scale_から作成
	void _SetCoordinate2dDeviceMatrix();
};

/**********************************************************
//RenderObjectTLX
//座標3D変換済み、ライティング済み、テクスチャ有り
//2D自由変形スプライト用
**********************************************************/
class RenderObjectTLX : public RenderObject {
public:
	RenderObjectTLX();
	~RenderObjectTLX() override;
	void Render() override;
	void SetVertexCount(int count) override;

	//頂点設定
	VERTEX_TLX* GetVertex(int index);
	void SetVertex(int index, const VERTEX_TLX& vertex);
	void SetVertexPosition(int index, float x, float y, float z = 1.0F, float w = 1.0F);
	void SetVertexUV(int index, float u, float v);
	void SetVertexColor(int index, D3DCOLOR color);
	void SetVertexColorARGB(int index, int a, int r, int g, int b);
	void SetVertexAlpha(int index, int alpha);
	void SetVertexColorRGB(int index, int r, int g, int b);
	void SetColorRGB(D3DCOLOR color);
	void SetAlpha(int alpha);

	//カメラ
	bool IsPermitCamera() const { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }

protected:
	bool bPermitCamera_;
	gstd::ByteBuffer vertCopy_;

	void _CreateVertexDeclaration() override;
};

/**********************************************************
//RenderObjectLX
//ライティング済み、テクスチャ有り
//3Dエフェクト用
**********************************************************/
class RenderObjectLX : public RenderObject {
public:
	RenderObjectLX();
	~RenderObjectLX() override;
	void Render() override;
	void SetVertexCount(int count) override;

	//頂点設定
	VERTEX_LX* GetVertex(int index);
	void SetVertex(int index, VERTEX_LX& vertex);
	void SetVertexPosition(int index, float x, float y, float z);
	void SetVertexUV(int index, float u, float v);
	void SetVertexColor(int index, D3DCOLOR color);
	void SetVertexColorARGB(int index, int a, int r, int g, int b);
	void SetVertexAlpha(int index, int alpha);
	void SetVertexColorRGB(int index, int r, int g, int b);
	void SetColorRGB(D3DCOLOR color);
	void SetAlpha(int alpha);

protected:
	void _CreateVertexDeclaration() override;
};

/**********************************************************
//RenderObjectNX
//法線有り、テクスチャ有り
**********************************************************/
class RenderObjectNX : public RenderObject {
public:
	RenderObjectNX();
	~RenderObjectNX() override;
	void Render() override;

	//頂点設定
	VERTEX_NX* GetVertex(int index);
	void SetVertex(int index, const VERTEX_NX& vertex);
	void SetVertexPosition(int index, float x, float y, float z);
	void SetVertexUV(int index, float u, float v);
	void SetVertexNormal(int index, float x, float y, float z);
	void SetColor(D3DCOLOR color) { color_ = color; }

protected:
	D3DCOLOR color_;
	void _CreateVertexDeclaration() override;
};

/**********************************************************
//RenderObjectBNX
//頂点ブレンド
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectBNX : public RenderObject {
public:
	struct Vertex {
		D3DXVECTOR3 position;
		D3DXVECTOR4 blendRate;
		D3DXVECTOR4 blendIndex;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
	};

public:
	RenderObjectBNX();
	~RenderObjectBNX() override;
	void InitializeVertexBuffer() override;
	void Render() override;

	//描画用設定
	void SetMatrix(gstd::ref_count_ptr<Matrices> matrix) { matrix_ = matrix; }
	void SetColor(D3DCOLOR color) { color_ = color; }

protected:
	gstd::ref_count_ptr<Matrices> matrix_;
	D3DCOLOR color_;
	D3DMATERIAL9 materialBNX_;
	void _CreateVertexDeclaration() override;
	virtual void _CopyVertexBufferOnInitialize() = 0;
};

class RenderObjectBNXBlock : public RenderBlock {
public:
	void SetMatrix(gstd::ref_count_ptr<Matrices> matrix) { matrix_ = matrix; }
	void SetColor(D3DCOLOR color) { color_ = color; }
	bool IsTranslucent() const { return ColorAccess::GetColorA(color_) != 255; }

protected:
	gstd::ref_count_ptr<Matrices> matrix_;
	D3DCOLOR color_;
};

/**********************************************************
//RenderObjectB2NX
//頂点ブレンド2
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectB2NX : public RenderObjectBNX {
public:
	RenderObjectB2NX();
	~RenderObjectB2NX() override;

	void CalculateWeightCenter() override;

	//頂点設定
	VERTEX_B2NX* GetVertex(int index);
	void SetVertex(int index, VERTEX_B2NX& vertex);
	void SetVertexPosition(int index, float x, float y, float z);
	void SetVertexUV(int index, float u, float v);
	void SetVertexBlend(int index, int pos, BYTE indexBlend, float rate);
	void SetVertexNormal(int index, float x, float y, float z);

protected:
	void _CopyVertexBufferOnInitialize() override;
};

class RenderObjectB2NXBlock : public RenderObjectBNXBlock {
public:
	RenderObjectB2NXBlock();
	~RenderObjectB2NXBlock() override;
	void Render() override;
};

/**********************************************************
//RenderObjectB4NX
//頂点ブレンド4
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectB4NX : public RenderObjectBNX {
public:
	RenderObjectB4NX();
	~RenderObjectB4NX() override;

	void CalculateWeightCenter() override;

	//頂点設定
	VERTEX_B4NX* GetVertex(int index);
	void SetVertex(int index, const VERTEX_B4NX& vertex);
	void SetVertexPosition(int index, float x, float y, float z);
	void SetVertexUV(int index, float u, float v);
	void SetVertexBlend(int index, int pos, BYTE indexBlend, float rate);
	void SetVertexNormal(int index, float x, float y, float z);

protected:
	void _CopyVertexBufferOnInitialize() override;
};

class RenderObjectB4NXBlock : public RenderObjectBNXBlock {
public:
	RenderObjectB4NXBlock();
	~RenderObjectB4NXBlock() override;
	void Render() override;
};

/**********************************************************
//Sprite2D
//矩形スプライト
**********************************************************/
class Sprite2D : public RenderObjectTLX {
public:
	Sprite2D();
	~Sprite2D() override;
	void Copy(const Sprite2D* src);
	void SetSourceRect(const RECT_D& rcSrc);
	void SetDestinationRect(const RECT_D& rcDest);
	void SetDestinationCenter();
	void SetVertex(const RECT_D& rcSrc, const RECT_D& rcDest, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));

	RECT_D GetDestinationRect();
};

/**********************************************************
//SpriteList2D
//矩形スプライトリスト
**********************************************************/
class SpriteList2D : public RenderObjectTLX {
public:
	SpriteList2D();
	int GetVertexCount() const override;
	void Render() override;
	void ClearVertexCount()
	{
		countRenderVertex_ = 0;
		bCloseVertexList_ = false;
	}
	void AddVertex();
	void SetSourceRect(const RECT_D& rcSrc) { rcSrc_ = rcSrc; }
	void SetDestinationRect(const RECT_D& rcDest) { rcDest_ = rcDest; }
	void SetDestinationCenter();
	D3DCOLOR GetColor() const { return color_; }
	void SetColor(D3DCOLOR color) { color_ = color; }
	void CloseVertex();

private:
	int countRenderVertex_;
	RECT_D rcSrc_;
	RECT_D rcDest_;
	D3DCOLOR color_;
	bool bCloseVertexList_;
	void _AddVertex(VERTEX_TLX& vertex);
};

/**********************************************************
//Sprite3D
//矩形スプライト
**********************************************************/
class Sprite3D : public RenderObjectLX {
public:
	Sprite3D();
	~Sprite3D() override;
	void SetSourceRect(const RECT_D& rcSrc);
	void SetDestinationRect(const RECT_D& rcDest);
	void SetVertex(const RECT_D& rcSrc, const RECT_D& rcDest, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));
	void SetSourceDestRect(const RECT_D& rcSrc);
	void SetVertex(const RECT_D& rcSrc, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));
	void SetBillboardEnable(bool bEnable) { bBillboard_ = bEnable; }

protected:
	bool bBillboard_;
	D3DXMATRIX _CreateWorldTransformMaxtrix() override;
};

/**********************************************************
//TrajectoryObject3D
//3D軌跡
**********************************************************/
class TrajectoryObject3D : public RenderObjectLX {
public:
	TrajectoryObject3D();
	~TrajectoryObject3D() override;
	virtual void Work();
	void Render() override;
	void SetInitialLine(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	void AddPoint(D3DXMATRIX mat);
	void SetAlphaVariation(int diff) { diffAlpha_ = diff; }
	void SetComplementCount(int count) { countComplement_ = count; }
	void SetColor(D3DCOLOR color) { color_ = color; }

private:
	struct Data {
		int alpha;
		D3DXVECTOR3 pos1;
		D3DXVECTOR3 pos2;
	};

protected:
	D3DCOLOR color_;
	int diffAlpha_;
	int countComplement_;
	Data dataInit_;
	Data dataLast1_;
	Data dataLast2_;
	std::list<Data> listData_;
	D3DXMATRIX _CreateWorldTransformMaxtrix() override;

};

/**********************************************************
//DxMesh
**********************************************************/
enum {
	MESH_ELFREINA,
	MESH_METASEQUOIA,
};

class DxMeshManager;
class DxMeshData {
public:
	friend DxMeshManager;

public:
	DxMeshData();
	virtual ~DxMeshData();
	void SetName(const std::wstring& name) { name_ = name; }
	std::wstring& GetName() { return name_; }
	virtual bool CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;

protected:
	std::wstring name_;
	DxMeshManager* manager_;
	volatile bool bLoad_;
};
class DxMesh : public gstd::FileManager::LoadObject {
public:
	friend DxMeshManager;

public:
	DxMesh();
	~DxMesh() override;
	virtual void Release();
	bool CreateFromFile(const std::wstring& _path);
	virtual bool CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;
	virtual bool CreateFromFileInLoadThread(const std::wstring& path, int type);
	virtual bool CreateFromFileInLoadThread(const std::wstring& path) = 0;
	virtual std::wstring GetPath() = 0;

	virtual void Render() = 0;
	virtual void Render(const std::wstring& nameAnime, int time) { Render(); }
	void SetPosition(D3DXVECTOR3 pos) { position_ = pos; }
	void SetPosition(float x, float y, float z)
	{
		position_.x = x;
		position_.y = y;
		position_.z = z;
	}
	void SetX(float x) { position_.x = x; }
	void SetY(float y) { position_.y = y; }
	void SetZ(float z) { position_.z = z; }
	void SetAngle(D3DXVECTOR3 angle) { angle_ = angle; }
	void SetAngleXYZ(float angx = 0.0F, float angy = 0.0F, float angz = 0.0F)
	{
		angle_.x = angx;
		angle_.y = angy;
		angle_.z = angz;
	}
	void SetScale(D3DXVECTOR3 scale) { scale_ = scale; }
	void SetScaleXYZ(float sx = 1.0F, float sy = 1.0F, float sz = 1.0F)
	{
		scale_.x = sx;
		scale_.y = sy;
		scale_.z = sz;
	}

	void SetColor(D3DCOLOR color) { color_ = color; }
	void SetColorRGB(D3DCOLOR color);
	void SetAlpha(int alpha);

	bool IsCoordinate2D() const { return bCoordinate2D_; }
	void SetCoordinate2D(bool b) { bCoordinate2D_ = b; }

	gstd::ref_count_ptr<RenderBlocks> CreateRenderBlocks() { return nullptr; }
	virtual D3DXMATRIX GetAnimationMatrix(const std::wstring& nameAnime, double time, const std::wstring& nameBone)
	{
		D3DXMATRIX mat;
		D3DXMatrixIdentity(&mat);
		return mat;
	}
	gstd::ref_count_ptr<Shader> GetShader() { return shader_; }
	void SetShader(gstd::ref_count_ptr<Shader> shader) { shader_ = shader; }

protected:
	D3DXVECTOR3 position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
	D3DCOLOR color_;
	bool bCoordinate2D_; //2D座標指定
	gstd::ref_count_ptr<Shader> shader_;

	gstd::ref_count_ptr<DxMeshData> data_;
	gstd::ref_count_ptr<DxMeshData> _GetFromManager(const std::wstring& name);
	void _AddManager(std::wstring name, gstd::ref_count_ptr<DxMeshData> data);
};

/**********************************************************
//DxMeshManager
**********************************************************/
class DxMeshInfoPanel;
class DxMeshManager : public gstd::FileManager::LoadThreadListener {
	friend DxMeshData;
	friend DxMesh;
	friend DxMeshInfoPanel;

public:
	DxMeshManager();
	~DxMeshManager() override;
	static DxMeshManager* GetBase() { return thisBase_; }
	bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	virtual void Clear();
	virtual void Add(const std::wstring& name, gstd::ref_count_ptr<DxMesh> mesh); //参照を保持します
	virtual void Release(const std::wstring& name); //保持している参照を解放します
	virtual bool IsDataExists(const std::wstring& name);

	gstd::ref_count_ptr<DxMesh> CreateFromFileInLoadThread(const std::wstring& path, int type);
	void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event) override;

	void SetInfoPanel(gstd::ref_count_ptr<DxMeshInfoPanel> panel) { panelInfo_ = panel; }

protected:
	gstd::CriticalSection lock_;
	std::map<std::wstring, gstd::ref_count_ptr<DxMesh>> mapMesh_;
	std::map<std::wstring, gstd::ref_count_ptr<DxMeshData>> mapMeshData_;
	gstd::ref_count_ptr<DxMeshInfoPanel> panelInfo_;

	void _AddMeshData(const std::wstring& name, gstd::ref_count_ptr<DxMeshData> data);
	gstd::ref_count_ptr<DxMeshData> _GetMeshData(const std::wstring& name);
	void _ReleaseMeshData(const std::wstring& name);

private:
	static DxMeshManager* thisBase_;
};

class DxMeshInfoPanel : public gstd::WindowLogger::Panel, public gstd::Thread {
public:
	DxMeshInfoPanel();
	~DxMeshInfoPanel() override;
	void LocateParts() override;
	virtual void Update(DxMeshManager* manager);

protected:
	enum {
		ROW_ADDRESS,
		ROW_NAME,
		ROW_FULLNAME,
		ROW_COUNT_REFFRENCE,
	};
	int timeUpdateInterval_;
	gstd::WListView wndListView_;
	bool _AddedLogger(HWND hTab) override;
	void _Run() override;
};

} // namespace directx

#endif
