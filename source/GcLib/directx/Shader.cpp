#include "Shader.hpp"
#include "HLSL.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ShaderData
**********************************************************/
ShaderData::ShaderData()
{
	manager_ = nullptr;
	bLoad_ = false;
	effect_ = nullptr;
	bText_ = false;
}
ShaderData::~ShaderData() = default;

/**********************************************************
//ShaderManager
**********************************************************/
const std::wstring NAME_DEFAULT_SKINNED_MESH = L"__NAME_DEFAULT_SKINNED_MESH__";
ShaderManager* ShaderManager::thisBase_ = nullptr;
ShaderManager::ShaderManager() = default;
ShaderManager::~ShaderManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);

	Clear();
}
bool ShaderManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;

	bool res = true;
	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);

	ref_count_ptr<Shader> shaderSkinedMesh = new Shader();
	std::string sourceSkinedMesh = HLSL_DEFAULT_SKINED_MESH;
	shaderSkinedMesh->CreateFromText(sourceSkinedMesh);
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderSkinedMesh);

	// shaderSkinedMesh->Begin();
	// shaderSkinedMesh->End();

	/*
	ref_count_ptr<Shader> shaderTest = new Shader();
	std::string pathDir = PathProperty::GetModuleDirectory() + "shader\\";
	shaderTest->CreateFromFile(pathDir + "Sharder_SkinnedMesh.txt");
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderTest);
	*/
	/*
	File file(pathDir + "test.txt");
	if (file.Open()) {
		std::string str;
		int size = file.GetSize();
		str.resize(size);
		file.Read(&str[0], size);

		ref_count_ptr<Shader> shaderTestFile = new Shader();
		shaderTestFile->CreateFromText(str);
	}
	*/
	return res;
}
void ShaderManager::Clear()
{
	{
		Lock lock(lock_);
		mapShader_.clear();
		mapShaderData_.clear();
	}
}
void ShaderManager::_ReleaseShaderData(const std::wstring& name)
{
	{
		Lock lock(lock_);
		if (IsDataExists(name)) {
			mapShaderData_[name]->bLoad_ = true; //読み込み完了扱い
			mapShaderData_.erase(name);
			Logger::WriteTop(StringUtility::Format(L"ShaderManager：Shaderを解放しました(Shader Released)[%s]", name.c_str()));
		}
	}
}
bool ShaderManager::_CreateFromFile(const std::wstring& _Path)
{
	lastError_ = L"";
	if (IsDataExists(_Path)) {
		return true;
	}

	std::wstring path = PathProperty::GetUnique(_Path);
	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if (reader == nullptr || !reader->Open()) {
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s", path.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
		return false;
	}

	int size = reader->GetFileSize();
	ByteBuffer buf;
	buf.SetSize(size);
	reader->Read(buf.GetPointer(), size);

	std::string source;
	source.resize(size);
	memcpy(&source[0], buf.GetPointer(), size);

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ID3DXBuffer* pErr = nullptr;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr);

	bool res = true;
	if (FAILED(hr)) {
		res = false;
		std::wstring err;
		if (pErr != nullptr) {
			char* cText = (char*)pErr->GetBufferPointer();
			err = StringUtility::ConvertMultiToWide(cText);
		}
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s\r\n[%s]", path.c_str(), err.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
	} else {
		std::wstring log = StringUtility::Format(L"Shader読み込み(Shader Load Success)：\r\n%s", path.c_str());
		Logger::WriteTop(log);

		mapShaderData_[path] = data;
		data->manager_ = this;
		data->name_ = path;
	}
	return res;
}
bool ShaderManager::_CreateFromText(const std::string& source)
{
	lastError_ = L"";
	std::wstring id = _GetTextSourceID(source);
	if (IsDataExists(id)) {
		return true;
	}

	bool res = true;
	DirectGraphics* graphics = DirectGraphics::GetBase();

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());
	ID3DXBuffer* pErr = nullptr;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr);

	std::string tStr = StringUtility::Slice(source, 128);
	if (FAILED(hr)) {
		res = false;
		char* err = "";
		if (pErr != nullptr)
			err = (char*)pErr->GetBufferPointer();
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Load Shader Failed)：\r\n%s\r\n[%s]", tStr.c_str(), err);
		Logger::WriteTop(log);
		lastError_ = log;
	} else {
		std::wstring log = L"Shader読み込み(Load Shader Success)：";
		log += StringUtility::FormatToWide("%s", tStr.c_str());
		Logger::WriteTop(log);

		mapShaderData_[id] = data;
		data->manager_ = this;
		data->name_ = id;
		data->bText_ = true;
	}
	return res;
}
std::wstring ShaderManager::_GetTextSourceID(const std::string& source)
{
	std::wstring res = StringUtility::ConvertMultiToWide(source);
	res = StringUtility::Slice(res, 64);
	return res;
}
void ShaderManager::_BeginShader(Shader* shader, int pass)
{
	Shader* lastShader = nullptr;
	if (!listExecuteShader_.empty()) {
		lastShader = *listExecuteShader_.rbegin();
	}

	if (shader != nullptr && shader != lastShader) {
		if (lastShader != nullptr) {
			lastShader->_EndPass();
			lastShader->_End();
		}

		shader->_Begin();
		shader->_BeginPass(pass);
	} else if (shader == nullptr && lastShader != nullptr) {
		lastShader->_EndPass();
		lastShader->_End();
	}
	listExecuteShader_.push_back(shader);
}
void ShaderManager::_EndShader(Shader* shader)
{
	Shader* preShader = nullptr;
	if (!listExecuteShader_.empty()) {
		preShader = *listExecuteShader_.rbegin();
		listExecuteShader_.pop_back();
	}

	if (shader != preShader)
		throw gstd::wexception(L"EndShader異常");

	preShader = nullptr;
	if (!listExecuteShader_.empty()) {
		preShader = *listExecuteShader_.rbegin();
	}

	//同じShaderなら何もしない
	if (shader == preShader)
		return;
	shader->_EndPass();
	shader->_End();

	if (preShader != nullptr) {
		shader->_Begin();
		shader->_BeginPass();
	}
}

