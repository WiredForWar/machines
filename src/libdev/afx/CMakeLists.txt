set(AFX_SOURCES
    AfxConfiguration.cpp
    AfxConfigurationData.hpp
    AfxConfiguration.hpp
    AfxConfigurationLoader.cpp
    AfxConfigurationLoader.hpp
    AfxSdlApp.hpp
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
add_library(afx OBJECT ${AFX_SOURCES} AfxSdlApp.cpp)
target_link_libraries(afx PUBLIC
    LibDev::render
    SDL2::SDL2
)
