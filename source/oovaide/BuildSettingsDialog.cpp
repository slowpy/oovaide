/*
 * BuildSettingsDialog.cpp
 *
 *  Created on: Sep 8, 2013
 *  \copyright 2013 DCBlaha.  Distributed under the GPL.
 */


#include "BuildSettingsDialog.h"
#include "Gui.h"
#include "Components.h"

static BuildSettingsDialog *gBuildDlg;


BuildSettingsDialog::BuildSettingsDialog(ProjectReader &project):
    mProject(project), mComponentFile(mProject)
    {
    gBuildDlg = this;
    mComponentTree.init(*Builder::getBuilder(), "ComponentListTreeview", "Component List");
    }


//////////////


BuildSettingsDialog::~BuildSettingsDialog()
    {}

void BuildSettingsDialog::enterScreen()
    {
    ScannedComponentInfo scannedCompInfo;
    OovStatus status = scannedCompInfo.readScannedInfo();
    if(status.ok())
        {
        GtkComboBoxText *typeBox = GTK_COMBO_BOX_TEXT(Builder::getBuilder()->getWidget(
                "ComponentTypeComboboxtext"));
        Gui::clear(typeBox);
        Gui::setSelected(GTK_COMBO_BOX(typeBox), -1);
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_Unknown));
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_StaticLib));
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_SharedLib));
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_Program));
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_JavaJarLib));
        Gui::appendText(typeBox, ComponentTypesFile::getLongComponentTypeName(
            CT_JavaJarProg));

        mComponentTree.clear();
        for(auto const &name : scannedCompInfo.getComponentNames())
            {
            if(mLastCompName.length() == 0)
                {
                mLastCompName = name;
                }
            mComponentTree.appendText(getParent(name),
                    ComponentTypesFile::getComponentChildName(name));
            }
        }
    if(status.needReport())
        {
        status.report(ET_Error, "Unable to read component settings file");
        }
    GuiTreePath path("0");
    gtk_tree_view_set_cursor(mComponentTree.getTreeView(), path.getPath(),
            nullptr, false);
    }

GuiTreeItem BuildSettingsDialog::getParent(std::string const &compName)
    {
    std::string parent = ComponentTypesFile::getComponentParentName(compName);
    return mComponentTree.getItem(parent, '/');
    }

void BuildSettingsDialog::switchComponent()
    {
    saveFromScreen(mLastCompName);
    mLastCompName = mComponentTree.getSelected('/');
    loadToScreen(mLastCompName);
    }

void BuildSettingsDialog::saveScreen()
    {
    saveFromScreen(mLastCompName);
    OovStatus status = mComponentFile.writeComponentTypes();
    if(status.needReport())
        {
        status.report(ET_Error, "Unable to save component type");
        }
    }

void BuildSettingsDialog::saveFromScreen(std::string const &compName)
    {
    GtkComboBoxText *typeBox = GTK_COMBO_BOX_TEXT(Builder::getBuilder()->getWidget(
            "ComponentTypeComboboxtext"));
    GtkTextView *argsView = GTK_TEXT_VIEW(Builder::getBuilder()->getWidget(
            "ComponentBuildArgsTextview"));

    char const * const boxValue = gtk_combo_box_text_get_active_text(typeBox);
    // If the cursor is changed during construction, boxValue will be NULL.
    if(compName.length())
        {
        if(boxValue)
            {
            mComponentFile.setComponentType(compName, boxValue);
            }
        std::string str = Gui::getText(argsView);
        CompoundValue buildArgs;
        buildArgs.parseString(str, '\n');
        std::string newBuildArgsStr = buildArgs.getAsString();
        mProject.setNameValue(ProjectReader::getCppArgsCompFilterName(compName),
            newBuildArgsStr);
        }
    }

void BuildSettingsDialog::loadToScreen(std::string const &compName)
    {
    if(compName.length())
        {
        GtkComboBoxText *typeBox = GTK_COMBO_BOX_TEXT(Builder::getBuilder()->getWidget(
                "ComponentTypeComboboxtext"));
        GtkTextView *argsView = GTK_TEXT_VIEW(Builder::getBuilder()->getWidget(
                "ComponentBuildArgsTextview"));

        int index = mComponentFile.getComponentType(compName);
        Gui::setSelected(GTK_COMBO_BOX(typeBox), index);

        CompoundValue cppArgs;
        cppArgs.parseString(mProject.getValue(
            ProjectReader::getCppArgsCompFilterName(compName)));
        std::string str = cppArgs.getAsString('\n');
        Gui::setText(argsView, str);
        }
    }

extern "C" G_MODULE_EXPORT void on_ComponentListTreeview_cursor_changed(GtkTreeView *view,
    gpointer *data)
    {
    gBuildDlg->switchComponent();
    }

extern "C" G_MODULE_EXPORT void on_BuildSettingsOkButton_clicked(
        GtkWidget *button, gpointer data)
    {
    gBuildDlg->saveScreen();
    gtk_widget_hide(Builder::getBuilder()->getWidget("BuildSettingsDialog"));
    }

extern "C" G_MODULE_EXPORT void on_BuildSettingsCancelButton_clicked(
        GtkWidget *button, gpointer data)
    {
    gtk_widget_hide(Builder::getBuilder()->getWidget("BuildSettingsDialog"));
    }

extern "C" G_MODULE_EXPORT void on_BuildSettingsMenuitem_activate()
    {
    gBuildDlg->enterScreen();

    Dialog dlg(GTK_DIALOG(Builder::getBuilder()->getWidget("BuildSettingsDialog")));
    dlg.run();
    }
