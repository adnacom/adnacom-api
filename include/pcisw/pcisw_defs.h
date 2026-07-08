/*!
 * \file `pcisw_defs.h`
 * Adnacom PCI Host Adapter diagnostics API data structures definition.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <stdint.h>


namespace Adnacom::Api {;

/*!
 * Describes available types of Host Adapter Port properties used in `HostAdapter::GetPortInfo()`.
 */
enum class HostAdapterPortProperty
{
	PortStatus,
	// ...
	Max
};


enum class HostAdapterBoardType : uint8_t
{
	Unknown = 0,
	H18 = 1,
	R34 = 2, //!< R34 is a remote-only board.
	H14 = 3,
	H12 = 4,
	H3 = 5,
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


// FORMATTING HELPERS

constexpr inline
const char* AsString(HostAdapterBoardType boardType)
{
	using enum HostAdapterBoardType;
	switch (boardType) {
	case Unknown: return "unknown";
	case H18: return "H18";
	case R34: return "R34";
	case H14: return "H14";
	case H12: return "H12";
	case H3: return "H3";
	default:
		return "<unknown>";
	}
}


} // Adnacom::Api namespace
