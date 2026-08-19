/*
 * A S T A R A L G . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    GraAStarAlg

    Implements the A* FindPath algorithm over an undirected graph, and a map
    from the graph vertex id to the GraAStarVertex class used to store vertex data
    for each vertex visited during the search.
*/

#ifndef _GRAPH_ASTARALG_HPP
#define _GRAPH_ASTARALG_HPP

#include "base/base.hpp"
#include "ctl/Vector.hpp"

// GRA_GRAPH IS_A GraGraph
// VERTEX_MAP IS_A FtlMap mapping GRA_GRAPH::VertexId to GraAStarVertex< GRA_GRAPH >
template <class GRA_GRAPH, class VERTEX_MAP> class GraAStarAlg
{
public:
    using Graph = GRA_GRAPH;
    using AStarVertices = VERTEX_MAP;
    using AStarVertex = typename AStarVertices::Value;
    using VertexId = typename Graph::VertexId;
    using ArcId = typename Graph::ArcId;
    using Weight = typename Graph::Weight;
    using VertexItem = typename Graph::VertexItem;
    using Vertices = ctl_vector<VertexId>;
    using Arcs = ctl_vector<ArcId>;

    // Algorithm state
    enum State
    {
        UNDEFINED,
        RUNNING,
        FINISHED
    };

    // ctor
    GraAStarAlg();

    // dtor.
    virtual ~GraAStarAlg();

    // Start/restart the algorithm finding a path from startVertex to endVertex in graph.
    void start(const Graph& graph, const VertexId& startVertex, const VertexId& endVertex);
    // PRE( graph.containsVertex( startVertex ) )
    // PRE( graph.containsVertex( endVertex ) )
    // PRE( startVertex != endVertex )

    // Advance the algorithm a little, returning its resulting state
    State update();
    // PRE( isDefined() )

    // The algorithm's current state
    State state() const;
    bool isFinished() const;
    bool isRunning() const;
    bool isDefined() const;

    // Derived class must supply the estimated cost of getting from from to to.
    // Ensure not an overestimate, or the algorithm may not find the best route.
    virtual Weight estimatedCost(const VertexItem& from, const VertexItem& to) const = 0;

    //  Derived class must specify a function which tells us whether a vertex
    //  is available for path finding.
    virtual bool vertexAvailable(const VertexItem& vertex) const = 0;

    Vertices output() const;
    // PRE( isFinished() )

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const GraAStarAlg<GRA_GRAPH, VERTEX_MAP>& t);

private:
    // Operations deliberately revoked
    GraAStarAlg(const GraAStarAlg&);
    GraAStarAlg& operator=(const GraAStarAlg&);
    bool operator==(const GraAStarAlg&);

    // Expands the vertex with id expandId. The current cost to this node is costToExpandVertex.
    // The cost is taken by value: expanding adds vertices to the vertex map, which can
    // reallocate its storage and invalidate a reference into the vertex we came from.
    void expand(const VertexId& expandId, Weight costToExpandVertex);

    // Adds id to the opne vertex list. Its current estimated total path cost is
    // estimatedTotalCost.
    void addOpenVertex(const VertexId& id, const Weight& estimatedTotalCost);

    // Update the journey costs for every vertex after id which has been visited.
    // The new better path found to id has cost costToVertex.
    void propogateBetterPath(const VertexId& id, const Weight& costToVertex);

    // Recursive method used to output vertices on best path in correct order
    void outputVertex(const VertexId& id, Vertices* pVertices) const;

    // Heap comparator: returns true when lhs has HIGHER estimated total cost
    // (i.e. lower priority), so the min-cost vertex ends up at the front.
    bool heapGreater(const VertexId& lhs, const VertexId& rhs) const;

    // Restore heap property after a vertex cost decrease
    void reheapAfterDecrease();

    // Data members
    const Graph* pGraph_; // The graph over which we will do the search
    AStarVertices* pAStarVertices_; // Vertex map for current search
    ctl_vector<VertexId> openVertices_; // Min-heap of open vertices by estimated total cost
    POST_DATA(Vertices closedVertices_;) // List of closed vertices
    VertexId startVertexId_; // Start vertex id
    VertexId endVertexId_; // Goal vertex id
    State state_; // Current state
    Weight bestPathCost_; // Cost of best path found to date to end vertex
    bool havePath_; // True when a path has been found to end vertex
};

// #ifdef _INSTANTIATE_TEMPLATE_CLASSES
#include "graph/AStarAlgorithm_templates.hpp"
// #endif

#endif

/* End ASTARALG.HPP *************************************************/
