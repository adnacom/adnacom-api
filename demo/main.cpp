/**
 * \file `main.cpp`
 * Demo application for Adnacom Host Adapter API.
 */
#include <cstdlib>

#include <pcisw/pcisw_api.h>
#include <pcisw/pcisw_defs.h>

#include <iostream>

using Adnacom::Api::AsString;

#if __cplusplus >= 202302L
#include <print>

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

#endif // C++23

// Detect if a type can be converted to string via `AsString(val)`.
template <typename Ty, typename = void> struct IsConvertibleToString : std::false_type {};
template <typename Conv> struct IsConvertibleToString<Conv, std::enable_if_t<std::is_same_v<decltype(AsString(std::declval<Conv&>())), const char*>>> : std::true_type {};

template <typename Ty, typename = std::enable_if_t<IsConvertibleToString<Ty>::value>> inline
std::ostream& operator<<(std::ostream& os, Ty val)
{
	return os << AsString(val);
}


int main(int argc, char* argv[])
{
	using namespace Adnacom::Api;

	// Retrieve IDs of adapters currently available in our system.
	auto adapterIds = Adapter::GetAdapterIds();

	if (adapterIds.empty()) {
		std::cerr << "No Adapters detected." << std::endl;
		return -1;
	}

	std::cout << "Available Host Adapters:" << std::endl;

	for (auto& id : adapterIds) {
		// Create a Host Adapter object for each adapter.
		Adapter ad{ id };
		auto boardType = ad.GetAdapterType();

		std::cout << "> " << id << " [" << boardType << "] -- " << ad.GetPortCount() << " ports" << std::endl;

		if (boardType == AdapterType::H18) {
			H18Status adStatus;
			unsigned statusSize = sizeof adStatus;
			ad.GetAdapterProperty(AdapterProperty::BoardStatusH18, &adStatus, statusSize);

			std::cout << "> H18: " << static_cast<unsigned>(adStatus.downstreamPortCount) << " DS ports, US port = " << static_cast<unsigned>(adStatus.upstreamPortNumber) << "; " << static_cast<unsigned>(adStatus.dipSwCount) << " DIP switches" << std::endl;
			std::cout << ">    | mode: " << (AdapterMode)adStatus.mode << ", board state: " << (AdapterBoardState)adStatus.boardState << std::endl;
		}

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < ad.GetPortCount(); ++i) {
			// This version of `GetPortProperty()` returns std::expected containing either underlying data structure or an error code.
			auto status = ad.GetPortProperty<AdapterPortProperty::PortStatus>(i);
			if (status)
				std::cout << ">> port " << i << ": link @ " << (int)status->negotiatedLinkSpeed << " " << (int)status->negotiatedLinkWidth << std::endl;
			else
				std::cout << ">> port " << i << ": [!] ERROR " << (int)status.error() << std::endl;
		}

		const int MaxTransceivers = 2;

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < MaxTransceivers; ++i) {
			auto caps = ad.GetTransceiverProperty<AdapterTransceiverProperty::DeviceCapabilities>(i);
			if (!caps)
				break;

			if (caps->revisionNumber[0] == 0 && caps->revisionNumber[1] == 0 || (uint8_t)caps->capabilities == 0)
				std::cout << ">> transceiver " << i << "> (not present)" << std::endl;
			else
				std::cout << ">> transceiver " << i << "> rev " << caps->revisionNumber[0] << "." << caps->revisionNumber[1]
					<< ", caps: " << std::hex << (unsigned)caps->capabilities << std::dec << ", type: " << (TransceiverType)caps->transceiverType
					<< ", technology: " << (TransceiverTechnology)caps->deviceTechnology << std::endl;
		}
	}

	return 0;
}
