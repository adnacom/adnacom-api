/*!
 * \file `pcisw_defs.h`
 * Adnacom PCI Host Adapter diagnostics API data structures definition.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <stdint.h>


namespace Adnacom::Api {;

/*!
 * Describes available types of Host Adapter properties used in `HostAdapter::GetAdapterInfo()`.
 */
enum class HostAdapterProperty
{
	/*!
	 * Generic type for board status information;
	 * will automatically determine and return correct data structure for the specific board.
	 */
	BoardStatus = 0,
	BoardStatusH18,
	BoardStatusH14,
	BoardStatusR34,
	// ...
	Max
};

/*!
 * Describes available types of Host Adapter Port properties used in `HostAdapter::GetPortInfo()`.
 */
enum class HostAdapterPortProperty
{
	PortStatus,
	// ...
	Max
};



enum class AdapterBoardType : uint8_t
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

enum class AdapterPortLinkSpeed : uint8_t
{
	Gen1 = 1,
	Gen2 = 2,
	Gen3 = 3,
};

enum class AdapterMode : uint8_t
{
	Host = 0,
	Remote = 1,
};

enum class AdapterBoardState : uint8_t
{
	Standby = 0,
	PoweringUp = 1,
	Reset = 2,
	PexInit = 3,
	On = 3,
};


#pragma pack(push, 1)

/*!
 * Describes current Host Adapter Port's link status and error counters.
 * Corresponds to `HostAdapterPortProperty::PortStatus`.
 */
struct AdapterPortStatus
{
	AdapterPortLinkSpeed negotiatedLinkSpeed;
	AdapterPortLinkSpeed maxLinkSpeed;
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
static_assert(sizeof(AdapterPortStatus) == 0x20);

//! \section Adapter-specific properties.

struct H18Status
{
	// 0x0
	uint8_t  downstreamPortCount;
	uint8_t  upstreamPortNumber;
	uint8_t  dipSwCount : 4;
	AdapterMode mode : 4;
	AdapterBoardState boardState : 4;
	uint8_t  rsvd : 4;
	// 0x4
	uint8_t  dipS1Status;
	uint8_t  dipS2Status;
	uint8_t  cpuTemperature;
	uint8_t  pexTemperature;
	// 0x8
	uint64_t boardTime;
	// 0x10
	uint16_t resetLength;
	uint16_t pciEnumerationTime;

	// 0x14
	uint16_t mVValue;
	uint16_t slot3v3;

	//0x18
	uint16_t voltage12Vrail;
	uint16_t voltage3V3aux;

	// 0x1c
	uint16_t cpuVoltage;
	uint16_t pexVoltage;

	// 0x20
	uint16_t pex0v9Voltage;
	uint16_t pex0v9VA;

	// 0x24
	uint16_t current12vRail;
	uint16_t current3v3aux;
};
static_assert(sizeof(H18Status) == 0x28);

struct H14Status
{
	// 0x0
	uint8_t  downstreamPortCount;
	uint8_t  upstreamPortNumber;
	uint8_t  dipSwCount : 4;
	AdapterMode mode : 4;
	AdapterBoardState boardState : 4;
	uint8_t  rsvd : 4;
	// 0x4
	uint8_t  dipS1Status;
	uint8_t  rsvd0;
	uint8_t  cpuTemperature;
	uint8_t  pexTemperature;
	// 0x8
	uint64_t boardTime;
	// 0x10
	uint16_t resetLength;
	uint16_t pciEnumerationTime;

	// 0x14
	uint16_t mVValue;
	uint16_t rsvd1;

	//0x18
	uint16_t voltage12Vrail;
	uint16_t voltage3V3aux;

	// 0x1c
	uint16_t cpuVoltage;
	uint16_t pexVoltage;

	// 0x20
	uint16_t pex0v9Voltage;
	uint16_t pex0v9VA;

	// 0x24
	uint16_t rsvd2;
	uint16_t rsvd3;
};
static_assert(sizeof(H14Status) == 0x28);

struct R34Status
{
	// 0x0
	uint8_t  dipS1Status;
	uint8_t  rsvd0;
	uint8_t  cpuTemperature;
	uint8_t  pexTemperature;

	// 0x4
	uint16_t mVValue;
	uint16_t rsvd1;

	// 0x8
	uint64_t boardTime;

	// 0x10
	uint16_t voltage12Vrail;
	uint16_t voltage3V3aux;

	// 0x14
	uint16_t cpuVoltage1v8;
	uint16_t pexVoltage1v8;

	//0x18
	uint16_t pexVoltage0v9;
	uint16_t pexVA0v9;

	// 0x1c
	uint16_t current12vRail;
	uint16_t current3v3aux;

	// 0x20
	uint16_t voltage5vsbRail;
	uint16_t current5vsbRail;

	// 0x24
	int8_t u89Temperature;
	int8_t u90Temperature;
	int8_t u91Temperature;
	int8_t u92Temperature;
};
static_assert(sizeof(R34Status) == 0x28);

#pragma pack(pop)


// FORMATTING HELPERS

constexpr inline
const char* AsString(AdapterBoardType boardType)
{
	using enum AdapterBoardType;
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
