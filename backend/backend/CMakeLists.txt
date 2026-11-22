cmake_minimum_required(VERSION 3.14)
project(trading_engine CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Python / pybind11 (embedded interpreter bridge to data_service) ---
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _pybind11_result
    )
    if(NOT _pybind11_result EQUAL 0)
        message(FATAL_ERROR "pybind11 not found. Install it with: pip install pybind11")
    endif()
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Site-packages of the selected Python, so the embedded interpreter used by
# the tests can import data_service's dependencies (yfinance, pandas, ...).
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE Python3_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# --- spdlog (logging) ---
FetchContent_Declare(spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.tar.gz)
FetchContent_MakeAvailable(spdlog)

# --- nlohmann_json (config parsing) ---
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(nlohmann_json)

include_directories(${CMAKE_SOURCE_DIR}/include)

# --- Engine executable ---
set(SOURCES src/main.cpp src/data_loader.cpp src/risk_manager.cpp src/strategy.cpp src/order_executor.cpp)
# pybind11 translation units are very slow to optimize; -O1 keeps builds fast
set_source_files_properties(src/data_loader.cpp src/main.cpp src/bindings.cpp PROPERTIES COMPILE_OPTIONS "-O1")
add_executable(trading_engine ${SOURCES})
target_compile_definitions(trading_engine PRIVATE AURUM_PYTHON_SITE_PACKAGES="${Python3_SITE_PACKAGES}")
target_link_libraries(trading_engine
    PRIVATE
        pybind11::embed
        spdlog::spdlog_header_only
        nlohmann_json::nlohmann_json
        pthread
)

# --- Tests ---
option(BUILD_TESTING "Build the test suite" ON)
if(BUILD_TESTING)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)

    add_executable(test_risk_manager tests/test_risk_manager.cpp src/risk_manager.cpp)
    target_link_libraries(test_risk_manager
        PRIVATE GTest::gtest_main spdlog::spdlog_header_only pthread)

    add_executable(test_data_loader
        tests/test_data_loader.cpp tests/test_main.cpp src/data_loader.cpp)
    target_compile_definitions(test_data_loader PRIVATE
        AURUM_DATA_SERVICE_PATH="${CMAKE_SOURCE_DIR}/.."
        AURUM_PYTHON_SITE_PACKAGES="${Python3_SITE_PACKAGES}")
    target_link_libraries(test_data_loader
        PRIVATE pybind11::embed GTest::gtest spdlog::spdlog_header_only pthread)

    gtest_discover_tests(test_risk_manager)
    gtest_discover_tests(test_data_loader PROPERTIES
        ENVIRONMENT "PYTHONPATH=${Python3_SITE_PACKAGES}")
endif()

pybind11_add_module(aurum_core src/bindings.cpp src/strategy.cpp src/risk_manager.cpp)
target_link_libraries(aurum_core PRIVATE spdlog::spdlog_header_only pthread)
set_target_properties(aurum_core PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/..")
if(BUILD_TESTING)
    add_executable(test_components tests/test_components.cpp src/strategy.cpp src/order_executor.cpp)
    target_link_libraries(test_components PRIVATE GTest::gtest_main spdlog::spdlog_header_only pthread)
    gtest_discover_tests(test_components)
endif()
