/*
 * PackagesDialogs.cpp
 *
 *  Created on: Jan 20, 2014
 *  \copyright 2014 DCBlaha.  Distributed under the GPL.
 */

#include "PackagesDialogs.h"
#include "OovString.h"
#include "ctype.h"
#include <algorithm>

static ProjectPackagesDialog *sProjectPackagesDialog;
static AddPackageDialog *sAddPackageDialog;


static void setEntry(OovStringRef const widgetName, OovStringRef const str)
    {
    GtkEntry *entry = GTK_ENTRY(Builder::getBuilder()->getWidget(widgetName));
    Gui::setText(entry, str);
    }

static OovString getEntry(OovStringRef const widgetName)
    {
    GtkEntry *entry = GTK_ENTRY(Builder::getBuilder()->getWidget(widgetName));
    return Gui::getText(entry);
    }


AddPackageDialog::AddPackageDialog():
    Dialog(GTK_DIALOG(Builder::getBuilder()->getWidget("AddPackageDialog")),
            GTK_WINDOW(Builder::getBuilder()->getWidget("ProjectPackagesDialog")))
    {
    sAddPackageDialog = this;
    mAllPackagesList.init(*Builder::getBuilder(), "AllPackagesTreeview", "Available Packages");
    mAllPackagesList.clear();
    for(auto const &pkgName : mAvailPackages.getAvailablePackages())
        mAllPackagesList.appendText(pkgName);
    }

AddPackageDialog::~AddPackageDialog()
    {
    sAddPackageDialog = nullptr;
    }

Package AddPackageDialog::getPackage() const
    {
    return mAvailPackages.getPackage(getEntry("AddPackageEntry"));
    }

void AddPackageDialog::selectPackage()
    {
    setEntry("AddPackageEntry", mAllPackagesList.getSelected());
    }

/////////////////

ProjectPackagesDialog::ProjectPackagesDialog(std::string &baseBuildArgs):
    Dialog(GTK_DIALOG(Builder::getBuilder()->getWidget("ProjectPackagesDialog")),
            GTK_WINDOW(Builder::getBuilder()->getWidget("OptionsDialog"))),
    mProjectPackages(true), mBaseBuildArgs(baseBuildArgs), mAllowSelection(true)
    {
    sProjectPackagesDialog = this;
    mProjectPackagesList.init(*Builder::getBuilder(), "ProjectPackagesTreeview", "Packages");
#ifdef __linux__
    Gui::setVisible(GTK_LABEL(Builder::getBuilder()->getWidget(
            "MissingDirectoryLabel")), false);
    Gui::setVisible(GTK_BUTTON(Builder::getBuilder()->getWidget(
            "ScanDirectoriesButton")), false);
#endif
    }

ProjectPackagesDialog::~ProjectPackagesDialog()
    {
    sProjectPackagesDialog = nullptr;
    }


void ProjectPackagesDialog::beforeRun()
    {
    // Don't read packages from args. They must be in project package file.
    size_t pos=0;

    OovString args(mBaseBuildArgs);
    while(pos != std::string::npos)
        {
        pos = args.find("-EP", pos);
        if(pos != std::string::npos)
            {
            size_t endPos = args.findSpace(pos+3);
//          std::string pkg;
            if(endPos != std::string::npos)
                {
//              pkg = args.substr(pos+3, endPos-(pos+3));
                if(args[endPos] == '\n')
                    endPos++;
                args.erase(pos, endPos-pos);
                }
            else
                {
//              pkg = args.substr(pos+3);
                args.erase(pos);
                }
            // Could use the package names for something.
//          pkgNames.push_back(pkg);
            }
        }
    updatePackageList();
    mBaseBuildArgs = args;
    }

static bool checkPackageDirs(Package const &pkg)
    {
    OovString badPath;
    bool ok = pkg.checkDirectories(badPath);
    if(!ok)
        {
        OovString str = "Unable to save package ";
        str += pkg.getPkgName();
        str += " because of incorrect directory ";
        str += badPath;
        Gui::messageBox(str.getStr());
        }
    return ok;
    }

