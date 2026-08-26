/**
 * \file pcisw_api.cpp
 * Adnacom PCI Host Adapter diagnostics API client implementation.
 * \date 2026.05.19
 */
#include "pcisw/pcisw_api.h"
#include "pcisw_api_lrpc.h"

#include "trace.h"
#include <cstdio>
#include <cassert>

#include <expected>


// FWD
void LrpcFreeMemory(void* ptr);
HRESULT LrpcConnect();

// String Utilities
inline
std::string bstr2string(BSTR bstr)
{
	if (!bstr)
		return {};

	int charCount = ::WideCharToMultiByte(CP_UTF8, 0, bstr, -1, nullptr, 0, nullptr, nullptr);
	if (charCount <= 0)
		return {};
	if (charCount == 1)
		return {};

	std::string s(charCount - 1, '\0');

	charCount = ::WideCharToMultiByte(CP_UTF8, 0, bstr, -1, s.data(), (int)s.length() + 1, nullptr, nullptr);
	assert(charCount == s.length() + 1);
	return s;
}
inline
std::string ws2string(const std::wstring& wstr)
{
	if (wstr.empty())
		return {};

	int charCount = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
	if (charCount <= 0)
		return {};

	// Remove trailing null character from string.
	std::string s(charCount, '\0');
	charCount = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), s.data(), (int)s.length(), nullptr, nullptr);
	assert(charCount == s.length());
	return s;
}
inline
std::wstring string2ws(const std::string& str)
{
	if (str.empty())
		return {};

	int charCount = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
	if (charCount <= 0)
		return {};

	// Remove trailing null character from string.
	std::wstring ws(charCount, '\0');
	charCount = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), ws.data(), (int)ws.length());
	assert(charCount == ws.length());
	return ws;
}


static struct Globals
{
	// Type of RPC client handle used as a first parameter in all RPC methods.
	using ClientHandle = handle_t;

	RPC_BINDING_HANDLE bindHandle {};
	unsigned protocolVersion = 0;

	bool connect()
	{
		if (!bindHandle) {
			HRESULT hr = LrpcConnect();
			if (FAILED(hr)) {
				traceErr("couldn't connect RPC: hr %#x", hr);
				return false;
			}
		}

		if (protocolVersion == 0) {
			HRESULT hr = wrapRpcCall_(PciswGetProtocolVersion, &protocolVersion);
			if (SUCCEEDED(hr))
				trace("RPC protocol: v%u", protocolVersion);
			else
				traceErr("couldn't get RPC protocol version: hr %#x", hr);
		}

		return bindHandle != 0 && protocolVersion > 0;
	}

	template <typename... FnArgs, typename... CallArgs>
	HRESULT rpcCall(HRESULT(&fn)(ClientHandle, FnArgs...), CallArgs&&... args)
	{
		if (!connect())
			return RPC_E_DISCONNECTED;

		return wrapRpcCall_(fn, std::forward<CallArgs>(args)...);
	}

protected:
	template <typename... FnArgs, typename... CallArgs>
	HRESULT wrapRpcCall_(HRESULT(&fn)(ClientHandle, FnArgs...), CallArgs&&... args)
	{
		HRESULT hr = E_FAIL;
		__try {
			hr = fn(bindHandle, std::forward<CallArgs>(args)...);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			hr = HRESULT_FROM_WIN32(GetExceptionCode());
		}
		return hr;
	}
} Gl;


HRESULT LrpcConnect()
{
	RPC_WSTR pszStringBinding = nullptr;
	RPC_STATUS status = ::RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)L"ncalrpc",
		NULL,
		(RPC_WSTR)L"Adnacom.Api",
		NULL,
		&pszStringBinding);

	if (status) {
		traceErr("couldn't create RPC string binding, err %u", status);
		return status;
	}

	RPC_BINDING_HANDLE bindHandle{};
	status = ::RpcBindingFromStringBinding(pszStringBinding, &bindHandle);
	if (FAILED(status)) {
		traceErr("couldn't create binding, err %u", status);
		return status;
	}

	status = ::RpcStringFree(&pszStringBinding);
	if (FAILED(status)) {
		traceErr("`RpcStringFree()` failed, err %u", status);
	}

	Gl.bindHandle = bindHandle;
	trace("IPC connection created! BindHandle %p", bindHandle);
	return S_OK;
}


HRESULT ApiGetProtocolVersion(unsigned* version)
{
	return Gl.rpcCall(PciswGetProtocolVersion, version);
}

