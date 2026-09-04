/**
 * \file `main.cpp`
 * Demo application for Adnacom Host Adapter API.
 */
#include <cstdlib>

#include <pcisw/pcisw_api.h>
#include <pcisw/pcisw_defs.h>

#include <print>

using Adnacom::Api::AsString;

template <typename Ty> concept ConvertibleToString = requires(const Ty& val)
{
	{ AsString(val) } -> std::convertible_to<const char*>;
};

// Define generic formatter for `std::format` based on the `AsString()` function.
template <ConvertibleToString Ty> struct std::formatter<Ty> : std::formatter<const char*>
{
	auto format(Ty val, std::format_context& ctx) const {
		return std::formatter<const char*>::format(AsString(val), ctx);
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

		std::println("> {} [{}] -- {} ports", id, boardType, ad.GetPortCount());

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

		const int MaxTransceivers = 2;

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < MaxTransceivers; ++i) {
			auto caps = ad.GetTransceiverProperty<AdapterTransceiverProperty::DeviceCapabilities>(i);
			if (!caps)
				break;

			if (caps->revisionNumber[0] == 0 && caps->revisionNumber[1] == 0 || (uint8_t)caps->capabilities == 0)
				std::println(">> transceiver {}> (not present)", i);
			else
				std::println(">> transceiver {}> rev {}.{}, caps: {:x}, type: {}, technology: {}", i, caps->revisionNumber[0], caps->revisionNumber[1],
					(unsigned)caps->capabilities, (TransceiverType)caps->transceiverType, (TransceiverTechnology)caps->deviceTechnology);
		}
	}

	return 0;
}
