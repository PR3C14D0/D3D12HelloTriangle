# CMakeList.txt: proyecto de CMake para D3D12HelloTriangle, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (D3D12HelloTriangle "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET D3D12HelloTriangle PROPERTY CXX_STANDARD 20)
  set_property(TARGET D3D12HelloTriangle PROPERTY WIN32_EXECUTABLE TRUE)
endif()

set(DX_LIB "d3d12.lib;d3dcompiler.lib;dxgi.lib") # These libraries are included with the Windows SDK.
set(GLOBAL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(SOURCE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories("${GLOBAL_INCLUDES}")
include_directories("${SOURCE_INCLUDES}")

target_link_libraries(D3D12HelloTriangle "${DX_LIB}")
