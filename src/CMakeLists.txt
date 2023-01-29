# CMakeList.txt: proyecto de CMake para D3D12HelloTriangle, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (D3D12HelloTriangle "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET D3D12HelloTriangle PROPERTY CXX_STANDARD 20)
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.
