
set(MATHEX_SOURCES
    abox2d.cpp
    abox2d.hpp
    abox3d.cpp
    abox3d.hpp
    angle.cpp
    angle.hpp
    cabox2d.cpp
    cabox2d.hpp
    circle2d.cpp
    circle2d.hpp
    cline3d.cpp
    cline3d.hpp
    coordsys.cpp
    coordsys.hpp
    cpoint2d.cpp
    cpoint2d.hpp
    cpoint3d.cpp
    cpoint3d.hpp
    ctrans3d.cpp
    ctrans3d.hpp
    cvec3.cpp
    cvec3.hpp
    cvexpgon.cpp
    cvexpgon.hpp
    degrees.cpp
    degrees.hpp
    double.cpp
    double.hpp
    epsilon.cpp
    epsilon.hpp
    eulerang.cpp
    eulerang.hpp
    grid2d.cpp
    grid2d.hpp
    hlfpln2d.cpp
    hlfpln2d.hpp
    hshape3d.cpp
    hshape3d.hpp
    intersec.cpp
    intersec.hpp
    line2d.cpp
    line2d.hpp
    line3d.cpp
    line3d.hpp
    mathex.cpp
    mathex.hpp
    outcode.cpp
    outcode.hpp
    plane3d.cpp
    plane3d.hpp
    point2d.cpp
    point2d.hpp
    point3d.cpp
    point3d.hpp
    poly2d.cpp
    poly2d.hpp
    quad3d.cpp
    quad3d.hpp
    quadrati.cpp
    quadrati.hpp
    quatern.cpp
    quatern.hpp
    radians.cpp
    radians.hpp
    random.cpp
    random.hpp
    sausag2d.cpp
    sausag2d.hpp
    sausage3d.hpp
    size2d.cpp
    size2d.hpp
    sphere3d.cpp
    sphere3d.hpp
    t3dkey.cpp
    t3dkey.hpp
    transf3d.cpp
    transf3d.hpp
    tri2d.cpp
    tri2d.hpp
    utility.cpp
    utility.hpp
    vec2.cpp
    vec2.hpp
    vec3.cpp
    vec3.hpp
)

add_library(mathex ${GAME_COMPONENT_LIB_TYPE} ${MATHEX_SOURCES})
add_library(LibDev::mathex ALIAS mathex)

target_link_libraries(mathex PUBLIC
    LibDev::base
    LibDev::recorder
)
