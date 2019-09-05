#ifndef __TOUHOUDANMAKUFU_DNHSTG_INTERSECTION__
#define __TOUHOUDANMAKUFU_DNHSTG_INTERSECTION__

#include "StgCommon.hpp"

class StgIntersectionSpace;
class StgIntersectionCheckList;
class StgIntersectionTarget;
class StgIntersectionObject;
class StgIntersectionTargetPoint;

/**********************************************************
//StgIntersectionManager
//下記を参考
//http://marupeke296.com/COL_2D_No8_QuadTree.html
**********************************************************/
class StgIntersectionManager : public ObjectPool<StgIntersectionTarget, false> {
	enum {
		SPACE_PLAYER_ENEMY = 0, //自機-敵、敵弾
		SPACE_PLAYERSOHT_ENEMY, //自弾,スペル-敵
		SPACE_PLAYERSHOT_ENEMYSHOT, //自弾,スペル-敵弾
	};

public:
	StgIntersectionManager();
	~StgIntersectionManager() override;
	void Work();

	void AddTarget(ref_count_ptr<StgIntersectionTarget>::unsync target);
	void AddEnemyTargetToShot(ref_count_ptr<StgIntersectionTarget>::unsync target);
	void AddEnemyTargetToPlayer(ref_count_ptr<StgIntersectionTarget>::unsync target);
	std::vector<StgIntersectionTargetPoint>* GetAllEnemyTargetPoint() { return &listEnemyTargetPoint_; }

	void CheckDeletedObject(std::string funcName);

	static bool IsIntersected(ref_count_ptr<StgIntersectionTarget>::unsync& target1, ref_count_ptr<StgIntersectionTarget>::unsync& target2);
	static bool IsIntersected(const DxCircle& circle, const DxWidthLine& line);
	static bool IsIntersected(const DxWidthLine& line1, const DxWidthLine& line2);

private:
	void _ResetPoolObject(gstd::ref_count_ptr<StgIntersectionTarget>::unsync& obj) override;
	gstd::ref_count_ptr<StgIntersectionTarget>::unsync _CreatePoolObject(int type) override;

	std::vector<ref_count_ptr<StgIntersectionSpace>> listSpace_;
	std::vector<StgIntersectionTargetPoint> listEnemyTargetPoint_;
	std::vector<StgIntersectionTargetPoint> listEnemyTargetPointNext_;
};

/**********************************************************
//StgIntersectionSpace
//以下サイトを参考
//　○×（まるぺけ）つくろーどっとコム
//　http://marupeke296.com/
**********************************************************/
class StgIntersectionSpace {
	enum {
		MAX_LEVEL = 9,
		TYPE_A = 0,
		TYPE_B = 1,
	};

public:
	StgIntersectionSpace();
	virtual ~StgIntersectionSpace();
	bool Initialize(int level, int left, int top, int right, int bottom);
	bool RegistTarget(int type, ref_count_ptr<StgIntersectionTarget>::unsync& target);
	bool RegistTargetA(ref_count_ptr<StgIntersectionTarget>::unsync& target) { return RegistTarget(TYPE_A, target); }
	bool RegistTargetB(ref_count_ptr<StgIntersectionTarget>::unsync& target) { return RegistTarget(TYPE_B, target); }
	void ClearTarget();
	ref_count_ptr<StgIntersectionCheckList>::unsync CreateIntersectionCheckList();

protected:
	unsigned int _GetMortonNumber(float left, float top, float right, float bottom);
	unsigned int _BitSeparate32(unsigned int n);
	unsigned short _Get2DMortonNumber(unsigned short x, unsigned short y);
	unsigned int _GetPointElem(float pos_x, float pos_y);
	void _WriteIntersectionCheckList(int indexSpace, ref_count_ptr<StgIntersectionCheckList>::unsync& listCheck, std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync>>& listStack);