void ShaderManager::ReleaseDxResource()
{
	for (auto itrMap = mapShader_.begin(); itrMap != mapShader_.end(); ++itrMap) {
		std::wstring name = itrMap->first;
		Shader* data = (itrMap->second).GetPointer();
		data->ReleaseDxResource();
	}
}
void ShaderManager::RestoreDxResource()
{
	for (auto itrMap = mapShader_.begin(); itrMap != mapShader_.end(); ++itrMap) {
		std::wstring name = itrMap->first;
		Shader* data = (itrMap->second).GetPointer();
		data->RestoreDxResource();
	}
}

bool ShaderManager::IsDataExists(const std::wstring& name) const
{
		Lock lock(lock_);
		return mapShaderData_.find(name) != mapShaderData_.end();
}
gstd::ref_count_ptr<ShaderData> ShaderManager::GetShaderData(const std::wstring& name)
{
	Lock lock(lock_);
	auto shaderDataItr = mapShaderData_.find(name);
	if (shaderDataItr != mapShaderData_.end()) {
		return shaderDataItr->second;
	}
	return nullptr;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromFile(const std::wstring& _Path)
{
	Lock lock(lock_);
	std::wstring path = PathProperty::GetUnique(_Path);
	auto shaderItr = mapShader_.find(path);
	if (shaderItr != mapShader_.end()) {
		return shaderItr->second;
	} else {
		if (_CreateFromFile(path)) {
			gstd::ref_count_ptr<Shader> res = new Shader();
			res->data_ = mapShaderData_[path];
			return res;
		}
	}
	return nullptr;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromText(const std::string& source)
{
	Lock lock(lock_);
	std::wstring id = _GetTextSourceID(source);
	auto shaderItr = mapShader_.find(id);
	if (shaderItr != mapShader_.end()) {
		return shaderItr->second;
	} else {
		if (_CreateFromText(source) == true) {
			gstd::ref_count_ptr<Shader> res = new Shader();
			res->data_ = mapShaderData_[id];
			return res;
		}
	}
	return nullptr;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromFileInLoadThread(const std::wstring& path)
{
	return false;
}
void ShaderManager::CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event)
{
}

void ShaderManager::AddShader(const std::wstring& name, gstd::ref_count_ptr<Shader> shader)
{
	Lock lock(lock_);
	mapShader_[name] = shader;
}
void ShaderManager::DeleteShader(const std::wstring& name)
{
	Lock lock(lock_);
	mapShader_.erase(name);
}
gstd::ref_count_ptr<Shader> ShaderManager::GetShader(const std::wstring& name)
{
	Lock lock(lock_);
	auto shaderItr = mapShader_.find(name);
	if (shaderItr != mapShader_.end()) {
		return shaderItr->second;
	}
	return nullptr;
}
gstd::ref_count_ptr<Shader> ShaderManager::GetDefaultSkinnedMeshShader()
{
	return GetShader(NAME_DEFAULT_SKINNED_MESH);
}
void ShaderManager::CheckExecutingShaderZero() const
{
	if (!listExecuteShader_.empty())
		throw gstd::wexception(L"CheckExecutingShaderZero");
}
std::wstring ShaderManager::GetLastError()
{
	{
		Lock lock(lock_);
		return lastError_;
	}
}

/**********************************************************
//ShaderParameter
**********************************************************/
ShaderParameter::ShaderParameter()
{
	type_ = TYPE_UNKNOWN;
	value_ = new ByteBuffer();
}
ShaderParameter::~ShaderParameter() = default;
void ShaderParameter::SetMatrix(D3DXMATRIX& matrix)
{
	type_ = TYPE_MATRIX;
	int size = sizeof(D3DXMATRIX);
	value_->Seek(0);
	value_->Write(&matrix, size);
}
D3DXMATRIX ShaderParameter::GetMatrix()
{
	return (D3DXMATRIX&)*value_->GetPointer();
}
void ShaderParameter::SetMatrixArray(std::vector<D3DXMATRIX>& listMatrix)
{
	type_ = TYPE_MATRIX_ARRAY;
	value_->Seek(0);
	for (auto& matrix : listMatrix) {
		value_->Write(&matrix.m, sizeof(D3DMATRIX));
	}
}
std::vector<D3DXMATRIX> ShaderParameter::GetMatrixArray()
{
	int count = value_->GetSize() / sizeof(D3DMATRIX);
	std::vector<D3DXMATRIX> matrixArray(count);

	value_->Seek(0);
	for (auto& matrix : matrixArray) {
		value_->Read(&matrix.m, sizeof(D3DMATRIX));
	}

	return matrixArray;
}
void ShaderParameter::SetVector(D3DXVECTOR4& vector)
{
	type_ = TYPE_VECTOR;
	int size = sizeof(D3DXVECTOR4);
	value_->Seek(0);
	value_->Write(&vector, size);
}
D3DXVECTOR4 ShaderParameter::GetVector()
{
	return (D3DXVECTOR4&)*value_->GetPointer();
}
void ShaderParameter::SetFloat(float value)
{
	type_ = TYPE_FLOAT;
	int size = sizeof(float);
	value_->Seek(0);
	value_->Write(&value, size);
}
float ShaderParameter::GetFloat()
{
	return (float&)*value_->GetPointer();
}
void ShaderParameter::SetFloatArray(std::vector<float>& values)
{
	type_ = TYPE_FLOAT_ARRAY;
	int size = sizeof(float) * values.size();
	value_->Seek(0);
	value_->Write(&values[0], size);
}
std::vector<float> ShaderParameter::GetFloatArray()
{
	int count = value_->GetSize() / sizeof(float);
	std::vector<float> floatArray(count);

	value_->Seek(0);
	value_->Read(&floatArray[0], value_->GetSize());
	return floatArray;
}
void ShaderParameter::SetTexture(const gstd::ref_count_ptr<Texture> texture)
{
	type_ = TYPE_TEXTURE;
	texture_ = texture;
}
gstd::ref_count_ptr<Texture> ShaderParameter::GetTexture()
{
	return texture_;
}

/**********************************************************
//Shader
**********************************************************/
Shader::Shader()
{
	data_ = nullptr;
	// bLoadShader_ = false;
	// pVertexShader_ = NULL;
	// pPixelShader_ = NULL;
}
Shader::Shader(const Shader* shader)
{
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		data_ = shader->data_;
	}
}
Shader::~Shader()
{
	Release();
}
void Shader::Release()
{
	// if (pVertexShader_ != NULL)
	// 	pVertexShader_->Release();
	// if (pPixelShader_ != NULL)
	// 	pPixelShader_->Release();
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		if (data_ != nullptr) {
			ShaderManager* manager = data_->manager_;
			if (manager != nullptr && manager->IsDataExists(data_->name_)) {
				int countRef = data_.GetReferenceCount();
				//自身とTextureManager内の数だけになったら削除
				if (countRef == 2) {
					manager->_ReleaseShaderData(data_->name_);
				}
			}
			data_ = nullptr;
		}
	}
}

