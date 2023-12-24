file(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.h" "*.inl")

add_executable(RayTracing ${SOURCE_FILES})

target_include_directories(RayTracing PUBLIC ${SOURCE_DIR})

target_precompile_headers(RayTracing PUBLIC pch.h)
