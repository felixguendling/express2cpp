![Logo](logo.svg)

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