void Shader::Begin(int pass)
{
	ShaderManager* manager = ShaderManager::GetBase();
	manager->_BeginShader(this, pass);
}
void Shader::End()
{
	ShaderManager* manager = ShaderManager::GetBase();
	manager->_EndShader(this);
}

ID3DXEffect* Shader::GetEffect()
{
	ID3DXEffect* res = nullptr;
	if (data_ != nullptr)
		res = data_->effect_;
	return res;
}
void Shader::ReleaseDxResource()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return;
	effect->OnLostDevice();
}
void Shader::RestoreDxResource()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return;
	effect->OnResetDevice();
}
bool Shader::CreateFromFile(const std::wstring& _Path)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	std::wstring path = PathProperty::GetUnique(_Path);
	if (data_ != nullptr)
		Release();
	ShaderManager* manager = ShaderManager::GetBase();
	ref_count_ptr<Shader> shader = manager->CreateFromFile(path);
	if (shader != nullptr) {
		data_ = shader->data_;
	}
	return (data_ != nullptr);
}
bool Shader::CreateFromText(const std::string& source)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();
	ShaderManager* manager = ShaderManager::GetBase();
	ref_count_ptr<Shader> shader = manager->CreateFromText(source);
	if (shader != nullptr) {
		data_ = shader->data_;
	}
	return (data_ != nullptr);
}