HRESULT ApiGetAdapterIds(SAFEARRAY** ids)
{
	return Gl.rpcCall(PciswGetAdapterIds, ids);
}

HRESULT ApiGetAdapterPorts(const BSTR adapterId, SAFEARRAY** portIds)
{
	return Gl.rpcCall(PciswGetAdapterPorts, adapterId, portIds);
}

HRESULT ApiGetPortInfo(const wchar_t* portPath, unsigned infoType, byte*& outBuffer, unsigned& outBufferSize)
{
	auto pathBstr = ::SysAllocString(portPath);
	HRESULT hr = Gl.rpcCall(PciswGetPortInfo, pathBstr, infoType, &outBufferSize, &outBuffer);
	::SysFreeString(pathBstr);
	return hr;
}


void __RPC_FAR* __RPC_USER midl_user_allocate(size_t len)
{
	void* ptr = ::GlobalAlloc(0, len);
	if (ptr)
		trace("midl> alloc'd %u bytes", (unsigned)len);
	else
		traceErr("midl> failed to alloc %u bytes", (unsigned)len);
	return ptr;
}
void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
{
	LrpcFreeMemory(ptr);
}

void LrpcFreeMemory(void* ptr)
{
	if (!ptr)
		return;
	size_t sz = ::GlobalSize(ptr);
	if (sz == 0)
		traceErr("midl> LrpcFreeMemory(): failed to get allocation size");
	::GlobalFree(ptr);
	trace("midl> free'd %u bytes", (unsigned)sz);
}


// DEBUGGING FACILITIES

void traceImpl_(const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	wchar_t buffer[256];
	vswprintf_s(buffer, fmt, args);

	va_end(args);

	::OutputDebugString(buffer);
}


// IPC WRAPPERS

template<typename Ty>
using Expected = std::expected<Ty, HRESULT>;

template <typename Ty, typename Src>
struct const_if
{
	using type = std::conditional_t<std::is_const_v<Src>, const Ty, Ty>;
};

template <typename Et>
struct SafearrayIterator
{
	SafearrayIterator(SAFEARRAY* sa, LONG index) : sa_(sa), idx_(index)
	{
		LONG lb = -1;
		::SafeArrayGetLBound(sa_, 1, &lb);
		LONG ub = -1;
		::SafeArrayGetUBound(sa_, 1, &ub);

		assert(lb <= idx_ && lb <= ub);
	}

	template <typename This>
	auto& operator*(this This&& self)
	{
		using Result = std::conditional_t<std::is_const_v<This>, const Et, Et>;

		assert(self.sa_);
		Result* ptr = nullptr;
		HRESULT hr = ::SafeArrayPtrOfIndex(self.sa_, &self.idx_, (void**)&ptr);
		assert(SUCCEEDED(hr) && ptr);
		return *ptr;
	}

	template <typename Self>
	auto* operator->(this Self&& self)
	{
		return std::addressof(*self);
	}

	bool operator==(const SafearrayIterator& other) const = default;
	bool operator!=(const SafearrayIterator& other) const = default;
	bool operator<(const SafearrayIterator& other) const
	{
		assert(sa_ == other.sa_);
		return idx_ < other.idx_;
	}

	SafearrayIterator operator++(int)
	{
		SafearrayIterator tmp = *this;
		++idx_;
		return tmp;
	}
	SafearrayIterator& operator++()
	{
		++idx_;
		return *this;
	}

protected:
	SAFEARRAY* sa_ = nullptr;
	LONG idx_ = 0;
};

template <typename Ty>
struct SafearrayWrapper
{
	using iterator = SafearrayIterator<Ty>;

	SafearrayWrapper() = default;
	SafearrayWrapper(SAFEARRAY* sa) : mysa_(sa)
	{
		if (mysa_) {
			::SafeArrayLock(mysa_);
			assert(::SafeArrayGetElemsize(mysa_) >= sizeof(Ty));
		}
	}
	~SafearrayWrapper()
	{
		if (mysa_) {
			::SafeArrayUnlock(mysa_);
			::SafeArrayDestroy(mysa_);
		}
	}

	size_t size() const
	{
		if (!mysa_)
			return 0;

		assert(::SafeArrayGetDim(mysa_) == 1);
		LONG lbound = 0;
		HRESULT hr = ::SafeArrayGetLBound(mysa_, 1, &lbound);
		assert(SUCCEEDED(hr));
		LONG ubound = 0;
		hr = ::SafeArrayGetUBound(mysa_, 1, &ubound);
		assert(SUCCEEDED(hr));
		assert(ubound - lbound + 1 >= 0);
		if (ubound - lbound + 1 < 0)
			return 0;
		return static_cast<size_t>(ubound - lbound + 1);
	}

