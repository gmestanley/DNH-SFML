#ifndef __TOUHOUDANMAKUFU_DNHSTG_SHOT__
#define __TOUHOUDANMAKUFU_DNHSTG_SHOT__

#include "StgCommon.hpp"
#include "StgIntersection.hpp"

class StgShotDataList;
class StgShotData;
class StgShotRenderer;
class StgShotObject;
/**********************************************************
//StgShotManager
**********************************************************/
class StgShotManager {
public:
	enum {
		DEL_TYPE_ALL,
		DEL_TYPE_SHOT,
		DEL_TYPE_CHILD,
		TO_TYPE_IMMEDIATE,
		TO_TYPE_FADE,
		TO_TYPE_ITEM,
	};

	enum {
		BIT_EV_DELETE_IMMEDIATE = 1,
		BIT_EV_DELETE_TO_ITEM,
		BIT_EV_DELETE_FADE,
		BIT_EV_DELETE_COUNT,
	};

public:
	StgShotManager(StgStageController* stageController);
	virtual ~StgShotManager();
	void Work();
	void Render(int targetPriority);
	void RegistIntersectionTarget();

	void AddShot(ref_count_ptr<StgShotObject>::unsync obj) { listObj_.push_back(obj); }

	StgShotDataList* GetPlayerShotDataList() { return listPlayerShotData_.GetPointer(); }
	StgShotDataList* GetEnemyShotDataList() { return listEnemyShotData_.GetPointer(); }

	bool LoadPlayerShotData(const std::wstring& path, bool bReload = false);
	bool LoadEnemyShotData(const std::wstring& path, bool bReload = false);

	RECT GetShotAutoDeleteClipRect();

	void DeleteInCircle(int typeDelete, int typeTo, int typeOnwer, int cx, int cy, double radius);
	std::vector<int> GetShotIdInCircle(int typeOnwer, int cx, int cy, int radius);
	int GetShotCount(int typeOwner);
	int GetShotCountAll() const { return listObj_.size(); }
	std::vector<bool> GetValidRenderPriorityList();

	void SetDeleteEventEnableByType(int type, bool bEnable);
	bool IsDeleteEventEnable(int bit) const;

protected:
	StgStageController* stageController_;
	ref_count_ptr<StgShotDataList>::unsync listPlayerShotData_;
	ref_count_ptr<StgShotDataList>::unsync listEnemyShotData_;
	std::list<ref_count_ptr<StgShotObject>::unsync> listObj_;

	std::bitset<BIT_EV_DELETE_COUNT> listDeleteEventEnable_;
};

/**********************************************************
//StgShotDataList
**********************************************************/
class StgShotDataList {
public:
	enum {
		RENDER_TYPE_COUNT = 6,
		RENDER_ALPHA = 0,
		RENDER_ADD_RGB,
		RENDER_ADD_ARGB,
		RENDER_MULTIPLY,
		RENDER_SUBTRACT,
		RENDER_INV_DESTRGB,
	};

public:
	StgShotDataList();
	virtual ~StgShotDataList();

	int GetTextureCount() const { return listTexture_.size(); }
	ref_count_ptr<Texture> GetTexture(int index) { return listTexture_[index]; }
	ref_count_ptr<StgShotRenderer>::unsync GetRenderer(int index, int typeRender) const { return listRenderer_[typeRender][index]; }
	std::vector<ref_count_ptr<StgShotRenderer>::unsync>* GetRendererList(int typeRender) { return &listRenderer_[typeRender]; }

	ref_count_ptr<StgShotData>::unsync GetData(int id) const { return (id >= 0 && id < listData_.size()) ? listData_[id] : nullptr; }

	bool AddShotDataList(const std::wstring& path, bool bReload);

private:
	void _ScanShot(std::vector<ref_count_ptr<StgShotData>::unsync>& listData, Scanner& scanner);
	void _ScanAnimation(ref_count_ptr<StgShotData>::unsync shotData, Scanner& scanner);
	std::vector<std::wstring> _GetArgumentList(Scanner& scanner);

	std::set<std::wstring> listReadPath_;
	std::vector<ref_count_ptr<Texture>> listTexture_;
	std::vector<std::vector<ref_count_ptr<StgShotRenderer>::unsync>> listRenderer_;
	std::vector<ref_count_ptr<StgShotData>::unsync> listData_;

