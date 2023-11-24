/*
 * G R A P H . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GRA_GRAPH_HPP
#define _GRA_GRAPH_HPP

#include "graph/vertex.hpp"
#include "graph/arc.hpp"
#include "graph/base.hpp"

/* //////////////////////////////////////////////////////////////// */

// GraGraph is a weighted, undirected graph, parameterised
// by it's vertex and arc collections.

template <class MAP_VERTEX_ID_TO_GRA_VERTEX, class MAP_ARC_ID_TO_GRA_ARC>
class GraGraph
    : public GraGraphBase<
          typename MAP_VERTEX_ID_TO_GRA_VERTEX::Key,
          typename MAP_VERTEX_ID_TO_GRA_VERTEX::Value::VertexItem,
          typename MAP_ARC_ID_TO_GRA_ARC::Key,
          typename MAP_ARC_ID_TO_GRA_ARC::Value::ArcItem,
          typename MAP_VERTEX_ID_TO_GRA_VERTEX::Value::ArcIds,
          typename MAP_ARC_ID_TO_GRA_ARC::Value::Weight>
{
public:
    void constraints() {
        // MAP_VERTEX_ID_TO_GRA_VERTEX IS-A FtlMap
        // MAP_ARC_ID_TO_GRA_ARC IS-A FtlMap
    };

    using GraphBase = GraGraphBase<
        typename MAP_VERTEX_ID_TO_GRA_VERTEX::Key,
        typename MAP_VERTEX_ID_TO_GRA_VERTEX::Value::VertexItem,
        typename MAP_ARC_ID_TO_GRA_ARC::Key,
        typename MAP_ARC_ID_TO_GRA_ARC::Value::ArcItem,
        typename MAP_VERTEX_ID_TO_GRA_VERTEX::Value::ArcIds,
        typename MAP_ARC_ID_TO_GRA_ARC::Value::Weight>;

    using VertexItem = typename MAP_VERTEX_ID_TO_GRA_VERTEX::Value::VertexItem;
    using ArcItem = typename MAP_ARC_ID_TO_GRA_ARC::Value::ArcItem;
    using Weight = typename MAP_ARC_ID_TO_GRA_ARC::Value::Weight;
    using VertexId = typename GraphBase::VertexId;
    using ArcId = typename GraphBase::ArcId;

    using Vertices = MAP_VERTEX_ID_TO_GRA_VERTEX;
    using Arcs = MAP_ARC_ID_TO_GRA_ARC;

    using Vertex = GraVertex<VertexItem, ArcId>;
    using Arc = GraArc<ArcItem, VertexId, Weight>;

    ///////////////////////////////

    // inherited from GraGraphBase...
    bool containsVertex(const VertexId& a) const override;
    bool isConnected(const VertexId& from, const VertexId& to) const override;
    // PRE( containsVertex( from ) );
    // PRE( containsVertex( to ) );

    VertexItem& vertex(const VertexId& id) override;
    // PRE( containsVertex( id ) );

    const VertexItem& vertex(const VertexId& id) const override;
    // PRE( containsVertex( id ) );

    ///////////////////////////////

    virtual const Arcs& arcs() const;

    bool containsArc(const ArcId& a) const override;

    VertexId fromVertex(const ArcId& id) const override;
    // PRE( containsArc( id ) );

    VertexId toVertex(const ArcId& id) const override;
    // PRE( containsArc( id ) );

    ///////////////////////////////

    ArcItem& arc(const ArcId& id) override;
    // PRE( containsArc( id ) );

    const ArcItem& arc(const ArcId& id) const override;
    // PRE( containsArc( id ) );

    ///////////////////////////////

    Weight weight(const ArcId& id) const override;
    // PRE( containsArc( id ) );

    void weight(const ArcId& id, Weight w) override;
    // PRE( containsArc( id ) );
    // POST( weight( id ) == w );

    ///////////////////////////////

    typename GraphBase::ArcCount arity(const VertexId& id) const override;

    typename GraphBase::ArcIds& arcs(const VertexId& id) override;
    // PRE( containsVertex( id ) );

    const typename GraphBase::ArcIds& arcs(const VertexId& id) const override;
    // PRE( containsVertex( id ) );

    typename GraphBase::ArcIds arcs(const VertexId& from, const VertexId& to) const override;
    // PRE( isConnected( from, to ) );

protected:
    // PRE( not containsVertex( id ) );
    void doAddVertex(const VertexId& id, const VertexItem& item) override;
    // POST( vertex( id ) == item );

    // PRE( containsVertex( id ) );
    void doRemoveVertex(const VertexId& id) override;
    // POST( not containsVertex( id ) );

    ///////////////////////////////

    // PRE( containsVertex( from ) );
    // PRE( containsVertex( to ) );
    // PRE( not containsArc( arcId ) );
    // POST( arcs( from ).contains( arcId ) );
    // POST( arcs( to ).contains( arcId ) );
    void doAddArc(const VertexId& from, const VertexId& to, const ArcId& arcId, const ArcItem& arcItem) override;
    // POST( arc( arcId ) == arcItem );

    // PRE( containsArc( id ) );
    // POST( not arcs( from ).contains( arcId ) );
    // POST( not arcs( to ).contains( arcId ) );
    void doRemoveArc(const ArcId& id) override;
    // POST( not containsArc( id ) );

private:
    Vertices vertices_;
    Arcs arcs_;
};

/* //////////////////////////////////////////////////////////////// */

// #ifdef _INSTANTIATE_TEMPLATE_CLASSES
#include "graph/graph_templates.hpp"
// #endif

/* //////////////////////////////////////////////////////////////// */

#endif /* #ifndef _GRA_GRAPH_HPP   */

/* End GRAPH.HPP ****************************************************/
