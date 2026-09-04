/*!
 * \file `pcisw_defs.h`
 * Adnacom PCIe Adapter diagnostics API data structures definition.
 * \copyright Copyright 2026 Adnacom Inc.
 */
#pragma once


#include <stdint.h>


namespace Adnacom::Api {;

enum class ErrorCode
{
	Ok = 0,
	GenericFailure,
	NotFound,
	InvalidParameter,
	OutOfMemory,
};

/*!
 * Describes available types of Adapter properties used in `Adapter::GetProperty()`.
 */
enum class AdapterProperty
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
 * Describes available types of Adapter Port properties used in `Adapter::GetPortProperty()`.
 */
enum class AdapterPortProperty
{
	PortStatus,
	// ...
	Max
};

/*!
 * Describes available types of Adapter Transceiver Port properties used in `Adapter::GetTransceiverProperty()`.
 */
enum class AdapterTransceiverProperty
{
	VendorName,
	VendorPartNumber,
	DeviceCapabilities,
	StartupTime,
	VendorSerialNumber,
	Thresholds,
	Status,
	// ...
	Max
};


enum class AdapterType : uint8_t
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
 * declared above in `AdapterPortProperty`.
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
	On = 4,
};


#pragma pack(push, 1)

/*!
 * Describes current Adapter Port's link status and error counters.
 * Corresponds to `AdapterPortProperty::PortStatus`.
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


// TRANSCEIVER PROPERTIES

enum class TransceiverTechnology : uint8_t
{
	Vcsel850nm = 0,
	Vcsel1310nm = 1,
	Vcsel1550nm = 2,
	Fp1310nmFP = 3,
	Dfb1310nm = 4,
	Dfb1550nm = 5,
	Eml1310nm = 6,
	Eml1550nm = 7,
	Others = 8,
	Dfb1490nm = 9,
	CopperCableUnequalized = 10,
	CopperCablePassiveEqualized = 11,
	CopperCableNearFarEndLimitActive = 12,
	CopperCableFarEndLimitActive = 13,
	CopperCableNearEndLimitActive = 14,
	CopperCableLinearActive = 15,
};

enum class TransceiverCapabilityFlags : uint8_t
{
	TxSquelch = 0b0000'0001,
	TxSquelchDisable = 0b0000'0010,
	RxOutputDisable = 0b0000'0100,
	RxSquelchEnable = 0b0000'1000,
	TxLossOfSignal = 0b0001'0000,
	TxSquelchReduceOMA = 0b0010'0000,
	TxFault = 0b0100'0000,
	TxDisable = 0b1000'0000,
};

enum class TransceiverType : uint8_t
{
	Qsfp = 0,
	Sfp = 1,
	Csfp = 2,
};

enum class TransceiverStatusFlags : uint8_t
{
	Present = 1,
	PowerOn = 2,
	PowerSource3V3 = 4,
};

enum class TransceiverState : uint8_t
{
	Off = 0,
	Reset = 1,
	Init = 2,
	Config = 3,
	TxOff = 4,
	On = 5,
};

enum class TransceiverTxRxStatus : uint8_t
{
	Rx1 = 1,
	Rx2 = 2,
	Rx3 = 4,
	Rx4 = 8,
	Tx1 = 0x10,
	Tx2 = 0x20,
	Tx3 = 0x40,
	Tx4 = 0x80,
};


struct TransceiverVendorName
{
	char vendorName[16];
};

struct TransceiverVendorPartNumber
{
	char vendorPartNumber[16];
};

// 0x20-0x2c: Device Capabilities

struct TransceiverDeviceCapabilities
{
	uint8_t revisionNumber[2];	// The vendor revision number ASCII characters.
	TransceiverTechnology deviceTechnology : 4;
	TransceiverType transceiverType : 4;
	TransceiverCapabilityFlags capabilities;
};

struct TransceiverStartupTime
{
	uint16_t startupTime; // In milliseconds
	uint16_t rsvd;
};

struct TransceiverVendorSerialNumber
{
	char vendorSerialNumber[16];
};

struct TransceiverThresholds
{
	int16_t tempHighAlarm;
	int16_t tempLowAlarm;
	int16_t tempHighWarning;
	int16_t tempLowWarning;

	uint16_t rxPowerHighAlarm;
	uint16_t rxPowerLowAlarm;
	uint16_t rxPowerHighWarning;
	uint16_t rxPowerLowWarning;

	uint16_t txBiasHighAlarm;
	uint16_t txBiasLowAlarm;
	uint16_t txBiasHighWarning;
	uint16_t txBiasLowWarning;

	uint16_t vccHighAlarm;
	uint16_t vccLowAlarm;
	uint16_t vccHighWarning;
	uint16_t vccLowWarning;
};

struct TransceiverStatus
{
	TransceiverStatusFlags status;
	TransceiverState state;
	TransceiverTxRxStatus txRxStatus;
	uint8_t rsvd;

	uint16_t losCounter;
	uint16_t i2cErrorCounter;

	uint16_t voltage3v3;
	uint16_t icc;

	int16_t temperature;
	uint16_t voltage;

	uint16_t rx1Power;
	uint16_t rx2Power;
	uint16_t rx3Power;
	uint16_t rx4Power;

	uint16_t tx1Bias; // Tx Bias in 500-microampere units.
	uint16_t tx2Bias;
	uint16_t tx3Bias;
	uint16_t tx4Bias;
};

#pragma pack(pop)


// PROPERTY TRAITS

template <AdapterProperty Ap> struct AdapterPropertyTraits;
template <> struct AdapterPropertyTraits<AdapterProperty::BoardStatusH18> { using Type = H18Status; };
template <> struct AdapterPropertyTraits<AdapterProperty::BoardStatusH14> { using Type = H14Status; };
template <> struct AdapterPropertyTraits<AdapterProperty::BoardStatusR34> { using Type = R34Status; };

template <AdapterPortProperty Pp> struct AdapterPortPropertyTraits;
template <> struct AdapterPortPropertyTraits<AdapterPortProperty::PortStatus> { using Type = AdapterPortStatus; };

template <AdapterTransceiverProperty Tp> struct AdapterTransceiverPropertyTraits;
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::VendorName> { using Type = TransceiverVendorName; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::VendorPartNumber> { using Type = TransceiverVendorPartNumber; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::DeviceCapabilities> { using Type = TransceiverDeviceCapabilities; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::StartupTime> { using Type = TransceiverStartupTime; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::VendorSerialNumber> { using Type = TransceiverVendorSerialNumber; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::Thresholds> { using Type = TransceiverThresholds; };
template <> struct AdapterTransceiverPropertyTraits<AdapterTransceiverProperty::Status> { using Type = TransceiverStatus; };

template <AdapterProperty AdPr> using AdapterPropertyDataType = typename AdapterPropertyTraits<AdPr>::Type;
template <AdapterPortProperty PtPr> using AdapterPortPropertyDataType = typename AdapterPortPropertyTraits<PtPr>::Type;
template <AdapterTransceiverProperty TrPr> using AdapterTransceiverPropertyDataType = typename AdapterTransceiverPropertyTraits<TrPr>::Type;

// FORMATTING HELPERS

constexpr inline
const char* AsString(AdapterType boardType)
{
	using enum AdapterType;
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

constexpr inline
const char* AsString(AdapterMode mode)
{
	using enum AdapterMode;
	switch (mode) {
	case Host: return "Host";
	case Remote: return "Remote";
	default:
		return "<unknown>";
	}
}

constexpr inline
const char* AsString(AdapterBoardState state)
{
	using enum AdapterBoardState;
	switch (state) {
	case Standby: return "Standby";
	case PoweringUp: return "PoweringUp";
	case Reset: return "Reset";
	case PexInit: return "PexInit";
	case On: return "On";
	default:
		return "<unknown>";
	}
}

constexpr inline
const char* AsString(TransceiverType type)
{
	using enum TransceiverType;
	switch (type) {
	case TransceiverType::Qsfp: return "QSFP";
	case TransceiverType::Sfp: return "SFP";
	case TransceiverType::Csfp: return "CSFP";
	default:
		return "<unknown>";
	}
}

constexpr inline
const char* AsString(TransceiverTechnology tech)
{
	using enum TransceiverTechnology;
	switch (tech) {
	case Vcsel850nm: return "850 nm VCSEL";
	case Vcsel1310nm: return "1310 nm VCSEL";
	case Vcsel1550nm: return "1550 nm VCSEL";
	case Fp1310nmFP: return "1310 nm FP";
	case Dfb1310nm: return "1310 nm DFB";
	case Dfb1550nm: return "1550 nm DFB";
	case Eml1310nm: return "1310 nm EML";
	case Eml1550nm: return "1550 nm EML";
	case Others: return "Others";
	case Dfb1490nm: return "1490 nm DFB";
	case CopperCableUnequalized: return "Copper cable unequalized";
	case CopperCablePassiveEqualized: return "Copper cable passive equalized";
	case CopperCableNearFarEndLimitActive: return "Copper cable, near and far end limiting active equalizers";
	case CopperCableFarEndLimitActive: return "Copper cable, far end limiting active equalizers";
	case CopperCableNearEndLimitActive: return "Copper cable, near end limiting active equalizers";
	case CopperCableLinearActive: return "Copper cable, linear active equalizers";
	default:
		return "<unknown>";
	}
}


} // Adnacom::Api namespace
