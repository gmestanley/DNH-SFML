#ifndef __TOUHOUDANMAKUFU_DNHSTG_ENEMY__
#define __TOUHOUDANMAKUFU_DNHSTG_ENEMY__

#include "StgCommon.hpp"
#include "StgIntersection.hpp"

class StgEnemyObject;
class StgEnemyBossSceneObject;
/**********************************************************
//StgEnemyManager
**********************************************************/
class StgEnemyManager {
public:
	StgEnemyManager(StgStageController* stageController);
	virtual ~StgEnemyManager();
	void Work();
	void RegistIntersectionTarget();

	void AddEnemy(ref_count_ptr<StgEnemyObject>::unsync obj) { listObj_.push_back(obj); }
	int GetEnemyCount() const { return listObj_.size(); }

	void SetBossSceneObject(ref_count_ptr<StgEnemyBossSceneObject>::unsync obj);
	ref_count_ptr<StgEnemyBossSceneObject>::unsync GetBossSceneObject();
	std::list<ref_count_ptr<StgEnemyObject>::unsync>& GetEnemyList() { return listObj_; }

private:
	StgStageController* stageController_;
	std::list<ref_count_ptr<StgEnemyObject>::unsync> listObj_;
	ref_count_ptr<StgEnemyBossSceneObject>::unsync objBossScene_;
};

/**********************************************************
//StgEnemyObject
**********************************************************/
class StgEnemyObject : public DxScriptSpriteObject2D, public StgMoveObject, public StgIntersectionObject {
public:
	StgEnemyObject(StgStageController* stageController);
	virtual ~StgEnemyObject();

	virtual void Work();
	virtual void Activate();
	virtual void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget);
	virtual void ClearEnemyObject() { ClearIntersectionRelativeTarget(); }
	virtual void RegistIntersectionTarget();

	virtual void SetX(double x)
	{
		posX_ = x;
		DxScriptRenderObject::SetX(x);
	}
	virtual void SetY(double y)
	{
		posY_ = y;
		DxScriptRenderObject::SetY(y);
	}

	ref_count_ptr<StgEnemyObject>::unsync GetOwnObject();
	double GetLife() const { return life_; }
	void SetLife(double life) { life_ = life; }
	void AddLife(double inc)
	{
		life_ += inc;
		life_ = max(life_, 0);
	}
	void SetDamageRate(double rateShot, double rateSpell)
	{
		rateDamageShot_ = rateShot;
		rateDamageSpell_ = rateSpell;
	}
	double GetShotDamageRate() const { return rateDamageShot_; }
	double GetSpellDamageRate() const { return rateDamageSpell_; }
	int GetIntersectedPlayerShotCount() const { return intersectedPlayerShotCount_; }

protected:
	virtual void _Move();
	virtual void _AddRelativeIntersection();

	StgStageController* stageController_;

	double life_;
	double rateDamageShot_;
	double rateDamageSpell_;
	int intersectedPlayerShotCount_;
};

/**********************************************************
//StgEnemyBossObject
**********************************************************/
class StgEnemyBossObject : public StgEnemyObject {
public:
	StgEnemyBossObject(StgStageController* stageController);

private:
	int timeSpellCard_;
};

/**********************************************************
//StgEnemyBossSceneObject
**********************************************************/
class StgEnemyBossSceneData;
class StgEnemyBossSceneObject : public DxScriptObjectBase {
public:
	StgEnemyBossSceneObject(StgStageController* stageController);
	virtual void Work();
	virtual void Activate();
	virtual void Render() {} //何もしない
	virtual void SetRenderState() {} //何もしない

	void AddData(int step, ref_count_ptr<StgEnemyBossSceneData>::unsync data);
	ref_count_ptr<StgEnemyBossSceneData>::unsync GetActiveData() const { return activeData_; }
	void LoadAllScriptInThread();

	int GetRemainStepCount() const;
	int GetActiveStepLifeCount() const;
	double GetActiveStepTotalMaxLife() const;
	double GetActiveStepTotalLife() const;
	double GetActiveStepLife(int index) const;
	std::vector<double> GetActiveStepLifeRateList() const;
	int GetDataStep() const { return dataStep_; }
	int GetDataIndex() const { return dataIndex_; }

	void AddPlayerShootDownCount();
	void AddPlayerSpellCount();

private:
	bool _NextStep();

	StgStageController* stageController_;
	volatile bool bLoad_;

	int dataStep_;
	int dataIndex_;
	ref_count_ptr<StgEnemyBossSceneData>::unsync activeData_;
	std::vector<std::vector<ref_count_ptr<StgEnemyBossSceneData>::unsync>> listData_;
};

class StgEnemyBossSceneData {
public:
	StgEnemyBossSceneData();
	virtual ~StgEnemyBossSceneData() {}
	std::wstring GetPath() const { return path_; }
	void SetPath(const std::wstring& path) { path_ = path; }
	_int64 GetScriptID() const { return isScript_; }
	void SetScriptID(_int64 id) { isScript_ = id; }
	std::vector<double>& GetLifeList() { return listLife_; }
	void SetLifeList(const std::vector<double>& list) { listLife_ = list; }
	std::vector<ref_count_ptr<StgEnemyBossObject>::unsync>& GetEnemyObjectList() { return listEnemyObject_; }
	void SetEnemyObjectList(const std::vector<ref_count_ptr<StgEnemyBossObject>::unsync>& list) { listEnemyObject_ = list; }
	int GetEnemyBossIdInCreate();
	bool IsReadyNext() const { return bReadyNext_; }
	void SetReadyNext() { bReadyNext_ = true; }

	_int64 GetCurrentSpellScore() const;
	_int64 GetSpellScore() const { return scoreSpell_; }
	void SetSpellScore(_int64 score) { scoreSpell_ = score; }
	int GetSpellTimer() const { return timerSpell_; }
	void SetSpellTimer(int timer) { timerSpell_ = timer; }
	int GetOriginalSpellTimer() const { return timerSpellOrg_; }
	void SetOriginalSpellTimer(int timer)
	{
		timerSpellOrg_ = timer;
		timerSpell_ = timer;
	}
	bool IsSpellCard() const { return bSpell_; }
	void SetSpellCard(bool b) { bSpell_ = b; }
	bool IsLastSpell() const { return bLastSpell_; }
	void SetLastSpell(bool b) { bLastSpell_ = b; }
	bool IsDurable() const { return bDurable_; }
	void SetDurable(bool b) { bDurable_ = b; }

	void AddPlayerShootDownCount() { countPlayerShootDown_++; }
	int GetPlayerShootDownCount() const { return countPlayerShootDown_; }
	void AddPlayerSpellCount() { countPlayerSpell_++; }
	int GetPlayerSpellCount() const { return countPlayerSpell_; }

private:
	std::wstring path_;
	_int64 isScript_;
	std::vector<double> listLife_;
	std::vector<ref_count_ptr<StgEnemyBossObject>::unsync> listEnemyObject_;
	int countCreate_; //ボス生成数。listEnemyObject_を超えて生成しようとしたらエラー。
	bool bReadyNext_;

	bool bSpell_; //スペルカード
	bool bLastSpell_; //ラストスペル
	bool bDurable_; //耐久スペル
	_int64 scoreSpell_;
	int timerSpellOrg_; //初期タイマー フレーム単位 -1で無効
	int timerSpell_; //タイマー フレーム単位 -1で無効
	int countPlayerShootDown_; //自機撃破数
	int countPlayerSpell_; //自機スペル使用数
};

#endif
