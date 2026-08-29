/*!
 * \file `pcisw_api.h`
 * Adnacom PCIe Adapter diagnostics API.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <cstdint>
#include <string>
#include <vector>

#include "pcisw_defs.h"


namespace Adnacom::Api {;

/*!
 * Type used as an Adapter ID.
 * Should be treated as an opaque datatype, as it may change in the future.
 * Application developers should not make assumptions about this type or
 * the Adapter ID values returned by the API.
 */
using AdapterId = std::string;


/*!
 * Describes an Adnacom PCIe Adapter instance - Host or Remote.
 */
class Adapter final
{
public /*static*/:
	/*!
	 * Retrieves list of IDs for PCIe Adapters currently present in the system.
	 * \return [std::vector<AdapterId>] Vector of available Adapter IDs.
	 */
	static std::vector<AdapterId> GetAdapterIds();

public:
	/*!
	 * Creates object for PCIe Adapter with a given ID.
	 * \param[in] id Adapter ID. \see `AdapterId`
	 */
	Adapter(const AdapterId& id);
	~Adapter();

	// Copy/move
	Adapter(const Adapter& other);
	Adapter(Adapter&& other);
	Adapter& operator=(const Adapter& other);
	Adapter& operator=(Adapter&& other);

	/*!
	 * Returns number of available ports on the adapter.
	 * \returns [int] - Port count (upstream and downstream). Returns negative value in case of error (e.g. invalid adapter ID).
	 */
	int GetPortCount() const;

	/*!
	 * Returns number of available ports on the adapter.
	 * \returns [int] - Port count (upstream and downstream). Returns negative value in case of error (e.g. invalid adapter ID).
	 */
	AdapterType GetAdapterType() const;

	/*!
	 * Reads adapter's property value.
	 * \param[in] propertyType - Type of property to be retrieved, \see `AdapterProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`; on output: receives the size of the requested property, in bytes.
	 *		If the output buffer is too small to hold the data, the data will be truncated, i.e. the function writes as many bytes
	 *		as possible into the supplied buffer, and returns the total available data size in the `*bufferSize` argument.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure; if the output data has been truncated because of insufficient buffer length, `MoreDataAvailable` is returned.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetAdapterProperty(AdapterProperty propertyType, void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;

	/*!
	 * Reads adapter port's property value.
	 * \param[in] portIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] propertyType - Type of property to be retrieved, \see `AdapterPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`; on output: size of the requested property, in bytes.
	 *		If the output buffer is too small to hold the data, the data will be truncated, i.e. the function writes as many bytes
	 *		as possible into the supplied buffer, and returns the total available data size in the `*bufferSize` argument.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure; if the output data has been truncated because of insufficient buffer length, `MoreDataAvailable` is returned.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetPortProperty(int portIndex, AdapterPortProperty propertyType, void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;

	/*!
	 * Reads property of adapter's transceiver.
	 * \param[in] transceiverIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] transceiverPropertyType - Type of property to be retrieved, \see `AdapterTransceiverPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`; on output: size of the requested property, in bytes.
	 *		If the output buffer is too small to hold the data, the data will be truncated, i.e. the function writes as many bytes
	 *		as possible into the supplied buffer, and returns the total available data size in the `*bufferSize` argument.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure; if the output data has been truncated because of insufficient buffer length, `MoreDataAvailable` is returned.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetTransceiverProperty(int transceiverIndex, AdapterTransceiverProperty transceiverPropertyType,
		void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;

	// Deprecated: use `GetPortProperty()` instead.
	bool GetPortInfo(int portIndex, AdapterPortProperty infoType, void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const
	{
		return GetPortProperty(portIndex, infoType, outBuffer, bufferSize, resultCode);
	}

protected:
	struct Impl;
	Impl* impl_ = nullptr;
};

} // Adnacom::Api namespace
