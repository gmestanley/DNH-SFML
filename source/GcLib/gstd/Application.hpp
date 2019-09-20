#ifndef __GSTD_APPLICATION__
#define __GSTD_APPLICATION__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"

namespace gstd {

/**********************************************************
//Application
**********************************************************/
class Application {
public:
	virtual ~Application();
	static Application* GetBase() { return thisBase_; }
	bool Initialize();

	virtual bool Run();
	bool IsActive() const { return this->bAppActive_; }
	void SetActive(bool b) { this->bAppActive_ = b; }
	bool IsRun() const { return bAppRun_; }
	void End() { bAppRun_ = false; }

	static HINSTANCE GetApplicationHandle() { return ::GetModuleHandle(NULL); }

protected:
	bool bAppRun_;
	bool bAppActive_;
	HINSTANCE hAppInstance_;
	virtual bool _Initialize() { return true; }
	virtual bool _Loop() { return true; }
	virtual bool _Finalize() { return true; }
	Application();

private:
	static Application* thisBase_;
};

} // namespace gstd

#endif
