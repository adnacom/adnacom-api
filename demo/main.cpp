/**
 * \file `main.cpp`
 * Demo application for Adnacom Host Adapter API.
 */
#include <cstdlib>

#include <pcisw/pcisw_api.h>
#include <pcisw/pcisw_defs.h>

#include <print>

template <> struct std::formatter<Adnacom::Api::AdapterMode> : std::formatter<const char*> {
	using Mode = Adnacom::Api::AdapterMode;
	auto format(Mode mode, std::format_context& ctx) const {
		const char* text = [mode] {
			switch (mode) {
			case Mode::Host: return "Host";
			case Mode::Remote: return "Remote";
			}
			return "<unknown>";
		}();
		return std::formatter<const char*>::format(text, ctx);
	}
};
template <> struct std::formatter<Adnacom::Api::AdapterBoardState> : std::formatter<const char*> {
	using BoardState = Adnacom::Api::AdapterBoardState;
	auto format(BoardState st, std::format_context& ctx) const {
		const char* text = [st] {
			switch (st) {
			case BoardState::Standby: return "Standby";
			case BoardState::PoweringUp: return "PoweringUp";
			case BoardState::Reset: return "Reset";
			case BoardState::PexInit: return "PexInit";
			case BoardState::On: return "On";
			}
			return "<unknown>";
		}();
		return std::formatter<const char*>::format(text, ctx);
	}
};
template <> struct std::formatter<Adnacom::Api::TransceiverType> : std::formatter<const char*> {
	using TransceiverType = Adnacom::Api::TransceiverType;
	auto format(TransceiverType tt, std::format_context& ctx) const {
		const char* text = [tt] {
			switch (tt) {
			case TransceiverType::Qsfp: return "QSFP";
			case TransceiverType::Sfp: return "SFP";
			case TransceiverType::Csfp: return "CSFP";
			}
			return "<unknown>";
		}();
		return std::formatter<const char*>::format(text, ctx);
	}
};
template <> struct std::formatter<Adnacom::Api::TransceiverTechnology> : std::formatter<const char*> {
	using Tech = Adnacom::Api::TransceiverTechnology;
	auto format(Tech t, std::format_context& ctx) const {
		const char* text = [t] {
			switch (t) {
			case Tech::Vcsel850nm: return "850 nm VCSEL";
			case Tech::Vcsel1310nm: return "1310 nm VCSEL";
			case Tech::Vcsel1550nm: return "1550 nm VCSEL";
			case Tech::Fp1310nmFP: return "1310 nm FP";
			case Tech::Dfb1310nm: return "1310 nm DFB";
			case Tech::Dfb1550nm: return "1550 nm DFB";
			case Tech::Eml1310nm: return "1310 nm EML";
			case Tech::Eml1550nm: return "1550 nm EML";
			case Tech::Others: return "Others";
			case Tech::Dfb1490nm: return "1490 nm DFB";
			case Tech::CopperCableUnequalized: return "Copper cable unequalized";
			case Tech::CopperCablePassiveEqualized: return "Copper cable passive equalized";
			case Tech::CopperCableNearFarEndLimitActive: return "Copper cable, near and far end limiting active equalizers";
			case Tech::CopperCableFarEndLimitActive: return "Copper cable, far end limiting active equalizers";
			case Tech::CopperCableNearEndLimitActive: return "Copper cable, near end limiting active equalizers";
			case Tech::CopperCableLinearActive: return "Copper cable, linear active equalizers";
			}
			return "<unknown>";
		}();
		return std::formatter<const char*>::format(text, ctx);
	}
};


int main(int argc, char* argv[])
{
	using namespace Adnacom::Api;

	// Retrieve IDs of adapters currently available in our system.
	auto adapterIds = Adapter::GetAdapterIds();

	if (adapterIds.empty()) {
		std::println("No Adapters detected.");
		return -1;
	}

	std::println("Available Host Adapters:");

	for (auto& id : adapterIds) {
		// Create a Host Adapter object for each adapter.
		Adapter ad{ id };
		auto boardType = ad.GetAdapterType();

		std::println("> {} [{}] -- {} ports", id, AsString(boardType), ad.GetPortCount());

		if (boardType == AdapterType::H18) {
			H18Status adStatus;
			unsigned statusSize = sizeof adStatus;
			ad.GetAdapterProperty(AdapterProperty::BoardStatusH18, &adStatus, statusSize);

			std::println("> H18: {} DS ports, US port = {}; {} DIP switches",
				adStatus.downstreamPortCount, adStatus.upstreamPortNumber, (int)adStatus.dipSwCount);
			std::println(">    | mode: {}, board state: {}", (AdapterMode)adStatus.mode, (AdapterBoardState)adStatus.boardState);
		}

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < ad.GetPortCount(); ++i) {
			// This version of `GetPortProperty()` returns std::expected containing either underlying data structure or an error code.
			auto status = ad.GetPortProperty<AdapterPortProperty::PortStatus>(i);
			if (status)
				std::println(">> port {}: link @ {} {}", i, (int)status->negotiatedLinkSpeed, (int)status->negotiatedLinkWidth);
			else
				std::println(">> port {}: [!] ERROR {}", i, (int)status.error());
		}

		const int MaxTransceivers = 4;

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < MaxTransceivers; ++i) {
			auto caps = ad.GetTransceiverProperty<AdapterTransceiverProperty::DeviceCapabilities>(i);
			if (!caps)
				break;

			caps->capabilities;

			std::println(">> transceiver {}> type: {}, technology: {}", i, (TransceiverType)caps->transceiverType, (TransceiverTechnology)caps->deviceTechnology);
		}
	}

	return 0;
}
