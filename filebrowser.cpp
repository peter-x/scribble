/*
 * scribble: Scribbling Application for Onyx Boox M92
 *
 * Copyright (C) 2012 peter-x
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "filebrowser.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/buttons.h"

class ModelBuilder {
public:
    ModelBuilder(QStandardItemModel &model, int numCols = 1 /* TODO bug for greater than one */)
            : col(0), row(0), numCols(numCols), model(model) {
        model.clear();
        model.setColumnCount(numCols);
    }

    void addItem(const QString &text) {
        addItem(QIcon(), text);
    }

    void addItem(const QIcon &icon, const QString &text, const QString &data = QString()) {
        QFont itemFont;
        itemFont.setPointSize(20);
        QStandardItem *item = new QStandardItem(icon, text);
        item->setData(data);
        item->setFont(itemFont);
        model.setItem(row, col, item);
        col += 1;
        if (col >= numCols) {
            col = 0;
            row ++;
        }
    }

private:
    int col;
    int row;
    int numCols;
    QStandardItemModel &model;
};

FileBrowser::FileBrowser(QWidget *parent)
    : QDialog(parent), layout(this),
      treeView(0, &model),
      statusBar(0, ui::PROGRESS | ui::MESSAGE)
{
    setModal(true);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Dark);

    layout.setSpacing(0);
    layout.setContentsMargins(0, 0, 0, 0);

    treeView.showHeader(false);
    layout.addWidget(&treeView, 1);

    layout.addWidget(&statusBar);

    connect(&treeView, SIGNAL(activated(const QModelIndex &)),
            SLOT(onItemActivated(const QModelIndex &)));
    connect(&treeView, SIGNAL(positionChanged(int, int)),
            &statusBar, SLOT(setProgress(int, int)));
    connect(&statusBar, SIGNAL(progressClicked(const int, const int)),
            SLOT(onStatusBarClicked(const int, const int)));
}

FileBrowser::~FileBrowser()
{

}

QString FileBrowser::showLoadFile()
{
    updateTreeView();

    showMaximized();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    exec();

    return currentRealPath;
}

void FileBrowser::updateRealPath()
{
    if (currentPath.isEmpty()) {
        currentRealPath = QString();
    } else {
        currentRealPath = currentPath.join("/");
    }
}

void FileBrowser::updateTreeView()
{
    treeView.clear();
    updateModel();
    treeView.setModel(&model);
    treeView.update();
    statusBar.setProgress(treeView.currentPage(), treeView.pages());
}

void FileBrowser::updateModel()
{
    ModelBuilder builder(model);

    if (currentPath.isEmpty()) {
        /* TODO only if they exist */
        builder.addItem(QIcon(":/images/sketch.png"), "Internal Scribbles", QDir::homePath() + QString("/notes"));
        builder.addItem(QIcon(), "Flash", LIBRARY_ROOT);
        builder.addItem(QIcon(), "SD Card", SDMMC_ROOT);
        builder.addItem(QIcon(), "Root", "/");
        return;
    }
    builder.addItem(QIcon(), "Up", "..");

    QDir dir(currentRealPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);

    QFileInfoList list = dir.entryInfoList();
    foreach (QFileInfo fileInfo, list) {
        builder.addItem(QIcon(), fileInfo.fileName(), fileInfo.fileName());
    }
}

void FileBrowser::onItemActivated(const QModelIndex &idx)
{
    QStandardItem *item = model.itemFromIndex(idx);
    QString data = item->data().toString();
    if (data == "..") {
        currentPath = currentPath.mid(0, currentPath.length() - 1);
    } else {
        currentPath += data;
    }
    updateRealPath();
    if (currentRealPath.isEmpty() || QDir(currentRealPath).exists()) {
        updateTreeView();
    } else if (QFile(currentRealPath).exists()) {
        accept();
    } else {
        /* TODO error */
        reject();
    }
}

void FileBrowser::onStatusBarClicked(const int percentage, const int page)
{
    treeView.jumpToPage(page);
}
