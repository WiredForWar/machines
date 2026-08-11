
set(GUI_SOURCES
    Clipboard.cpp
    Clipboard.hpp
    ResolvedUiString.cpp
    ResolvedUiString.hpp
    StringId.hpp
    Border.cpp
    Border.hpp
    Button.cpp
    Button.hpp
    Displayable.cpp
    Displayable.hpp
    EditBox.cpp
    EditBox.hpp
    Event.cpp
    Event.hpp
    Font.hpp
    gui.cpp
    gui.hpp
    Icon.cpp
    Icon.hpp
    IconSequence.cpp
    IconSequence.hpp
    Image.cpp
    Image.hpp
    Layout.cpp
    Layout.hpp
    internal/FontCore.hpp
    ListObserver.cpp
    ListObserver.hpp
    Manager.cpp
    Manager.hpp
    GuiPainter.cpp
    GuiPainter.hpp
    ProgressBar.cpp
    ProgressBar.hpp
    ResourceString.cpp
    ResourceString.hpp
    Root.cpp
    Root.hpp
    Screenshots.cpp
    Screenshots.hpp
    ScrollBar.cpp
    ScrollBar.hpp
    ScrollButton.cpp
    ScrollButton.hpp
    ScrollableList.cpp
    ScrollableList.hpp
    SingleSelectionListBox.cpp
    SingleSelectionListBox.hpp
    SingleSelectionListBoxItem.cpp
    SingleSelectionListBoxItem.hpp
    RootSharedBitmaps.hpp
    RootSharedBitmaps.cpp
)

add_library(gui ${GAME_COMPONENT_LIB_TYPE} ${GUI_SOURCES})
add_library(LibDev::gui ALIAS gui)

target_link_libraries(gui PUBLIC
    LibDev::device
    LibDev::render
)

target_link_libraries(gui PRIVATE
    LibDev::world4d
)
