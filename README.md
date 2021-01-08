![Logo](logo.png)

![Windows Build](https://github.com/baumhaus-project/express2cpp/workflows/Windows%20Build/badge.svg)
![Unix Build](https://github.com/baumhaus-project/express2cpp/workflows/Unix%20Build/badge.svg)

# Usage

CMakeLists.txt
```cmake
express2cpp(IFC23.EXP ifc2x3)
add_executable(exe main.cc)
target_link_libraries(exe ifc2x3)
```

main.cc
```cpp
#include "IFC2X3/IfcProduct.h"
#include "IFC2X3/register_all_types.h"

auto parser = step::entry_parser{};
IFC2X3::register_all_types(parser);
auto model = step::entity_map{parser, ifc_input};
model.get_entity<IFC2X3::IfcProduct>(step::id_t{0});
```

# Supported Targets

  - GCC 10.2 (10.1 not working!)
  - Clang 11 (previous versions not tested)
  - Apple Clang 12 (previous versions not tested)
  - MSVC Latest (previous versions not tested)