	iterator begin() const
	{
		LONG idx = 0;
		HRESULT hr = ::SafeArrayGetLBound(mysa_, 1, &idx);
		return iterator{mysa_, idx};
	}

	iterator end() const
	{
		LONG idx = 0;
		if (mysa_) {
			HRESULT hr = ::SafeArrayGetUBound(mysa_, 1, &idx);
			++idx; // end() is one past the last element.
		}
		return iterator{mysa_, idx};
	}
	
	bool empty() const { return size() == 0; }
	explicit operator bool() const { return mysa_ != nullptr; }

	SAFEARRAY* get() const { return mysa_; }

	SAFEARRAY** operator&() { return &mysa_; }
	

protected:
	SAFEARRAY* mysa_ = nullptr;
};

namespace Ipc {;

Expected<std::vector<std::string>> getAdIds_()
{
	SafearrayWrapper<BSTR> adsa;
	HRESULT hr = ApiGetAdapterIds(&adsa);
	if (FAILED(hr)) {
		traceErr("GetAdapters() -> hr %#x", hr);
		return std::unexpected(hr);
	}

	const auto count = adsa.size();
	std::vector<std::string> ids;
	ids.reserve(count);
	for (auto& e : adsa)
		ids.emplace_back(bstr2string(e));

	return ids;
}

Expected<std::vector<std::string>> getAdPorts_(const wchar_t* adId)
{
	auto* adIdBstr = ::SysAllocString(adId);
	if (!adIdBstr) {
		return std::unexpected(E_OUTOFMEMORY);
	}

	SafearrayWrapper<BSTR> ptsa;
	HRESULT hr = ApiGetAdapterPorts(adIdBstr, &ptsa);
	if (FAILED(hr)) {
		traceErr("GetAdapterPorts() -> hr %#x", hr);
		return std::unexpected(hr);
	}

	std::vector<std::string> ports;
	ports.reserve(ptsa.size());
	for (auto& ptBstr : ptsa) {
		if (!ptBstr)
			continue;
		ports.emplace_back(bstr2string(ptBstr));
	}

	return ports;
}

} // Ipc namespace


/// \region API Implementation

namespace Adnacom::Api {;


/*static*/
std::vector<HostAdapterId> HostAdapter::GetAdapterIds()
{
	return Ipc::getAdIds_().value_or({}); // Return empty vector on error.
}

HostAdapter::HostAdapter(const HostAdapterId& id) : id_{id}
{
}

int HostAdapter::GetPortCount() const
{
	auto result = Ipc::getAdPorts_(string2ws(id_).c_str());
	if (!result.has_value()) {
		traceErr("GetAdapters() -> %d", result.error());
		return -1;
	}

	return  static_cast<int>(result->size());
}

bool HostAdapter::GetPortInfo(int portIndex, HostAdapterPortProperty infoType, void* outBuffer, uint32_t& bufferSize)
{
	auto result = Ipc::getAdPorts_(string2ws(id_).c_str());

	if (!result) {
		traceErr("GetPortInfo: failed to enumerate ports! Error %d", result.error());
		return false;
	}
	auto& ports = *result;
	if (ports.empty()) {
		traceErr("GetPortInfo: no ports on adapter '%s'", id_.c_str());
		return false;
	}

	if (portIndex >= (int)ports.size()) {
		traceErr("GetPortInfo(): invalid port index %d; port count: %d", portIndex, (int)ports.size());
		return false;
	}

	const auto& portId = ports[portIndex];

	byte* info = nullptr;
	unsigned infoSize = bufferSize;
	HRESULT hr = ApiGetPortInfo(string2ws(portId).c_str(), static_cast<unsigned>(infoType), info, infoSize);
	if (FAILED(hr) || !info) {
		traceErr("GetPortInfo(index %d) -> %#x", portIndex, hr);
		return false;
	}

	bool succeeded = false;
	if (outBuffer && bufferSize >= infoSize) {
		memcpy(outBuffer, info, infoSize);
		succeeded = true;
	} else {
		bufferSize = infoSize;
	}

	LrpcFreeMemory(info);

	return succeeded;
}

} // Adnacom::Api namespace