int Shader::_Begin()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return 0;
	_SetupParameter();

	unsigned int res = S_OK;
	HRESULT hr = effect->Begin(&res, 0);
	return res;
}
void Shader::_End()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return;
	effect->End();
}
void Shader::_BeginPass(int pass)
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return;
	effect->BeginPass(pass);
	/*
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	if(!bLoadShader_)
	{
		//http://www.gamedev.net/topic/646178-given-an-effect-technique-pass-handle-how-to-get-the-pixelshader/
		D3DXHANDLE hTechnique = effect->GetCurrentTechnique();
		D3DXHANDLE hPass = effect->GetPass(hTechnique, pass);

		D3DXPASS_DESC passDesc;
		effect->GetPassDesc(hPass, &passDesc);
		if (passDesc.pVertexShaderFunction != NULL)
			device->CreateVertexShader(passDesc.pVertexShaderFunction, &pVertexShader_);
		if (passDesc.pPixelShaderFunction != NULL)
			device->CreatePixelShader(passDesc.pPixelShaderFunction, &pPixelShader_);
		bLoadShader_ = true;
	}

	// device->SetVertexShader(pVertexShader_);
	device->SetPixelShader(pPixelShader_);
	*/
}
void Shader::_EndPass()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return;
	effect->EndPass();

	/*
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	// device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
	*/
}

