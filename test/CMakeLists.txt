include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/libdev
)

enable_testing(true)

macro(append_test test_name test_srcs)
    set(TESTS_SRCS ${TESTS_SRCS} ${test_srcs})
    set(TESTS_NAME ${TESTS_NAME} ${test_name})
endmacro(append_test)

# Can't use `libs` because some libraries had to be substituted, like `afx` ':^/
# # You can still unit test afx lib's classes, except for AfxSdlApp
set(TESTING_THESE_GUYS
    base
    mathex
    afx_lite
    crashdump
    utility
    profiler
    system
    device
    world4d
    gxin
    xin
    stdlib
    render
    sound
    network
    ctl
    phys
    sim
    gui
    recorder
    envirnmt
    machphys
    machlog
    machgui
)

#### Test Cases
#~:afx
append_test(AfxConfigurationTests afx/AfxConfigurationTests.cpp)
append_test(AfxInvokeArgsTests afx/AfxInvokeArgsTests.cpp)
#~:ani
if(USE_SWSCALE)
    append_test(FrameScalerTests ani/FrameScalerTests.cpp)
endif()
#~:crashdump
append_test(WatchdogTests crashdump/WatchdogTests.cpp)
#~:device
append_test(BasePersistenceTests base/PersistenceTests.cpp)
append_test(DevButtonEventTests device/DevButtonEventTests.cpp)
append_test(DevEventQueueTests device/DevEventQueueTests.cpp)
append_test(DevInputTests device/DevInputTests.cpp)
append_test(DevInputStateTests device/DevInputStateTests.cpp)
append_test(DevKeyNamesTests device/DevKeyNamesTests.cpp)
append_test(DevKeyWithModifiersTests device/DevKeyWithModifiersTests.cpp)
append_test(DevMouseTests device/DevMouseTests.cpp)
append_test(DevKeyToCommandTranslator device/DevKeyToCommandTranslator.cpp)
append_test(DevMusicResumeStoreTests device/MusicResumeStoreTests.cpp)
#~:gui
append_test(GuiBaselineTests gui/GuiBaselineTests.cpp)
append_test(GuiRootSharedBitmapsTests gui/GuiRootSharedBitmapsTests.cpp)
append_test(GuiResourceStringTests gui/GuiResourceStringTests.cpp)
append_test(GuiDisplayableGeometryTests gui/GuiDisplayableGeometryTests.cpp)
append_test(ScreenshotsTests gui/ScreenshotsTests.cpp)
#~:machgui
append_test(DropDownPlacementTests machgui/DropDownPlacementTests.cpp)
append_test(MouseLookTests machgui/MouseLookTests.cpp)
append_test(ScaleFactorTests machgui/ScaleFactorTests.cpp)
#~:mathex
append_test(MexLine3dTests mathex/MexLine3dTests.cpp)
#~:network
append_test(NetUtilsTest network/NetUtilsTests.cpp)
#~:system
append_test(SysPathNameTests system/SysPathNameTests.cpp)
#~:utility
append_test(UtlBoundedIdGeneratorTests utility/BoundedIdGeneratorTests.cpp)

append_test(ColourTests render/ColourTests.cpp)
append_test(DisplayModeCatalogueTests render/DisplayModeCatalogueTests.cpp)
append_test(TextWrapTests render/TextWrapTests.cpp)

#### Create executables
list(LENGTH TESTS_SRCS len1)
math(EXPR len2 "${len1} - 1")
foreach( val RANGE ${len2} )
    list(GET TESTS_SRCS ${val} test_src)
    list(GET TESTS_NAME ${val} test_name)
    add_executable(${test_name} AssertionOverrides.cpp RenSurfaceOverrides.cpp ${test_src})
    target_link_libraries( ${test_name}
        gtest
        gmock
        gtest_main
        ${TESTING_THESE_GUYS}
    )
    if(MSVC)
        # AssertionOverrides.cpp and RenSurfaceOverrides.cpp replace the library
        # implementations, which are only weak symbols when building with GCC
        # (see WEAK_SYMBOL in base/compiler.hpp). MSVC has no weak symbols, so
        # ask the linker to keep the first definition instead of reporting a
        # duplicate one; the overrides are linked before the libraries.
        target_link_options(${test_name} PRIVATE /FORCE:MULTIPLE)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
