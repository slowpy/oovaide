/*
 * OperationDrawer.h
 *
 *  Created on: Jul 24, 2013
 *  \copyright 2013 DCBlaha.  Distributed under the GPL.
 */

#ifndef OPERATIONDRAWER_H_
#define OPERATIONDRAWER_H_

#include "DiagramDrawer.h"
#include "OperationGraph.h"
#include <set>

struct DrawString
    {
    DrawString()
        {}
    DrawString(GraphPoint p, OovStringRef const s):
        pos(p), str(s)
        {}
    GraphPoint pos;
    OovString str;
    };

/// Used to draw an operation graph.
class OperationDrawer
    {
    public:
        OperationDrawer():
            mPad(0), mCharHeight(0)
            {}

        // Graph is not const because class positions get updated.
        GraphSize drawDiagram(DiagramDrawer &drawer, OperationGraph &graph,
                const OperationDrawOptions &options);
        GraphSize getDrawingSize(DiagramDrawer &drawer, OperationGraph &graph,
                const OperationDrawOptions &options);

    private:
        static const size_t NO_INDEX = static_cast<size_t>(-1);
        int mPad;
        int mCharHeight;
        GraphSize drawClass(DiagramDrawer &drawer, const OperationClass &node,
                const OperationDrawOptions &options);
        void drawLifeLines(DiagramDrawer &drawer,
                const std::vector<OperationClass> &classes,
                std::vector<int> const &classEndY, int endy);
        GraphSize drawOperation(DiagramDrawer &drawer, GraphPoint pos,
                OperationDefinition &operDef,
                const OperationGraph &graph, const OperationDrawOptions &options,
                std::set<const OperationDefinition*> &drawnOperations);
        GraphSize drawOperationNoText(DiagramDrawer &drawer, GraphPoint pos,
                OperationDefinition &operDef,
                const OperationGraph &graph, const OperationDrawOptions &options,
                std::set<const OperationDefinition*> &drawnOperations,
                std::vector<DrawString> &drawStrings);
        GraphSize drawOrSizeDiagram(DiagramDrawer &drawer, OperationGraph &graph,
                const OperationDrawOptions &options);
    };

#endif /* OPERATIONDRAWER_H_ */