	D3DCOLOR defaultDelayColor_;
};

class StgShotData {
	friend StgShotDataList;
	struct AnimationData {
		RECT rcSrc_;
		int frame_;
	};

public:
	StgShotData(StgShotDataList* listShotData);
	virtual ~StgShotData();

	int GetTextureIndex() const { return indexTexture_; }
	int GetRenderType() const { return typeRender_; }
	int GetDelayRenderType() const { return typeDelayRender_; }
	RECT GetRect(int frame);
	RECT GetDelayRect() const { return rcDelay_; }
	int GetAlpha() const { return alpha_; }
	D3DCOLOR GetDelayColor() const { return colorDelay_; }
	std::vector<DxCircle>* GetIntersectionCircleList() { return &listCol_; }
	double GetAngularVelocityMin() const { return angularVelocityMin_; }
	double GetAngularVelocityMax() const { return angularVelocityMax_; }
	bool IsFixedAngle() const { return bFixedAngle_; }

	ref_count_ptr<Texture> GetTexture();
	StgShotRenderer* GetRenderer();
	StgShotRenderer* GetRenderer(int type);
	StgShotRenderer* GetRendererFromGraphicsBlendType(int blendType);
	static bool IsAlphaBlendValidType(int blendType);

private:
	StgShotDataList* listShotData_;
	int indexTexture_;
	int typeRender_;
	int typeDelayRender_;
	RECT rcSrc_;
	RECT rcDelay_;
	int alpha_;
	D3DCOLOR colorDelay_;
	std::vector<DxCircle> listCol_;

	std::vector<AnimationData> listAnime_;
	int totalAnimeFrame_;

	double angularVelocityMin_;
	double angularVelocityMax_;
	bool bFixedAngle_;
};

/**********************************************************
//StgShotRenderer
**********************************************************/
class StgShotRenderer : public RenderObjectTLX {
public:
	StgShotRenderer();
	int GetVertexCount() const override;
	void Render() override;
	void AddVertex(VERTEX_TLX& vertex);
	void AddSquareVertex(VERTEX_TLX* listVertex);

private:
	int countRenderVertex_;
};

/**********************************************************
//StgShotObject
**********************************************************/
class StgShotObject : public DxScriptRenderObject, public StgMoveObject, public StgIntersectionObject {
public:
	enum {
		OWNER_PLAYER = 0,
		OWNER_ENEMY,
		OWNER_NULL,

		FRAME_FADEDELETE = 30,
		FRAME_FADEDELETE_LASER = 30,

		LIFE_SPELL_UNREGIST = 5,
		LIFE_SPELL_REGIST = 256 * 256 * 256,
	};
	class ReserveShotList;
	class ReserveShotListData;

public:
	StgShotObject(StgStageController* stageController);
	~StgShotObject() override;

	void Work() override;
	void Render() override {} //一括で描画するためオブジェクト管理での描画はしない
	virtual void Activate() {}
	virtual void RenderOnShotManager(D3DXMATRIX mat) {}
	double cssn(double s, double ang);
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
	virtual void ClearShotObject() { ClearIntersectionRelativeTarget(); }
	virtual void RegistIntersectionTarget() = 0;

	void SetX(double x) override
	{
		posX_ = x;
		DxScriptRenderObject::SetX(x);
	}
	void SetY(double y) override
	{
		posY_ = y;
		DxScriptRenderObject::SetY(y);
	}
	void SetColor(int r, int g, int b) override;
	void SetAlpha(int alpha) override;
	void SetRenderState() override {}

	ref_count_ptr<StgShotObject>::unsync GetOwnObject();
	int GetShotDataID() const { return idShotData_; }
	virtual void SetShotDataID(int id) { idShotData_ = id; }
	int GetOwnerType() const { return typeOwner_; }
	void SetOwnerType(int type) { typeOwner_ = type; }

