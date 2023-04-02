
set(GUI_SOURCES
    border.cpp
    border.hpp
    button.cpp
    button.hpp
    displaya.cpp
    displaya.hpp
    editbox.cpp
    editbox.hpp
    event.cpp
    event.hpp
    font.cpp
    font.hpp
    gui.cpp
    gui.hpp
    icon.cpp
    icon.hpp
    iconseq.cpp
    iconseq.hpp
    image.cpp
    image.hpp
    internal/displayi.hpp
    internal/fontcore.hpp
    internal/iconsei.hpp
    internal/managei.hpp
    internal/scrolisi.hpp
    listobvr.cpp
    listobvr.hpp
    manager.cpp
    manager.hpp
    mouse.cpp
    mouse.hpp
    painter.cpp
    painter.hpp
    progress.cpp
    progress.hpp
    restring.cpp
    restring.hpp
    root.cpp
    root.hpp
    scrolbar.cpp
    scrolbar.hpp
    scrolbut.cpp
    scrolbut.hpp
    scrolist.cpp
    scrolist.hpp
    sslistbx.cpp
    sslistbx.hpp
    sslistit.cpp
    sslistit.hpp
    RootSharedBitmaps.hpp
    RootSharedBitmaps.cpp)

add_library(gui ${GAME_COMPONENT_LIB_TYPE} ${GUI_SOURCES})
add_library(LibDev::gui ALIAS gui)

target_link_libraries(gui PUBLIC
    LibDev::device
    LibDev::render
)
