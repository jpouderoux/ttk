cmake_minimum_required(VERSION 2.4)

# name of the project (dummy)
project(blankCmd)

# command-line build-mode
set(TTK_BUILD_MODE "VTK-CMD")

# path to the common code base
find_path(TTK_DIR ttk.cmake
  PATHS
    core/
    ../core/
    ../../core/
    ../../../core/
)
include(${TTK_DIR}/ttk.cmake)

# include the different packages required for the application
ttk_add_vtkWrapper_package(vtkBlank)

# add the local cpp files 
ttk_add_source(main.cpp)

# wrap-up the build
ttk_wrapup_binary(blankCmd)
