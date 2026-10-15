# Adnacom Host Adapter Diagnostics API
`pcisw-api` library provides a C++ API to discover and interact with Adnacom PCI Express Host Adapter devices on Windows systems.
Its functionality includes:
* enumerating Host Adapter devices
* enumerating Host Adapter's ports
* reading port's properties

## Usage

### Prerequisites
Adnacom Monitor v1.2 or higher must be installed on the system for the API to work correctly.

### Using in a C++ code
Include the [<pcisw/pcisw_api.h>](https://github.com/adnacom/adnacom-api/blob/master/include/pcisw/pcisw_api.h) header file which contains library's interface definitions.

All library functions and types are enclosed in the `Adnacom::Api` namespace.

`HostAdapter` is the central class of the API that represents Host Adapter instances. Static member function `HostAdapter::GetAdapterIds()` can be used to retrieve a list of available adapters:
```c++
using namespace Adnacom::Api;
// List available adapters' IDs.
auto adapterIds = HostAdapter::GetAdapterIds();
// Create Host Adapter object.
HostAdapter ad{adapterIds[0]};
```

To read property data for a specific Host Adapter's port index, `HostAdapter::GetPortInfo()` should be used. To get the number of ports in a Host Adapter, call `HostAdapter::GetPortCount()`

```c++
uint8_t buffer[BufferSize];
unsigned bufferSize = sizeof buffer;
bool ok = ad.GetPortInfo(portIndex, HostAdapterPortProperty::PortStatus, &status, bufferSize);
```

See [demo/main.cpp](demo/main.cpp) for a complete example.

**Note:** executable should be linked with the `libpciswapi.lib` static library.

## Building
The repository contains a Visual Studio project and solution files required to build the static library and a demo application.
Project may also be built from the command line using `msbuild`:
```cmd
msbuild /p:Platform=x64,Configuration=Release pcisw-api.slnx
```

Other build systems may also be used, however, additional build scripts are not currently provided in this repository. Please note that a MIDL compiler must be used during build to generate the RPC client stub code.

### Build Configuration

When the library is built as part of another project using Msbuild or Visual Studio,
it is sometimes required to use a specific Runtime Library type. 

`pcisw-api` library can be configured to match desired Runtime Library type and other build settings.
To apply a custom configuration, create a `pcisw-api-config.props` in your project's solution directory and set the following properties ([pcisw-api-config.props](\pcisw-api-config.props) from this repository may be used as a reference):
* `PciswApiRuntimeLib`(override C++ runtime library type):
  * `MultiThreadedDebug`/`MultiThreadedDebugDLL`/`MultiThreaded`/`MultiThreadedDLL` (same as `ClCompile`/`RuntimeLibrary`)
  * `Static`: use `MultiThreadedDebug` for Debug builds, and `MultiThreaded` for Release
  * `Dynamic`: use `MultiThreadedDebugDLL` for Debug builds, and `MultiThreadedDLL` for Release
* `PciswCxxStandard` (override C++ language standard):
  * `stdcpp17`: C++17
  * `stdcpp20`: C++20
  * `stdcpp23`: C++23 (currently a preview version)
