#include "toolBar.h"

#include <QCursor>
#include <QMenu>
#include <QMessageBox>

#include "subMenuFile.h"

ToolBar::ToolBar(QWidget* parent) : QToolBar(parent) {
    // Initialize toolbar and actions
    fileAction = addAction(tr("File"));
    editAction = addAction(tr("Edit"));
    viewAction = addAction(tr("View"));
    imageAction = addAction(tr("Image"));
    toolsAction = addAction(tr("Tools"));
    helpAction = addAction(tr("Help"));

    // Connect actions to their respective slots
    connect(fileAction, &QAction::triggered, this, &ToolBar::onFileActionTriggered);
    connect(editAction, &QAction::triggered, this, &ToolBar::onEditActionTriggered);
    connect(viewAction, &QAction::triggered, this, &ToolBar::onViewActionTriggered);
    connect(imageAction, &QAction::triggered, this, &ToolBar::onImageActionTriggered);
    connect(toolsAction, &QAction::triggered, this, &ToolBar::onToolsActionTriggered);
    connect(helpAction, &QAction::triggered, this, &ToolBar::onHelpActionTriggered);
}
ToolBar::~ToolBar() {}
// TODO: Move to Controller later
void ToolBar::onFileActionTriggered() {
    QMessageBox::information(this, tr("File Action"), tr("File action triggered."));
    SubMenuFile* subMenu = new SubMenuFile(this);
    subMenu->exec(QCursor::pos());
}

void ToolBar::onEditActionTriggered() {
    QMessageBox::information(this, tr("Edit Action"), tr("Edit action triggered."));
}

void ToolBar::onViewActionTriggered() {
    QMessageBox::information(this, tr("View Action"), tr("View action triggered."));
}

void ToolBar::onImageActionTriggered() {
    QMessageBox::information(this, tr("Image Action"), tr("Image action triggered."));
}

void ToolBar::onToolsActionTriggered() {
    QMessageBox::information(this, tr("Tools Action"), tr("Tools action triggered."));
}

void ToolBar::onHelpActionTriggered() {
    QMessageBox::information(this, tr("Help Action"), tr("Help action triggered."));
}