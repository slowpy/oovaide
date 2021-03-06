/*
 * PortionDiagramView.cpp
 *
 *  Created on: Jun 19, 2015
 *  \copyright 2015 DCBlaha.  Distributed under the GPL.
 */

#include "PortionDiagramView.h"
#include "Svg.h"
#include "Options.h"


void PortionDiagramView::drawToDrawingArea()
    {
    setCairoContext();
    CairoDrawer cairoDrawer(mPortionDiagram, mCairoContext.getCairo());
    cairoDrawer.clearAndSetDefaults();

    mPortionDiagram.drawDiagram(cairoDrawer);
    updateDrawingAreaSize();
    }

OovStatusReturn PortionDiagramView::drawSvgDiagram(File &file)
    {
    SvgDrawer svgDrawer(mPortionDiagram, file, mCairoContext.getCairo());
    mPortionDiagram.drawDiagram(svgDrawer);
    return svgDrawer.writeFile();
    }

void PortionDiagramView::updateDrawingAreaSize()
    {
    GraphSize size = mPortionDiagram.getDrawingSize(mNullDrawer);
    gtk_widget_set_size_request(getDiagramWidget(), size.x, size.y);
    }

static PortionDiagramView *sPortionDiagramView;

static void portionGraphDisplayContextMenu(guint button, guint32 acttime, gpointer data)
    {
    GtkMenu *menu = Builder::getBuilder()->getMenu("DrawPortionPopupMenu");
    gtk_menu_popup(menu, nullptr, nullptr, nullptr, nullptr, button, acttime);
    }

static GraphPoint sStartPosInfo;

void PortionDiagramView::graphButtonPressEvent(const GdkEventButton *event)
    {
    sStartPosInfo.set(event->x, event->y);
    sPortionDiagramView = this;
    }

void PortionDiagramView::graphButtonReleaseEvent(const GdkEventButton *event)
    {
    if(event->button == 1)
        {
        size_t nodeIndex = mPortionDiagram.getNodeIndex(mNullDrawer, sStartPosInfo);
        if(nodeIndex != PortionDrawer::NO_INDEX)
            {
            if(!mPortionDiagram.isSelected(nodeIndex))
                {
                mPortionDiagram.setSingleSelection(nodeIndex);
                }
//            mPortionDiagram.setPosition(nodeIndex, sStartPosInfo,
//                    GraphPoint(event->x, event->y));
            mPortionDiagram.moveSelection(GraphPoint(event->x, event->y) -
                sStartPosInfo);
            requestRedraw();
            }
        else
            {
            // If no nodes are contained in the rectangle, this clears the selection.
            mPortionDiagram.setRectSelection(sStartPosInfo, GraphPoint(event->x, event->y));
            requestRedraw();
            }
        }
    else
        {
        portionGraphDisplayContextMenu(event->button, event->time, (gpointer)event);
        }
    }

void PortionDiagramView::viewClassSource()
    {
    ModelClassifier const *classifier = mPortionDiagram.getCurrentClass();
    if(classifier && classifier->getModule())
        {
        viewSource(mGuiOptions, classifier->getModule()->getModulePath(),
        classifier->getLineNum());
        }
    }


extern "C" G_MODULE_EXPORT void on_PortionAlignTopmenuitem_activate(
        GtkWidget *widget, gpointer data)
        {
        sPortionDiagramView->alignTopSelection();
        }

extern "C" G_MODULE_EXPORT void on_PortionAlignLeftMenuitem_activate(
    GtkWidget *widget, gpointer data)
    {
    sPortionDiagramView->alignLeftSelection();
    }

extern "C" G_MODULE_EXPORT void on_PortionSpaceEvenlyDownMenuitem_activate(
    GtkWidget *widget, gpointer data)
    {
    sPortionDiagramView->spaceEvenlyDownSelection();
    }


extern "C" G_MODULE_EXPORT void on_PortionViewSourceMenuitem_activate(
    GtkWidget *widget, gpointer data)
    {
    sPortionDiagramView->viewClassSource();
    }

extern "C" G_MODULE_EXPORT void on_PortionFontMenuitem_activate(
        GtkWidget *widget, gpointer data)
    {
    int fontSize = sPortionDiagramView->getFontSize();
    if(setFontDialog(fontSize))
        {
        sPortionDiagramView->setFontSize(fontSize);
        }
    }

extern "C" G_MODULE_EXPORT void on_PortionRelayoutMenuitem_activate(
    GtkWidget *widget, gpointer data)
    {
    sPortionDiagramView->relayout();
    }