	bool IsValidGraze() const { return frameGrazeInvalid_ <= 0; }
	int GetDelay() const { return delay_; }
	void SetDelay(int delay) { delay_ = delay; }
	int GetSourceBlendType() const { return typeSourceBrend_; }
	void SetSourceBlendType(int type) { typeSourceBrend_ = type; }
	double GetLife() const { return life_; }
	void SetLife(double life) { life_ = life; }
	double GetDamage() const { return damage_; }
	void SetDamage(double damage) { damage_ = damage; }
	virtual void SetFadeDelete()
	{
		if (frameFadeDelete_ < 0)
			frameFadeDelete_ = FRAME_FADEDELETE;
	}
	bool IsAutoDelete() const { return bAutoDelete_; }
	void SetAutoDelete(bool b) { bAutoDelete_ = b; }
	void SetAutoDeleteFrame(int frame) { frameAutoDelete_ = frame; }
	bool IsEraseShot() const { return bEraseShot_; }
	void SetEraseShot(bool bErase) { bEraseShot_ = bErase; }
	bool IsSpellFactor() const { return bSpellFactor_; }
	void SetSpellFactor(bool bSpell) { bSpellFactor_ = bSpell; }
	void SetUserIntersectionMode(bool b) { bUserIntersectionMode_ = b; }
	void SetIntersectionEnable(bool b) { bIntersectionEnable_ = b; }
	void SetItemChangeEnable(bool b) { bChangeItemEnable_ = b; }

	virtual void DeleteImmediate();
	virtual void ConvertToItem();
	void AddShot(int frame, int idShot, int radius, int angle);

protected:
	StgStageController* stageController_;

	int frameWork_;
	int idShotData_;
	int typeOwner_;

	D3DCOLOR color_;
	int delay_; //遅延時間
	int typeSourceBrend_; //遅延時間ブレンド種別
	int frameGrazeInvalid_; //かすり無効フレーム
	int frameFadeDelete_;
	double damage_;
	double life_; //貫通力
	bool bAutoDelete_;
	bool bEraseShot_; //弾削除機能
	bool bSpellFactor_; //スペル付加
	int frameAutoDelete_; //自動削除フレーム
	ref_count_ptr<ReserveShotList>::unsync listReserveShot_;

	bool bUserIntersectionMode_; //ユーザ定義あたり判定モード
	bool bIntersectionEnable_;
	bool bChangeItemEnable_;

	StgShotData* _GetShotData();
	void _SetVertexPosition(VERTEX_TLX& vertex, float x, float y, float z = 1.0F, float w = 1.0F);
	void _SetVertexUV(VERTEX_TLX& vertex, float u, float v);
	void _SetVertexColorARGB(VERTEX_TLX& vertex, D3DCOLOR color);
	virtual void _DeleteInLife();
	virtual void _DeleteInAutoClip();
	virtual void _DeleteInFadeDelete();
	virtual void _DeleteInAutoDeleteFrame();
	void _Move() override;
	void _AddReservedShotWork();
	virtual void _AddReservedShot(ref_count_ptr<StgShotObject>::unsync obj, ReserveShotListData* data);
	virtual void _ConvertToItemAndSendEvent() {}
	virtual void _SendDeleteEvent(int bit);
};

class StgShotObject::ReserveShotListData {
	friend ReserveShotList;

public:
	ReserveShotListData()
	{
		idShot_ = DxScript::ID_INVALID;
		radius_ = 0;
		angle_ = 0;
	}
	virtual ~ReserveShotListData() = default;
	int GetShotID() const { return idShot_; }
	double GetRadius() const { return radius_; }
	double GetAngle() const { return angle_; }

private:
	int idShot_; //対象ID
	double radius_; //出現位置距離
	double angle_; //出現位置角度
};

class StgShotObject::ReserveShotList {
public:
	class ListElement {
		std::list<ReserveShotListData> list_;

	public:
		ListElement() = default;
		virtual ~ListElement() = default;
		void Add(ReserveShotListData& data) { list_.push_back(data); }
		std::list<ReserveShotListData>* GetDataList() { return &list_; }
	};

public:
	ReserveShotList() { frame_ = 0; }
	virtual ~ReserveShotList() = default;
	ref_count_ptr<ListElement>::unsync GetNextFrameData();
	void AddData(int frame, int idShot, int radius, int angle);
	void Clear(StgStageController* stageController);

private:
	int frame_;
	std::map<int, ref_count_ptr<ListElement>::unsync> mapData_;
};

