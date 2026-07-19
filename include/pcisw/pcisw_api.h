/*!
 * \file `pcisw_api.h`
 * Adnacom PCI Host Adapter diagnostics API.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <cstdint>
#include <string>
#include <vector>

#include "pcisw_defs.h"


namespace Adnacom::Api {;

/*!
 * Type used as a Host Adapter ID.
 * Should be treated as an opaque datatype, as it may change in the future.
 * Application developers should not make assumptions about this type or
 * the Adapter ID values returned by the API.
 */
using HostAdapterId = std::string;


/*!
 * Describes an Adnacom Host Adapter instance.
 */
class HostAdapter final
{
public /*static*/:
	/*!
	 * Retrieves list of IDs for Host Adapters currently present in the system.
	 * \return [std::vector<HostAdapterId>] Vector of available Host Adapter IDs.
	 */
	static std::vector<HostAdapterId> GetAdapterIds();

public:
	/*!
	 * Creates object for Host Adapter with a given ID.
	 * \param[in] id Host Adapter ID. \see `HostAdapterId`
	 */
	HostAdapter(const HostAdapterId& id);
	~HostAdapter();

	// Copy/move
	HostAdapter(const HostAdapter& other);
	HostAdapter(HostAdapter&& other);
	HostAdapter& operator=(const HostAdapter& other);
	HostAdapter& operator=(HostAdapter&& other);

	/*!
	 * Returns number of available ports on the adapter.
	 * \returns [int] - Port count (upstream and downstream). Returns negative value in case of error (e.g. invalid adapter ID).
	 */
	int GetPortCount() const;

	/*!
	 * Returns number of available ports on the adapter.
	 * \returns [int] - Port count (upstream and downstream). Returns negative value in case of error (e.g. invalid adapter ID).
	 */
	AdapterBoardType GetBoardType() const;

	/*!
	 * Reads adapter's property value.
	 * \param[in] propertyType - Type of property to be retrieved, \see `HostAdapterProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetAdapterProperty(HostAdapterProperty propertyType, void* outBuffer, uint32_t& bufferSize);

	/*!
	 * Reads adapter port's property value.
	 * \param[in] portIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] propertyType - Type of property to be retrieved, \see `HostAdapterPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetPortProperty(int portIndex, HostAdapterPortProperty propertyType, void* outBuffer, uint32_t& bufferSize);

	/*!
	 * Reads property of adapter's transceiver.
	 * \param[in] transceiverIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] transceiverPropertyType - Type of property to be retrieved, \see `HostAdapterTransceiverPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetTransceiverProperty(int transceiverIndex, HostAdapterTransceiverProperty transceiverPropertyType,
		void* outBuffer, uint32_t& bufferSize);

	// Deprecated: use `GetPortProperty()` instead.
	bool GetPortInfo(int portIndex, HostAdapterPortProperty infoType, void* outBuffer, uint32_t& bufferSize)
	{
		return GetPortProperty(portIndex, infoType, outBuffer, bufferSize);
	}

protected:
	struct Impl;
	Impl* impl_ = nullptr;
};

} // Adnacom::Api namespace
