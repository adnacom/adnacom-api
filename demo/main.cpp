/**
 * \file `main.cpp`
 * Demo application for Adnacom Host Adapter API.
 */
#include <cstdlib>

#include <pcisw/pcisw_api.h>
#include <pcisw/pcisw_defs.h>

#include <print>


int main(int argc, char* argv[])
{
	using namespace Adnacom::Api;

	// Retrieve IDs of adapters currently available in our system.
	auto adapterIds = HostAdapter::GetAdapterIds();

	if (adapterIds.empty()) {
		std::println("No Adapters detected.");
		return -1;
	}

	std::println("Available Host Adapters:");

	for (auto& id : adapterIds) {
		// Create a Host Adapter object for each adapter.
		HostAdapter ad{ adapterIds.front() };
		auto boardType = ad.GetBoardType();

		std::println("> {} [{}] -- {} ports", id, AsString(boardType), HostAdapter{id}.GetPortCount());

		// Iterate over adapter's ports and print link speed for each of them.
		for (int i = 0; i < ad.GetPortCount(); ++i) {
			// This variable will be used as an output buffer for `HostAdapter::GetPortInfo()`.
			AdapterPortStatus status{};
			unsigned bufferSize = sizeof status;
			auto succeeded = ad.GetPortInfo(i, HostAdapterPortProperty::PortStatus, &status, bufferSize);
			if (succeeded)
				std::println(">> port {}: link @ {} {}", i, (int)status.negotiatedLinkSpeed, (int)status.negotiatedLinkWidth);
			else
				std::println(">> port {}: [!] ERROR", i);
		}
	}

	return 0;
}
