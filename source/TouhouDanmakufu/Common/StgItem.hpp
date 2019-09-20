#ifndef __TOUHOUDANMAKUFU_DNHSTG_ITEM__
#define __TOUHOUDANMAKUFU_DNHSTG_ITEM__

#include "StgCommon.hpp"
#include "StgIntersection.hpp"

class StgItemDataList;
class StgItemObject;
class StgItemData;
class StgItemRenderer;
/**********************************************************
//StgItemManager
**********************************************************/
class StgItemManager {
public:
	StgItemManager(StgStageController* stageController);
	virtual ~StgItemManager();
	void Work();
	void Render(int targetPriority);

	void AddItem(ref_count_ptr<StgItemObject>::unsync obj) { listObj_.push_back(obj); }
	int GetItemCount() const { return listObj_.size(); }

	SpriteList2D* GetItemRenderer() { return listSpriteItem_.GetPointer(); }
	SpriteList2D* GetDigitRenderer() { return listSpriteDigit_.GetPointer(); }
	std::vector<bool> GetValidRenderPriorityList();

	StgItemDataList* GetItemDataList() { return listItemData_.GetPointer(); }
	bool LoadItemData(const std::wstring& path, bool bReload = false);

	ref_count_ptr<StgItemObject>::unsync CreateItem(int type);

	void CollectItemsAll();
	void CollectItemsByType(int type);
	void CollectItemsInCircle(const DxCircle& circle);
	void CancelCollectItems();

	bool IsDefaultBonusItemEnable() const { return bDefaultBonusItemEnable_; }
	void SetDefaultBonusItemEnable(bool bEnable) { bDefaultBonusItemEnable_ = bEnable; }

private:
	StgStageController* stageController_;
	ref_count_ptr<SpriteList2D>::unsync listSpriteItem_;
	ref_count_ptr<SpriteList2D>::unsync listSpriteDigit_;
	ref_count_ptr<StgItemDataList>::unsync listItemData_;

	std::list<ref_count_ptr<StgItemObject>::unsync> listObj_;
	std::set<int> listItemTypeToPlayer_;
	std::list<DxCircle> listCircleToPlayer_;
	bool bAllItemToPlayer_;
	bool bCancelToPlayer_;
	bool bDefaultBonusItemEnable_;
};

/**********************************************************
//StgItemDataList
**********************************************************/
class StgItemDataList {
public:
	enum {
		RENDER_TYPE_COUNT = 3,
		RENDER_ALPHA = 0,
		RENDER_ADD_RGB,
		RENDER_ADD_ARGB,
	};

public:
	StgItemDataList();
	virtual ~StgItemDataList();

	int GetTextureCount() const { return listTexture_.size(); }
	ref_count_ptr<Texture> GetTexture(int index) { return listTexture_[index]; }
	ref_count_ptr<StgItemRenderer>::unsync GetRenderer(int index, int typeRender) const { return listRenderer_[typeRender][index]; }
	std::vector<ref_count_ptr<StgItemRenderer>::unsync>* GetRendererList(int typeRender) { return &listRenderer_[typeRender]; }

	ref_count_ptr<StgItemData>::unsync GetData(int id) const { return (id >= 0 && id < listData_.size()) ? listData_[id] : nullptr; }

	bool AddItemDataList(const std::wstring& path, bool bReload);

private:
	void _ScanItem(std::vector<ref_count_ptr<StgItemData>::unsync>& listData, Scanner& scanner);
	void _ScanAnimation(ref_count_ptr<StgItemData>::unsync itemData, Scanner& scanner);
	std::vector<std::wstring> _GetArgumentList(Scanner& scanner);

	std::set<std::wstring> listReadPath_;
	std::vector<ref_count_ptr<Texture>> listTexture_;
	std::vector<std::vector<ref_count_ptr<StgItemRenderer>::unsync>> listRenderer_;
	std::vector<ref_count_ptr<StgItemData>::unsync> listData_;

};

class StgItemData {
	friend StgItemDataList;

private:
	struct AnimationData {
		RECT rcSrc_;
		int frame_;
	};

public:
	StgItemData(StgItemDataList* listItemData);
	virtual ~StgItemData();

	int GetTextureIndex() const { return indexTexture_; }
	int GetItemType() const { return typeItem_; }
	int GetRenderType() const { return typeRender_; }
	RECT GetRect(int frame);
	RECT GetOut() const { return rcOut_; }
	int GetAlpha() const { return alpha_; }

	ref_count_ptr<Texture> GetTexture();
	StgItemRenderer* GetRenderer();
	StgItemRenderer* GetRenderer(int type);

private:
	StgItemDataList* listItemData_;
	int indexTexture_;

	int typeItem_;
	int typeRender_;
	RECT rcSrc_;
	RECT rcOut_;
	int alpha_;

	std::vector<AnimationData> listAnime_;
	int totalAnimeFrame_;
};