/**********************************************************
//StgNormalShotObject
**********************************************************/
class StgNormalShotObject : public StgShotObject {
public:
	StgNormalShotObject(StgStageController* stageController);
	~StgNormalShotObject() override;
	void Work() override;
	void RenderOnShotManager(D3DXMATRIX mat) override;
	void ClearShotObject() override;
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;

	void RegistIntersectionTarget() override;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> GetIntersectionTargetList() override;
	void SetShotDataID(int id) override;

protected:
	void _AddIntersectionRelativeTarget();
	void _ConvertToItemAndSendEvent() override;
	double angularVelocity_;
};

/**********************************************************
//StgLaserObject(レーザー基本部)
**********************************************************/
class StgLaserObject : public StgShotObject {
public:
	StgLaserObject(StgStageController* stageController);
	void ClearShotObject() override;
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;

	int GetLength() const { return length_; }
	void SetLength(int length);
	int GetRenderWidth() const { return widthRender_; }
	void SetRenderWidth(int width);
	int GetIntersectionWidth() const { return widthIntersection_; }
	void SetIntersectionWidth(int width) { widthIntersection_ = width; }
	void SetInvalidLength(int start, int end)
	{
		invalidLengthStart_ = start;
		invalidLengthEnd_ = end;
	}
	void SetGrazeInvalidFrame(int frame) { frameGrazeInvalidStart_ = frame; }
	void SetItemDistance(double dist) { itemDistance_ = dist; }

protected:
	void _AddIntersectionRelativeTarget();
	int length_;
	int widthRender_;
	int widthIntersection_;
	int invalidLengthStart_;
	int invalidLengthEnd_;
	int frameGrazeInvalidStart_;
	double itemDistance_;
};

/**********************************************************
//StgLooseLaserObject(射出型レーザー)
**********************************************************/
class StgLooseLaserObject : public StgLaserObject {
public:
	StgLooseLaserObject(StgStageController* stageController);
	void Work() override;
	void RenderOnShotManager(D3DXMATRIX mat) override;

	void RegistIntersectionTarget() override;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> GetIntersectionTargetList() override;
	void SetX(double x) override
	{
		StgShotObject::SetX(x);
		posXE_ = x;
	}
	void SetY(double y) override
	{
		StgShotObject::SetY(y);
		posYE_ = y;
	}

protected:
	void _DeleteInAutoClip() override;
	void _Move() override;
	void _ConvertToItemAndSendEvent() override;

	double posXE_; //後方x
	double posYE_; //後方y
};

/**********************************************************
//StgStraightLaserObject(設置型レーザー)
**********************************************************/
class StgStraightLaserObject : public StgLaserObject {
public:
	StgStraightLaserObject(StgStageController* stageController);
	void Work() override;
	void RenderOnShotManager(D3DXMATRIX mat) override;
	void RegistIntersectionTarget() override;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> GetIntersectionTargetList() override;

	double GetLaserAngle() const { return angLaser_; }
	void SetLaserAngle(double angle) { angLaser_ = angle; }
	void SetFadeDelete() override
	{
		if (frameFadeDelete_ < 0)
			frameFadeDelete_ = FRAME_FADEDELETE_LASER;
	}
	void SetSourceEnable(bool bEnable) { bUseSouce_ = bEnable; }

protected:
	void _DeleteInAutoClip() override;
	void _DeleteInAutoDeleteFrame() override;
	void _AddReservedShot(ref_count_ptr<StgShotObject>::unsync obj, ReserveShotListData* data) override;
	void _ConvertToItemAndSendEvent() override;

	double angLaser_;
	bool bUseSouce_;
	double scaleX_;
};

/**********************************************************
//StgCurveLaserObject(曲がる型レーザー)
**********************************************************/
class StgCurveLaserObject : public StgLaserObject {
protected:
	struct Position {
		double x;
		double y;
	};

public:
	StgCurveLaserObject(StgStageController* stageController);
	void Work() override;
	void RenderOnShotManager(D3DXMATRIX mat) override;
	void RegistIntersectionTarget() override;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> GetIntersectionTargetList() override;
	void SetTipDecrement(double dec) { tipDecrement_ = dec; }

protected:
	std::list<Position> listPosition_;
	double tipDecrement_;

	void _DeleteInAutoClip() override;
	void _Move() override;
	void _ConvertToItemAndSendEvent() override;
};

#endif
