
file(GLOB GRAPH_SOURCES
    arc.hpp
    arc_templates.hpp
    astaralg.hpp
    astaralg_templates.hpp
    astarvtx.hpp
    astarvtx_inline_templates.hpp
    astarvtx_templates.hpp
    base.hpp
    base_templates.hpp
    graph.hpp
    graph_templates.hpp
    path.hpp
    path_templates.hpp
    vertex.hpp
    vertex_templates.hpp
)

add_library(graph INTERFACE ${GRAPH_SOURCES})
add_library(LibDev::graph ALIAS graph)