/**********************************************************
//StgItemRenderer
**********************************************************/
class StgItemRenderer : public RenderObjectTLX {
public:
	StgItemRenderer();
	int GetVertexCount() const override;
	void Render() override;
	void AddVertex(const VERTEX_TLX& vertex);
	void AddSquareVertex(const VERTEX_TLX* listVertex);

private:
	int countRenderVertex_;
};

/**********************************************************
//StgItemObject
**********************************************************/
class StgItemObject : public DxScriptRenderObject, public StgMoveObject, public StgIntersectionObject {
public:
	enum {
		ITEM_1UP = -256 * 256,
		ITEM_1UP_S,
		ITEM_SPELL,
		ITEM_SPELL_S,
		ITEM_POWER,
		ITEM_POWER_S,
		ITEM_POINT,
		ITEM_POINT_S,

		ITEM_SCORE,
		ITEM_BONUS,

		ITEM_USER = 0,
	};

public:
	StgItemObject(StgStageController* stageController);
	void Work() override;
	void Render() override {} //一括で描画するためオブジェクト管理での描画はしない
	virtual void RenderOnItemManager(D3DXMATRIX mat);
	void SetRenderState() override {}
	virtual void Activate() {}

	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override = 0;

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
	void SetToPosition(POINT pos);

	int64_t GetScore() const { return score_; }
	void SetScore(int64_t score) { score_ = score; }
	bool IsMoveToPlayer() const { return bMoveToPlayer_; }
	void SetMoveToPlayer(bool b) { bMoveToPlayer_ = b; }
	bool IsPermitMoveToPlayer() const { return bPermitMoveToPlayer_; }
	void SetPermitMoveToPlayer(bool bPermit) { bPermitMoveToPlayer_ = bPermit; }
	void SetChangeItemScore(bool b) { bChangeItemScore_ = b; }

	int GetMoveType();
	void SetMoveType(int type);

	int GetItemType() const { return typeItem_; }
	void SetItemType(int type) { typeItem_ = type; }
	StgStageController* GetStageController() { return stageController_; }

protected:
	void _DeleteInAutoClip();
	void _CreateScoreItem();
	void _NotifyEventToPlayerScript(const std::vector<long double>& listValue);
	void _NotifyEventToItemScript(const std::vector<long double>& listValue);

	StgStageController* stageController_;
	int typeItem_;
	D3DCOLOR color_;

	int64_t score_;
	bool bMoveToPlayer_; //自機移動フラグ
	bool bPermitMoveToPlayer_; //自機自動回収許可
	bool bChangeItemScore_;
};

class StgItemObject_1UP : public StgItemObject {
public:
	StgItemObject_1UP(StgStageController* stageController);
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
};

class StgItemObject_Bomb : public StgItemObject {
public:
	StgItemObject_Bomb(StgStageController* stageController);
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
};

class StgItemObject_Power : public StgItemObject {
public:
	StgItemObject_Power(StgStageController* stageController);
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
};

class StgItemObject_Point : public StgItemObject {
public:
	StgItemObject_Point(StgStageController* stageController);
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
};

class StgItemObject_Bonus : public StgItemObject {
public:
	StgItemObject_Bonus(StgStageController* stageController);
	void Work() override;
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;
};

class StgItemObject_Score : public StgItemObject {
public:
	StgItemObject_Score(StgStageController* stageController);
	void Work() override;
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;

private:
	int frameDelete_;
};

class StgItemObject_User : public StgItemObject {
public:
	StgItemObject_User(StgStageController* stageController);
	void Work() override;
	void RenderOnItemManager(D3DXMATRIX mat) override;
	void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) override;

	void SetImageID(int id);

private:
	StgItemData* _GetItemData();
	void _SetVertexPosition(VERTEX_TLX& vertex, float x, float y, float z = 1.0F, float w = 1.0F);
	void _SetVertexUV(VERTEX_TLX& vertex, float u, float v);
	void _SetVertexColorARGB(VERTEX_TLX& vertex, D3DCOLOR color);

	int frameWork_;
	int idImage_;
};

/**********************************************************
//StgMovePattern_Item
**********************************************************/
class StgMovePattern_Item : public StgMovePattern {
public:
	enum {
		MOVE_NONE,
		MOVE_TOPOSITION_A, //指定ポイントへの移動(60フレーム)
		MOVE_DOWN, //下降
		MOVE_TOPLAYER, //自機へ移動
		MOVE_SCORE, //得点(上昇)
	};

public:
	StgMovePattern_Item(StgMoveObject* target);
	void Move() override;
	int GetType() { return TYPE_OTHER; }
	double GetSpeed() const override { return 0; }
	double GetDirectionAngle() const override { return 0; }
	void SetToPosition(POINT pos) { posTo_ = pos; }

	int GetItemMoveType() const { return typeMove_; }
	void SetItemMoveType(int type) { typeMove_ = type; }

protected:
	int frame_;
	int typeMove_;
	double speed_;
	double angDirection_;

	POINT posTo_;
};

#endif
