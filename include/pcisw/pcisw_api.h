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
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetAdapterProperty(AdapterProperty propertyType, void* outBuffer, uint32_t& bufferSize);

	/*!
	 * Reads adapter port's property value.
	 * \param[in] portIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] propertyType - Type of property to be retrieved, \see `AdapterPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetPortProperty(int portIndex, AdapterPortProperty propertyType, void* outBuffer, uint32_t& bufferSize);

	/*!
	 * Reads property of adapter's transceiver.
	 * \param[in] transceiverIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] transceiverPropertyType - Type of property to be retrieved, \see `AdapterTransceiverPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetTransceiverProperty(int transceiverIndex, AdapterTransceiverProperty transceiverPropertyType,
		void* outBuffer, uint32_t& bufferSize);

	// Deprecated: use `GetPortProperty()` instead.
	bool GetPortInfo(int portIndex, AdapterPortProperty infoType, void* outBuffer, uint32_t& bufferSize)
	{
		return GetPortProperty(portIndex, infoType, outBuffer, bufferSize);
	}

protected:
	struct Impl;
	Impl* impl_ = nullptr;
};

} // Adnacom::Api namespace
