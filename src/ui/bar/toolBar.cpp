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

    // Connect actions to their respective slots
    connect(fileAction, &QAction::triggered, this, &ToolBar::onFileActionTriggered);
    connect(editAction, &QAction::triggered, this, &ToolBar::onEditActionTriggered);
    connect(viewAction, &QAction::triggered, this, &ToolBar::onViewActionTriggered);
    connect(imageAction, &QAction::triggered, this, &ToolBar::onImageActionTriggered);
}
ToolBar::~ToolBar() {}
// TODO: Move to Controller later
void ToolBar::onFileActionTriggered() {
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