bool Shader::_SetupParameter()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == nullptr)
		return false;
	HRESULT hr = effect->SetTechnique(technique_.c_str());
	if (FAILED(hr))
		return false;

	std::map<std::string, gstd::ref_count_ptr<ShaderParameter>>::iterator itrParam;
	for (itrParam = mapParam_.begin(); itrParam != mapParam_.end(); itrParam++) {
		std::string name = itrParam->first;
		gstd::ref_count_ptr<ShaderParameter> param = itrParam->second;
		int type = param->GetType();
		switch (type) {
		case ShaderParameter::TYPE_MATRIX: {
			D3DXMATRIX matrix = param->GetMatrix();
			hr = effect->SetMatrix(name.c_str(), &matrix);
			break;
		}
		case ShaderParameter::TYPE_MATRIX_ARRAY: {
			std::vector<D3DXMATRIX> matrixArray = param->GetMatrixArray();
			hr = effect->SetMatrixArray(name.c_str(), &matrixArray[0], matrixArray.size());
			break;
		}
		case ShaderParameter::TYPE_VECTOR: {
			D3DXVECTOR4 vect = param->GetVector();
			hr = effect->SetVector(name.c_str(), &vect);
			break;
		}
		case ShaderParameter::TYPE_FLOAT: {
			float value = param->GetFloat();
			hr = effect->SetFloat(name.c_str(), value);
			break;
		}
		case ShaderParameter::TYPE_FLOAT_ARRAY: {
			std::vector<float> value = param->GetFloatArray();
			hr = effect->SetFloatArray(name.c_str(), &value[0], value.size());
			break;
		}
		case ShaderParameter::TYPE_TEXTURE: {
			gstd::ref_count_ptr<Texture> texture = param->GetTexture();
			IDirect3DTexture9* pTex = texture->GetD3DTexture();
			hr = effect->SetTexture(name.c_str(), pTex);
			break;
		}
		}
		// if (FAILED(hr))
		// 	return false;
	}
	return true;
}
gstd::ref_count_ptr<ShaderParameter> Shader::_GetParameter(const std::string& name, bool bCreate)
{
	bool bFind = mapParam_.find(name) != mapParam_.end();
	if (!bFind && !bCreate)
		return nullptr;

	gstd::ref_count_ptr<ShaderParameter> res = nullptr;
	if (!bFind) {
		res = new ShaderParameter();
		mapParam_[name] = res;
	} else {
		res = mapParam_[name];
	}

	return res;
}
bool Shader::SetTechnique(const std::string& name)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == nullptr)
	// 	return false;
	// effect->SetTechnique(name.c_str());

	technique_ = name;
	return true;
}
bool Shader::SetMatrix(const std::string& name, D3DXMATRIX& matrix)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == nullptr)
	// 	return false;
	// effect->SetMatrix(name.c_str(), &matrix);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrix(matrix);

	return true;
}
bool Shader::SetMatrixArray(const std::string& name, std::vector<D3DXMATRIX>& matrix)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == nullptr)
	// 	return false;
	// effect->SetMatrixArray(name.c_str(), &matrix[0], matrix.size());

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrixArray(matrix);

	return true;
}
bool Shader::SetVector(const std::string& name, D3DXVECTOR4& vector)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == nullptr)
	// 	return false;
	// effect->SetVector(name.c_str(), &vector);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetVector(vector);
	return true;
}
bool Shader::SetFloat(const std::string& name, float value)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == nullptr)
	// 	return false;
	// effect->SetFloat(name.c_str(), value);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloat(value);
	return true;
}
bool Shader::SetFloatArray(const std::string& name, std::vector<float>& values)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloatArray(values);
	return true;
}
bool Shader::SetTexture(const std::string& name, gstd::ref_count_ptr<Texture> texture)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetTexture(texture);
	return true;
}
