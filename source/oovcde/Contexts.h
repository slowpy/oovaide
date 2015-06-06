/*
 * Contexts.h
 * Created on: May 14, 2015
 * \copyright 2015 DCBlaha.  Distributed under the GPL.
 */

#ifndef CONTEXTS_H_
#define CONTEXTS_H_

#include "OovProject.h"
#include "Journal.h"
#include "JournalList.h"
#include "ClassList.h"
#include "ComponentList.h"
#include "OperationList.h"


class IncludeList:public GuiList
    {
    public:
        void init(Builder &builder)
            {
            GuiList::init(builder, "IncludeTreeview", "Include List");
            }
    };

// A context is related to a task that the programmer is performing.
// There is usually a diagram associated with things that affect the diagram.
// It handles the routing of user functions (callbacks) depending on context.
//
// Examples:
//	View components
//		Display component diagram
//		Display component/module selection list
// 	View related classes:
//		Display class diagram
//		Display class selection list
//	View part of a class:
//		Display portion diagram
//		Display class selection list
class Contexts:public JournalListener
    {
    public:
	enum eContexts
	    {
	    C_BinaryComponent, C_Include, C_Zone, C_Class, C_Portion,
	    C_Operation, C_Journal
	    };

	Contexts(OovProject &proj);
	void init(OovTaskStatusListener &taskStatusListener);
	void stopAndWaitForBackgroundComplete()
	    { mJournal.stopAndWaitForBackgroundComplete(); }

	void clear();
	/// Oovcde performs the analysis, then calls this after analysis completes.
	/// Then this updates the component list, and starts loading the project files.
        void updateContextAfterAnalysisCompletes();
        /// Once the project files are loaded, then the lists that depend on the
        /// project can be updated.
	void updateContextAfterProjectLoaded();
	void saveFile(FILE *fp)
	    { mJournal.saveFile(fp); }
	void cppArgOptionsChangedUpdateDrawings()
	    { mJournal.cppArgOptionsChangedUpdateDrawings(); }
        const JournalRecord *getCurrentJournalRecord()
            { return mJournal.getCurrentRecord(); }

	// JournalListener callbacks
	virtual void displayClass(OovStringRef const className) override;
	virtual void displayOperation(OovStringRef const className,
		OovStringRef const operName, bool isConst) override;

	// Called from callbacks
	void displayClassDiagram();
	void displayOperationsDiagram();
	void displayJournal();
	void displayComponentDiagram();
        void displayIncludeDiagram();
	void displayZoneDiagram();
	void zoneTreeviewButtonRelease(const GdkEventButton *event);
	void setContext(eContexts context);

    private:
	OovProject &mProject;
	eContexts mCurrentContext;

	Journal mJournal;
        // This is the list of components (directories), not modules (source files).
        ComponentList mComponentList;
        IncludeList mIncludeList;
	ClassList mClassList;
	OperationList mOperationList;
	ZoneDiagramList mZoneList;
	JournalList mJournalList;

	void addClass(OovStringRef const className);
	std::string getSelectedClass() const
	    {
	    return mClassList.getSelected();
	    }
	void clearSelectedComponent()
	    {
	    mComponentList.clearSelection();
	    }
        /// Reads from the journal and updates the GUI journal list.
        void updateJournalList();
        void updateIncludeList();
        void updateClassList();
        void updateOperationList(const ModelData &modelData, OovStringRef const className);
    };

#endif /* CONTEXTS_H_ */