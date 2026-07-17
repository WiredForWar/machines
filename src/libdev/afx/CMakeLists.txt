set(AFX_SOURCES
    AfxConfiguration.cpp
    AfxConfigurationData.hpp
    AfxConfiguration.hpp
    AfxConfigurationLoader.cpp
    AfxConfigurationLoader.hpp
    app.cpp
    app.hpp
    invkargs.cpp
    invkargs.hpp
    printmsg.hpp
    resource.cpp
    resource.hpp
)

# afx_lite is used with test suite
add_library(afx_lite OBJECT ${AFX_SOURCES})

add_library(afx_lite_iface INTERFACE)
target_link_libraries(afx_lite_iface INTERFACE afx_lite $<TARGET_OBJECTS:afx_lite>)

add_library(afx OBJECT
    AfxSdlApp.cpp
    AfxSdlApp.hpp
)
target_link_libraries(afx PUBLIC
    afx_lite_iface
    SDL3::SDL3
)

add_library(afx_iface INTERFACE)
target_link_libraries(afx_iface INTERFACE afx $<TARGET_OBJECTS:afx>)

target_link_libraries(afx_lite_iface INTERFACE
    LibDev::base
    LibDev::device
    LibDev::render
    LibDev::recorder
    LibDev::system
)

add_library(LibDev::afx_lite ALIAS afx_lite_iface)
add_library(LibDev::afx ALIAS afx_iface)