	// Cell TARGETA/B listTarget
	std::vector<std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync>>> listCell_;
	int listCountLevel_[MAX_LEVEL + 1]; // 各レベルのセル数
	double spaceWidth_; // 領域のX軸幅
	double spaceHeight_; // 領域のY軸幅
	double spaceLeft_; // 領域の左側（X軸最小値）
	double spaceTop_; // 領域の上側（Y軸最小値）
	double unitWidth_; // 最小レベル空間の幅単位
	double unitHeight_; // 最小レベル空間の高単位
	int countCell_; // 空間の数
	int unitLevel_; // 最下位レベル
	ref_count_ptr<StgIntersectionCheckList>::unsync listCheck_;
};

class StgIntersectionCheckList {
public:
	StgIntersectionCheckList() { count_ = 0; }
	virtual ~StgIntersectionCheckList() = default;

	void Clear() { count_ = 0; }
	int GetCheckCount() const { return count_; }
	void Add(ref_count_ptr<StgIntersectionTarget>::unsync& targetA, ref_count_ptr<StgIntersectionTarget>::unsync& targetB)
	{
		if (listTargetA_.size() <= count_) {
			listTargetA_.push_back(targetA);
			listTargetB_.push_back(targetB);
		} else {
			listTargetA_[count_] = targetA;
			listTargetB_[count_] = targetB;
		}
		++count_;
	}
	ref_count_ptr<StgIntersectionTarget>::unsync GetTargetA(int index)
	{
		ref_count_ptr<StgIntersectionTarget>::unsync res = listTargetA_[index];
		listTargetA_[index] = nullptr;
		return res;
	}
	ref_count_ptr<StgIntersectionTarget>::unsync GetTargetB(int index)
	{
		ref_count_ptr<StgIntersectionTarget>::unsync res = listTargetB_[index];
		listTargetB_[index] = nullptr;
		return res;
	}

private:
	int count_;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> listTargetA_;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> listTargetB_;
};

class StgIntersectionObject {
public:
	StgIntersectionObject()
	{
		bIntersected_ = false;
		intersectedCount_ = 0;
	}
	virtual ~StgIntersectionObject() = default;
	virtual void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) = 0;
	void ClearIntersected()
	{
		bIntersected_ = false;
		intersectedCount_ = 0;
	}
	bool IsIntersected() const { return bIntersected_; }
	void SetIntersected()
	{
		bIntersected_ = true;
		++intersectedCount_;
	}
	int GetIntersectedCount() const { return intersectedCount_; }
	void ClearIntersectedIdList()
	{
		if (!listIntersectedID_.empty())
			listIntersectedID_.clear();
	}
	void AddIntersectedId(int id) { listIntersectedID_.push_back(id); }
	std::vector<int>& GetIntersectedIdList() { return listIntersectedID_; }

	void ClearIntersectionRelativeTarget();
	void AddIntersectionRelativeTarget(ref_count_ptr<StgIntersectionTarget>::unsync target);
	ref_count_ptr<StgIntersectionTarget>::unsync GetIntersectionRelativeTarget(int index) const { return listRelativeTarget_[index]; }

	void UpdateIntersectionRelativeTarget(int posX, int posY, double angle);
	void RegistIntersectionRelativeTarget(StgIntersectionManager* manager);
	int GetIntersectionRelativeTargetCount() const { return listRelativeTarget_.size(); }
	int GetDxScriptObjectID();

	virtual std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> GetIntersectionTargetList() { return std::vector<ref_count_ptr<StgIntersectionTarget>::unsync>(); }

protected:
	bool bIntersected_; //衝突判定
	int intersectedCount_;
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync> listRelativeTarget_;
	std::vector<DxCircle> listOrgCircle_;
	std::vector<DxWidthLine> listOrgLine_;
	std::vector<int> listIntersectedID_;
};

/**********************************************************
//StgIntersectionTarget
**********************************************************/
class StgIntersectionTarget : public IStringInfo {
	friend StgIntersectionManager;

public:
	enum {
		SHAPE_CIRCLE = 0,
		SHAPE_LINE = 1,

		TYPE_PLAYER,
		TYPE_PLAYER_SHOT,
		TYPE_PLAYER_SPELL,
		TYPE_ENEMY,
		TYPE_ENEMY_SHOT,
	};

public:
	StgIntersectionTarget() { mortonNo_ = -1; }
	~StgIntersectionTarget() override = default;
	virtual RECT GetIntersectionSapceRect() = 0;

