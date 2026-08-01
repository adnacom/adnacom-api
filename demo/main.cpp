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
			// This variable will be used as an output buffer for `HostAdapter::GetPortInfo()`.
			AdapterPortStatus status{};
			unsigned bufferSize = sizeof status;
			auto succeeded = ad.GetPortProperty(i, AdapterPortProperty::PortStatus, &status, bufferSize);
			if (succeeded)
				std::println(">> port {}: link @ {} {}", i, (int)status.negotiatedLinkSpeed, (int)status.negotiatedLinkWidth);
			else
				std::println(">> port {}: [!] ERROR", i);
		}
	}

	return 0;
}
