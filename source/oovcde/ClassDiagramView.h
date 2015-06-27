/*
 * ClassDiagramView.h
 *
 *  Created on: Jun 17, 2015
 *  \copyright 2015 DCBlaha.  Distributed under the GPL.
 */

#ifndef CLASSDIAGRAMVIEW_H_
#define CLASSDIAGRAMVIEW_H_

#include "ClassDiagram.h"
#include "Builder.h"
#include "CairoDrawer.h"
#include "Gui.h"

class ClassDiagramListener
    {
    public:
        virtual ~ClassDiagramListener();
        virtual void gotoClass(OovStringRef const className) = 0;
    };

class ForegroundThreadBusyDialog:public OovTaskStatusListener
    {
    public:
        virtual OovTaskStatusListenerId startTask(OovStringRef const &text,
            size_t totalIterations) override
            {
            mDialog.startTask(text, totalIterations);
            return 0;
            }
        /// @return false to stop iteration.
        virtual bool updateProgressIteration(OovTaskStatusListenerId id, size_t i,
            OovStringRef const &text=nullptr) override
            { return mDialog.updateProgressIteration(i, text, true); }
        /// This is a signal that indicates the task completed.  To end the
        /// task, override updateProcessIteration.
        virtual void endTask(OovTaskStatusListenerId id) override
            { mDialog.endTask(); }

    private:
        TaskBusyDialog mDialog;
    };

class ClassDiagramView:public ClassGraphListener
    {
    public:
        ClassDiagramView():
            mNullDrawer(nullptr), mListener(nullptr)
            {}
        ~ClassDiagramView()
            {}
        void initialize(const ModelData &modelData,
                ClassDiagramListener &listener,
                OovTaskStatusListener &taskStatusListener);

        void clearGraphAndAddClass(OovStringRef const className,
                ClassGraph::eAddNodeTypes addType=ClassGraph::AN_All)
            {
            setCairoContext();
            mClassDiagram.clearGraphAndAddClass(className, addType);
            // It doesn't seem like this should be needed, but even with
            // updateDrawingAreaSize, this doesn't work. It messes up
            // drawing in other non-drawing area widgets, like the
            // class selection list.
            drawToDrawingArea();
            }
        void addClass(OovStringRef const className,
                ClassGraph::eAddNodeTypes addType=ClassGraph::AN_All,
                int depth=ClassDiagram::DEPTH_IMMEDIATE_RELATIONS)
            {
            setCairoContext();
            mClassDiagram.addClass(className, addType, depth);
            }
        void buttonPressEvent(const GdkEventButton *event);
        void buttonReleaseEvent(const GdkEventButton *event);
        void drawToDrawingArea();
        void drawSvgDiagram(FILE *fp);
        void saveDiagram(FILE *fp)
            { mClassDiagram.saveDiagram(fp); }
        void loadDiagram(FILE *fp)
            {
            setCairoContext();
            mClassDiagram.loadDiagram(fp);
            }
        bool isModified() const
            { return mClassDiagram.isModified(); }
        void requestRedraw()
            {
            gtk_widget_queue_draw(getDiagramWidget());
            }
        ClassDiagram &getDiagram()
            { return mClassDiagram; }
        // For extern functions
        ClassNode *getNode(int x, int y)
            { return mClassDiagram.getNode(x, y); }
        void updatePositions();
        void gotoClass(OovStringRef const className);
        void displayListContextMenu(guint button, guint32 acttime, gpointer data);

    private:
        ClassDiagram mClassDiagram;
        /// Used to calculate font sizes. Apparently the setContext call that
        /// calls gdk_cairo_create cannot be called from the background
        /// thread, but the other calls to get text extents can be made from
        /// the background thread.
        GtkCairoContext mCairoContext;
        NullDrawer mNullDrawer;
        ClassDiagramListener *mListener;
        void displayDrawContextMenu(guint button, guint32 acttime, gpointer data);
        void updateDrawingAreaSize();
        GtkWidget *getDiagramWidget()
            { return Builder::getBuilder()->getWidget("DiagramDrawingarea"); }
        void setCairoContext()
            {
            mCairoContext.setContext(getDiagramWidget());
            mNullDrawer.setGraphicsLib(mCairoContext.getCairo());
            mClassDiagram.setNullDrawer(mNullDrawer);
            }
        // WARNING - This is called from a background thread.
        virtual void doneRepositioning() override
            { requestRedraw(); }
        ForegroundThreadBusyDialog mForegroundBusyDialog;
    };


#endif /* CLASSDIAGRAMVIEW_H_ */