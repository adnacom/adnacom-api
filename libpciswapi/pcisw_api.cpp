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

namespace Ipc {;

Expected<std::vector<std::string>> getAdIds_()
{
	SAFEARRAY* adsa = nullptr;

	HRESULT hr = ApiGetAdapterIds(&adsa);
	if (FAILED(hr)) {
		traceErr("GetAdapters() -> hr %#x", hr);
		return std::unexpected(hr);
	}

	assert(adsa);
	assert(adsa->cDims == 1);

	const auto count = adsa->rgsabound->cElements;

	const auto* arrayData = reinterpret_cast<BSTR*>(adsa->pvData);
	assert(adsa->cbElements >= count * sizeof(arrayData[0]));

	std::vector<std::string> ids;
	ids.reserve(count);
	for (unsigned i = adsa->rgsabound->lLbound; i < count; ++i)
		ids.emplace_back(bstr2string(arrayData[i]));

	::SafeArrayDestroy(adsa);

	return ids;
}

Expected<std::vector<std::string>> getAdPorts_(const wchar_t* adId)
{
	auto* adIdBstr = ::SysAllocString(adId);
	if (!adIdBstr) {
		return std::unexpected(E_OUTOFMEMORY);
	}

	SAFEARRAY* ptsa = nullptr;
	HRESULT hr = ApiGetAdapterPorts(adIdBstr, &ptsa);
	if (FAILED(hr)) {
		traceErr("GetAdapterPorts() -> hr %#x", hr);
		return std::unexpected(hr);
	}

	assert(ptsa);
	assert(::SafeArrayGetDim(ptsa) == 1);

	::SafeArrayLock(ptsa);

	{
		VARTYPE vt = VT_UNKNOWN;
		::SafeArrayGetVartype(ptsa, &vt);
		assert(vt == VT_BSTR);

		assert(::SafeArrayGetElemsize(ptsa) == sizeof(BSTR));
	}

	long startIdx = 0, endIdx = 0;
	hr = ::SafeArrayGetLBound(ptsa, 1, &startIdx);
	assert(SUCCEEDED(hr));
	hr = ::SafeArrayGetUBound(ptsa, 1, &endIdx);
	assert(SUCCEEDED(hr));
	++endIdx;

	const auto count = endIdx - startIdx;

	std::vector<std::string> ports;
	ports.reserve(count);
	for (auto i = startIdx; i < endIdx; ++i) {
		BSTR* ptId = nullptr;
		::SafeArrayPtrOfIndex(ptsa, &i, (void**)&ptId);
		if (ptId) {
			ports.emplace_back(bstr2string(*ptId));
		}
	}

	::SafeArrayUnlock(ptsa);
	::SafeArrayDestroy(ptsa);

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
