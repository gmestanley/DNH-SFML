#ifndef __TOUHOUDANMAKUFU_EXE_STG_SCENE__
#define __TOUHOUDANMAKUFU_EXE_STG_SCENE__

#include "../Common/StgSystem.hpp"

/**********************************************************
//EStgSystemController
**********************************************************/
class EStgSystemController : public StgSystemController {
protected:
	void DoEnd() override;
	void DoRetry() override;
};

/**********************************************************
//PStgSystemController
**********************************************************/
class PStgSystemController : public StgSystemController {
protected:
	void DoEnd() override;
	void DoRetry() override;
};

#endif
