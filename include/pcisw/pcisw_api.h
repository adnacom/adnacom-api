/*!
 * \file `pcisw_api.h`
 * Adnacom PCIe Adapter diagnostics API.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <cstdint>
#include <string>
#include <vector>
#include <expected>

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
	 * \param[in] propertyType - Type of the property to be retrieved, see `AdapterProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes; on output: size of the requested property, in bytes.
	 *		If `outBuffer` is non-null, writes the requested property data into the buffer; maximum number of bytes written
	 *		is limited by the property size. If the output buffer is smaller than the property size, the output data is truncated
	 *		to fit into the buffer, and the function returns successfully.
	 *		To get the property data size without reading the property, pass `nullptr` as an output buffer, then use the value returned
	 *		in `*bufferSize`.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure. Possible error codes include:
	 *		`NotFound` - returned when the specified adapter is not found. Make sure the device is connected and is operating correctly,
	 *			and the device ID used to create the Adapter object is valid.
	 *		`InvalidParameter` - returned when the supplied `propertyType` is not recognized, or `outBuffer` is null.
	 *		`OutOfMemory` - insufficient system resources to complete the operation.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetAdapterProperty(AdapterProperty propertyType, void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;

	/*!
	 * Reads adapter's property value.
	 * Type of the property value is deduced automatically at compile time based on the property type `Pr`.
	 * \tparam	Pr	Type of the property to be retrieved, see \ref `AdapterProperty`.
	 * \returns [std::expected] - `std::expected` containing either the retrieved property value
	 *		or an error code on failure.
	 */
	template <AdapterProperty Pr>
	auto GetAdapterProperty() const
		-> std::expected<AdapterPropertyDataType<Pr>, ErrorCode>
	{
		AdapterPropertyDataType<Pr> outData;
		uint32_t size = sizeof outData;
		ErrorCode err = ErrorCode::Ok;
		if (GetAdapterProperty(Pr, &outData, size, &err))
			return outData;

		return std::unexpected{err};
	}

	/*!
	 * Reads adapter port's property value.
	 * \param[in] portIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] propertyType - Type of property to be retrieved, see `AdapterPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes; on output: size of the requested property, in bytes.
	 *		If `outBuffer` is non-null, writes the requested property data into the buffer; maximum number of bytes written
	 *		is limited by the property size. If the output buffer is smaller than the property size, the output data is truncated
	 *		to fit into the buffer, and the function returns successfully.
	 *		To get the property data size without reading the property, pass `nullptr` as an output buffer, then use the value returned
	 *		in `*bufferSize`.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure. Possible error codes include:
	 *		`NotFound` - returned when the specified port is not found. Make sure the supplied port index is valid.
	 *		`InvalidParameter` - returned when the supplied `propertyType` is not recognized, or `outBuffer` is null.
	 *		`OutOfMemory` - insufficient system resources to complete the operation.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetPortProperty(int portIndex, AdapterPortProperty propertyType, void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;


	/*!
	 * Reads adapter port's property value.
	 * Type of the property value is deduced automatically at compile time based on the property type `Pr`.
	 * \tparam	Pr	- Type of the property to be retrieved, see `AdapterPortProperty`.
	 * \param[in]	portIndex	- Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \returns [std::expected] - `std::expected` containing either the retrieved property value
	 *		or an error code on failure.
	 */
	template <AdapterPortProperty Pr>
	auto GetPortProperty(int portIndex) const
		-> std::expected<AdapterPortPropertyDataType<Pr>, ErrorCode>
	{
		AdapterPortPropertyDataType<Pr> outData;
		uint32_t size = sizeof outData;
		ErrorCode err = ErrorCode::Ok;
		if (GetPortProperty(portIndex, Pr, &outData, size, &err))
			return outData;

		return std::unexpected{err};
	}

	/*!
	 * Reads property of adapter's transceiver.
	 * \param[in] transceiverIndex - Transceiver index.
	 * \param[in] transceiverPropertyType - Type of property to be retrieved, see `AdapterTransceiverProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes; on output: size of the requested property, in bytes.
	 *		If `outBuffer` is non-null, writes the requested property data into the buffer; maximum number of bytes written
	 *		is limited by the property size. If the output buffer is smaller than the property size, the output data is truncated
	 *		to fit into the buffer, and the function returns successfully.
	 *		To get the property data size without reading the property, pass `nullptr` as an output buffer, then use the value returned
	 *		in `*bufferSize`.
	 * \param[out] resultCode	- Optional parameter; if non-null, receives a result code on return: `Ok` on success or an error code
	 *		on failure. Possible error codes include:
	 *		`NotFound` - returned when the specified transceiver is not found. Make sure the specified index is correct.
	 *		`InvalidParameter` - returned when the supplied `propertyType` is not recognized, or `outBuffer` is null.
	 *		`OutOfMemory` - insufficient system resources to complete the operation.
	 * \returns [bool] - `true` on success, `false` on error. If the output data is truncated, the function will still
	 *		return `true` indicating success.
	 */
	bool GetTransceiverProperty(int transceiverIndex, AdapterTransceiverProperty transceiverPropertyType,
		void* outBuffer, uint32_t& bufferSize, ErrorCode* resultCode = nullptr) const;

	/*!
	 * Reads property of adapter's transceiver.
	 * Type of the property value is deduced automatically at compile time based on the property type `Pr`.
	 * \tparam Pr - Type of the property to be retrieved, see `AdapterTransceiverProperty`.
	 * \param[in] transceiverIndex - Transceiver index.
	 * \returns [std::expected] - `std::expected` containing either the retrieved property value
	 *		or an error code on failure.
	 */
	template <AdapterTransceiverProperty Pr>
	auto GetTransceiverProperty(int transceiverIndex) const
		-> std::expected<AdapterTransceiverPropertyDataType<Pr>, ErrorCode>
	{
		AdapterTransceiverPropertyDataType<Pr> outData;
		uint32_t size = sizeof outData;
		ErrorCode err = ErrorCode::Ok;
		if (GetTransceiverProperty(transceiverIndex, Pr, &outData, size, &err))
			return outData;

		return std::unexpected{err};
	}


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