bool ProjectPackagesDialog::afterRun(bool ok)
    {
    bool saved = false;
    if(ok)
        {
        saved = savePackage(mProjectPackagesList.getSelected());
        if(saved)
            {
            for(auto const &pkg : mProjectPackages.getPackages())
                {
                saved = checkPackageDirs(pkg);
                if(!saved)
                    {
                    break;
                    }
                }
            }
        if(saved)
            {
            OovStatus status = mProjectPackages.savePackages();
            if(status.needReport())
                {
                status.report(ET_Error, "Unable to save packages");
                }
            int len = mBaseBuildArgs.length();
            if(len > 0)
                {
                if(mBaseBuildArgs[len-1] != '\n')
                    mBaseBuildArgs.append("\n");
                }
            for(auto const &str : mProjectPackagesList.getText())
                {
                mBaseBuildArgs += "-EP";
                mBaseBuildArgs += str;
                mBaseBuildArgs += "\n";
                }
            }
        }
    return saved;
    }

bool ProjectPackagesDialog::savePackage(const std::string &pkgName)
    {
    bool saved = false;
    if(pkgName.length() > 0)
        {
        bool addOk = true;
        if(addOk)
            {
            Package pkg(pkgName, getEntry("PackageRootDirEntry"));
            pkg.setCompileInfo(getEntry("PackageIncDirEntry"),
                    getEntry("PackageCompileArgsEntry"));
            pkg.setLinkInfo(getEntry("PackageLibDirEntry"), getEntry("PackageLibNamesEntry"),
                    getEntry("PackageLinkArgsEntry"));
            if(checkPackageDirs(pkg))
                {
                saved = true;
                mProjectPackages.insertPackage(pkg);
                }
            }
        }
    return saved;
    }

void ProjectPackagesDialog::selectPackage()
    {
    if(mAllowSelection)
        {
        std::string pkgName = mProjectPackagesList.getSelected();
        savePackage(mLastSelectedPackage);
        Package pkg = mProjectPackages.getPackage(pkgName);

        setEntry("PackageRootDirEntry", pkg.getRootDir());
        setEntry("PackageIncDirEntry", pkg.getIncludeDirsAsString());
        setEntry("PackageCompileArgsEntry", pkg.getCompileArgsAsStr());
        setEntry("PackageLibNamesEntry", pkg.getLibraryNamesAsString());
        setEntry("PackageLibDirEntry", pkg.getLibraryDirsAsString());
        setEntry("PackageLinkArgsEntry", pkg.getLinkArgsAsStr());
        mLastSelectedPackage = pkgName;
        }
    }

void ProjectPackagesDialog::clearPackageDisplay()
    {
    setEntry("PackageRootDirEntry", "");
    setEntry("PackageIncDirEntry", "");
    setEntry("PackageCompileArgsEntry", "");
    setEntry("PackageLibNamesEntry", "");
    setEntry("PackageLibDirEntry", "");
    setEntry("PackageLinkArgsEntry", "");
    mLastSelectedPackage.clear();
    }

void ProjectPackagesDialog::removePackage()
    {
    std::string pkgName = mProjectPackagesList.getSelected();
    mProjectPackages.removePackage(pkgName);
    updatePackageList();
    }

void ProjectPackagesDialog::updatePackageList()
    {
    mAllowSelection = false;
    clearPackageDisplay();
    mProjectPackagesList.clear();
    for(auto const &pkg : mProjectPackages.getPackages())
        mProjectPackagesList.appendText(pkg.getPkgName());
    mAllowSelection = true;
    }

void ProjectPackagesDialog::displayAddPackageDialog()
    {
    AddPackageDialog dlg;
    if(dlg.run(true))
        {
        OovStringVec packages = mProjectPackagesList.getText();

        Package pkg = dlg.getPackage();
        if(std::find(packages.begin(), packages.end(), pkg.getPkgName()) ==
                packages.end())
            {
            mProjectPackages.insertPackage(pkg);
            }
        else
            {
            Gui::messageBox("Package already exists");
            }
        updatePackageList();
        mProjectPackagesList.setSelected(pkg.getPkgName());
#ifndef __linux__
        winScanDirectories();
#endif
        }
    }


