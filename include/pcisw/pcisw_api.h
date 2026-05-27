/*!
 * \file `pcisw_api.h`
 * Adnacom PCI Host Adapter diagnostics API.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <cstdint>
#include <string>
#include <vector>


namespace Adnacom::Api {;

/*!
 * Type used as a Host Adapter ID.
 * Should be treated as an opaque datatype, as it may change in the future.
 * Application developers should not make assumptions about this type or
 * the Adapter ID values returned by the API.
 */
using HostAdapterId = std::string;

/*!
 * Describes available types of Host Adapter Port properties used in `HostAdapter::GetPortInfo()`.
 */
enum class HostAdapterPortProperty
{
	PortStatus,
	// ...
	Max
};

/*!
 * Describes an Adnacom Host Adapter instance.
 */
class HostAdapter
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
	virtual ~HostAdapter() = default;

	/*!
	 * Returns number of available ports on the adapter.
	 * \returns [int] - Port count (upstream and downstream). Returns negative value in case of error (e.g. invalid adapter ID).
	 */
	int GetPortCount() const;

	/*!
	 * Reads adapter port's property value.
	 * \param[in] portIndex - Port index. Should not exceed the value returned by `GetPortCount()`.
	 * \param[in] infoType - Type of property to be retrieved, \see `HostAdapterPortProperty`.
	 * \param[out] outBuffer - Data buffer where the retrieved data will be placed.
	 * \param[in,out] bufferSize - On input: size of the `outBuffer`, in bytes. On output: number of bytes written to the output buffer.
	 *		If the output buffer is too small to hold the data, `bufferSize` will contain the minimum required size of an output buffer.
	 * \returns [bool] - `true` on success, `false` on error.
	 */
	bool GetPortInfo(int portIndex, HostAdapterPortProperty infoType, void* outBuffer, uint32_t& bufferSize);

protected:
	HostAdapterId id_;
};


/*!
 * Section below contains descriptions of Adapter Port Properties
 * declared above in `HostAdapterPortProperty`.
 */

enum class PortLinkSpeed : uint8_t
{
	Gen1 = 1,
	Gen2 = 2,
	Gen3 = 3,
};

#pragma pack(push, 1)

/*!
 * Describes current Host Adapter Port's link status and error counters.
 * Corresponds to `HostAdapterPortProperty::PortStatus`.
 */
struct HostAdapterPortStatus
{
	PortLinkSpeed negotiatedLinkSpeed;
	PortLinkSpeed maxLinkSpeed;
	uint8_t negotiatedLinkWidth;
	uint8_t maxLinkWidth;

	uint32_t recoveryCounter;
	uint32_t rxErrorCounter;
	uint32_t badTlp;
	uint32_t badDll;
	uint16_t linkUpTime;
	uint16_t remoteUpTime;
	uint16_t widthRetrain;
	uint16_t linkDown;

	uint32_t framingErrors;
};
static_assert(sizeof(HostAdapterPortStatus) == 0x20);

#pragma pack(pop)


} // Adnacom::Api namespace