	int GetTargetType() const { return typeTarget_; }
	void SetTargetType(int type) { typeTarget_ = type; }
	int GetShape() const { return shape_; }
	ref_count_weak_ptr<StgIntersectionObject>::unsync GetObject() const { return obj_; }
	void SetObject(ref_count_weak_ptr<StgIntersectionObject>::unsync obj) { obj_ = obj; }

	int GetMortonNumber() const { return mortonNo_; }
	void SetMortonNumber(int no) { mortonNo_ = no; }
	void ClearObjectIntersectedIdList();

	std::wstring GetInfoAsString() override;

protected:
	int mortonNo_;
	int typeTarget_;
	int shape_;
	ref_count_weak_ptr<StgIntersectionObject>::unsync obj_;
};

class StgIntersectionTarget_Circle : public StgIntersectionTarget {
	friend StgIntersectionManager;

public:
	StgIntersectionTarget_Circle() { shape_ = SHAPE_CIRCLE; }
	~StgIntersectionTarget_Circle() override = default;
	RECT GetIntersectionSapceRect() override
	{
		DirectGraphics* graphics = DirectGraphics::GetBase();
		int screenWidth = graphics->GetScreenWidth();
		int screenHeight = graphics->GetScreenWidth();

		double x = circle_.GetX();
		double y = circle_.GetY();
		double r = circle_.GetR();
		RECT rect = { (int)(x - r), (int)(y - r), (int)(x + r), (int)(y + r) };
		rect.left = max(rect.left, 0);
		rect.left = min(rect.left, screenWidth);
		rect.top = max(rect.top, 0);
		rect.top = min(rect.top, screenHeight);

		rect.right = max(rect.right, 0);
		rect.right = min(rect.right, screenWidth);
		rect.bottom = max(rect.bottom, 0);
		rect.bottom = min(rect.bottom, screenHeight);
		return rect;
	}

	DxCircle& GetCircle() { return circle_; }
	void SetCircle(const DxCircle& circle) { circle_ = circle; }

private:
	DxCircle circle_;
};

class StgIntersectionTarget_Line : public StgIntersectionTarget {
	friend StgIntersectionManager;

protected:
	StgIntersectionTarget_Line() { shape_ = SHAPE_LINE; }

public:
	~StgIntersectionTarget_Line() override = default;
	RECT GetIntersectionSapceRect() override
	{
		double x1 = line_.GetX1();
		double y1 = line_.GetY1();
		double x2 = line_.GetX2();
		double y2 = line_.GetY2();
		double width = line_.GetWidth();
		if (x1 > x2) {
			double tx = x1;
			x1 = x2;
			x2 = tx;
		}
		if (y1 > y2) {
			double ty = y1;
			y1 = y2;
			y2 = ty;
		}

		x1 -= width;
		x2 += width;
		y1 -= width;
		y2 += width;

		DirectGraphics* graphics = DirectGraphics::GetBase();
		int screenWidth = graphics->GetScreenWidth();
		int screenHeight = graphics->GetScreenWidth();
		x1 = min(x1, screenWidth);
		x1 = max(x1, 0);
		x2 = min(x2, screenWidth);
		x2 = max(x2, 0);

		y1 = min(y1, screenHeight);
		y1 = max(y1, 0);
		y2 = min(y2, screenHeight);
		y2 = max(y2, 0);

		// RECT rect = {x1 - width, y1 - width, x2 + width, y2 + width};
		RECT rect = { (int)x1, (int)y1, (int)x2, (int)y2 };
		return rect;
	}

	DxWidthLine& GetLine() { return line_; }
	void SetLine(DxWidthLine& line) { line_ = line; }

private:
	DxWidthLine line_;
};

/**********************************************************
//StgIntersectionTargetPoint
**********************************************************/
class StgIntersectionTargetPoint {
public:
	POINT GetPoint() { return pos_; }
	void SetPoint(POINT pos) { pos_ = pos; }
	int GetObjectID() const { return idObject_; }
	void SetObjectID(int id) { idObject_ = id; }

private:
	POINT pos_;
	int idObject_;
};

#endif