#ifndef __linux__

void ProjectPackagesDialog::winSetEnableScanning()
    {
    OovStatus status(true, SC_File);
    bool missing = !FileIsDirOnDisk(getEntry("PackageRootDirEntry"), status);
    Gui::setEnabled(GTK_LABEL(Builder::getBuilder()->getWidget(
            "MissingDirectoryLabel")), missing);
    Gui::setEnabled(GTK_BUTTON(Builder::getBuilder()->getWidget(
            "ScanDirectoriesButton")), missing);
    }

/*
static int winMatchPackage(OovStringRef const pkgName, OovStringRef const dirName)
    {
    int matchQuality = 0;
    std::vector<std::string> pkgParts;
    char const * p = pkgName;
    enum CharTypes { CT_Start, CT_Other, CT_Num, CT_Alpha };
    CharTypes lastCt = CT_Start;
    std::string part;
    while(*p)
        {
        CharTypes ct;
        if(isalpha(*p))
            ct = CT_Alpha;
        else if(isdigit(*p))
            ct = CT_Num;
        else
            ct = CT_Other;
        if(lastCt == CT_Start)
            lastCt = ct;
        if(ct == lastCt)
            {
            if(ct != CT_Other)
                part += *p;
            }
        else
            {
            pkgParts.push_back(part);
            lastCt = ct;
            part += *p;
            }
        p++;
        }
    pkgParts.push_back(part);
    // Rate alpha more than numeric, and longer strings more than many shorter strings
    OovString dir = dirName;
    std::transform(dir.begin(), dir.end(), dir.begin(), ::tolower);
    for(auto &part : pkgParts)
        {
        std::transform(part.begin(), part.end(), part.begin(), ::tolower);
        if(std::string(dir).find(part) != std::string::npos)
            {
            int len = part.length();
            int mult = 1;
            if(isalpha(part[0]))
                mult = 2;
            matchQuality += len * len * mult;
            }
        }
    return matchQuality;
    }
*/


void ProjectPackagesDialog::winScanDirectories()
    {
    Package pkg = mProjectPackages.getPackage(
            mProjectPackagesList.getSelected());
    if(pkg.getPkgName().length() > 0)
        {
        OovString rootDir = getEntry("PackageRootDirEntry");
        if(pkg.winScanAndSetRootDir(rootDir))
            {
            // move to project packages.
            mProjectPackages.insertPackage(pkg);
            }
        setEntry("PackageRootDirEntry", pkg.getRootDir());
        }
    else
        Gui::messageBox("Select a package to scan", GTK_MESSAGE_INFO);
    }

#endif


///////////////////

extern "C" G_MODULE_EXPORT void on_PackageAddButton_clicked(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
    if(sProjectPackagesDialog)
        sProjectPackagesDialog->displayAddPackageDialog();
    }

extern "C" G_MODULE_EXPORT void on_PackageRemoveButton_clicked(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
    if(sProjectPackagesDialog)
        sProjectPackagesDialog->removePackage();
    }

extern "C" G_MODULE_EXPORT void on_ProjectPackagesTreeview_cursor_changed(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
    if(sProjectPackagesDialog)
        sProjectPackagesDialog->selectPackage();
    }

//////////////

extern "C" G_MODULE_EXPORT void on_AllPackagesTreeview_cursor_changed(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
    if(sAddPackageDialog)
        sAddPackageDialog->selectPackage();
    }

extern "C" G_MODULE_EXPORT void on_ScanDirectoriesButton_clicked(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
#ifndef __linux__
    if(sProjectPackagesDialog)
        sProjectPackagesDialog->winScanDirectories();
#endif
    }

extern "C" G_MODULE_EXPORT void on_PackageRootDirEntry_changed(
    GtkWidget * /*button*/, gpointer /*data*/)
    {
#ifndef __linux__
    if(sProjectPackagesDialog)
        sProjectPackagesDialog->winSetEnableScanning();
#endif
    }
