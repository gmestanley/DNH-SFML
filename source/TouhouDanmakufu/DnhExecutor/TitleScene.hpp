#ifndef __TOUHOUDANMAKUFU_EXE_TITLE_SCENE__
#define __TOUHOUDANMAKUFU_EXE_TITLE_SCENE__

#include "Common.hpp"
#include "GcLibImpl.hpp"

/**********************************************************
//TitleScene
**********************************************************/
class TitleScene : public TaskBase, public MenuTask {
public:
	enum {
		TASK_PRI_WORK = 5,
		TASK_PRI_RENDER = 5,
	};

private:
	enum {
		ITEM_COUNT = 7,
		ITEM_ALL = 0,
		ITEM_SINGLE,
		ITEM_PLURAL,
		ITEM_STAGE,
		ITEM_PACKAGE,
		ITEM_DIRECTORY,
		ITEM_QUIT,
	};
	ref_count_ptr<Sprite2D> spriteBack_;

public:
	TitleScene();
	void Work() override;
	void Render() override;
};

class TitleSceneMenuItem : public TextLightUpMenuItem {
public:
	TitleSceneMenuItem(const std::wstring& text, const std::wstring& description, int x, int y);
	~TitleSceneMenuItem() override;
	void Work() override;
	void Render() override;

private:
	ref_count_ptr<DxTextRenderObject> objText_;
	POINT pos_;

	TitleScene* _GetTitleScene() { return (TitleScene*)menu_; }
};

#endif
