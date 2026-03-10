
file(GLOB GRAPH_SOURCES
    Arc.hpp
    Arc_templates.hpp
    AStarAlgorithm.hpp
    AStarAlgorithm_templates.hpp
    AStarVertex.hpp
    AStarVertex_inline_templates.hpp
    AStarVertex_templates.hpp
    Base.hpp
    Base_templates.hpp
    Graph.hpp
    Graph_templates.hpp
    Vertex.hpp
    Vertex_templates.hpp
)

add_library(graph INTERFACE ${GRAPH_SOURCES})
add_library(LibDev::graph ALIAS graph)
