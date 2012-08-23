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
      statusBar(0, ui::PROGRESS | ui::MESSAGE |
                   ui::BATTERY | ui::SCREEN_REFRESH |
                   ui::CLOCK)
{
    setModal(true);
    treeView.setFocusPolicy(Qt::TabFocus);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Dark);

    breadCrumbsLayout.setAlignment(Qt::AlignLeft);

    layout.setSpacing(0);
    layout.setContentsMargins(0, 0, 0, 0);

    layout.addLayout(&breadCrumbsLayout);

    treeView.showHeader(false);
    layout.addWidget(&treeView, 1);

    layout.addWidget(&statusBar);

    connect(&treeView, SIGNAL(activated(const QModelIndex &)),
            SLOT(onItemActivated(const QModelIndex &)));
    connect(&treeView, SIGNAL(positionChanged(int, int)),
            &statusBar, SLOT(setProgress(int, int)));
    connect(&statusBar, SIGNAL(progressClicked(const int, const int)),
            SLOT(onStatusBarClicked(const int, const int)));
    treeView.setFocus();
}

FileBrowser::~FileBrowser()
{

}

QString FileBrowser::showLoadFile(const QString &path)
{
    QString absPath = path;
    if (!QDir(path).exists()) {
        /* perhaps it is a file inside the directory */
        if (QFile(path).exists()) {
            QDir dir(path);
            dir.cdUp();
            absPath = dir.absolutePath();
            /* TODO hilight that file */
        }
    }
    if (!QFile(absPath).exists()) {
        absPath = QString();
    }

    currentPath = realToVirtualPath(absPath);
    updateTreeView();

    showMaximized();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    if (exec() == QDialog::Accepted)
        return currentRealPath;
    else
        return QString();
}

void FileBrowser::updateBreadCrumbs()
{
    QLayoutItem *it;
    while ((it = breadCrumbsLayout.takeAt(0)) != 0) {
        if (it->widget()) {
            it->widget()->hide();
            it->widget()->deleteLater();
        }
        delete it;
    }

    QStringList path;

    ui::OnyxPushButton *button = new ui::OnyxPushButton(" ", 0);
    button->setData(path);
    connect(button, SIGNAL(released()), SLOT(onBreadCrumbActivated()));
    breadCrumbsLayout.addWidget(button);

    foreach (const QString &p, currentPath) {
        path.append(p);
        button = new ui::OnyxPushButton(p, 0);
        button->setData(path);
        connect(button, SIGNAL(released()), SLOT(onBreadCrumbActivated()));
        breadCrumbsLayout.addWidget(button);
    }

    breadCrumbsLayout.addStretch(1);

    button = new ui::OnyxPushButton(QIcon(":/images/close.png"), "", this);
    connect(button, SIGNAL(released()), SLOT(reject()));
    breadCrumbsLayout.addWidget(button);
}

void FileBrowser::updateTreeView()
{
    updateBreadCrumbs();
    treeView.clear();
    updateModel();
    treeView.setModel(&model);
    statusBar.setProgress(treeView.currentPage(), treeView.pages());
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void FileBrowser::updateModel()
{
    ModelBuilder builder(model);

    if (currentPath.isEmpty()) {
        /* TODO only if they exist */
        //builder.addItem(QIcon(":/images/sketch.png"), "Internal Scribbles", QDir::homePath() + QString("/notes"));
        builder.addItem(QIcon(), "Flash", LIBRARY_ROOT);
        builder.addItem(QIcon(), "SD Card", SDMMC_ROOT);
        builder.addItem(QIcon(), "Root", "/");
        return;
    }
    builder.addItem(QIcon(), "Up", "..");

    currentRealPath = currentPath.join("/");

    QDir dir(currentRealPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);

    QFileInfoList list = dir.entryInfoList();
    foreach (QFileInfo fileInfo, list) {
        builder.addItem(QIcon(), fileInfo.fileName(), fileInfo.fileName());
    }
}

QStringList FileBrowser::realToVirtualPath(const QString &rPath)
{
    if (rPath.isEmpty()) return QStringList();

    QString path = QDir(rPath).absolutePath();
    /* TODO define these paths only at one place */
    QStringList vRoots;
    vRoots.append(QDir::homePath() + QString("/notes"));
    vRoots.append(LIBRARY_ROOT);
    vRoots.append(SDMMC_ROOT);
    vRoots.append("/");
    foreach (const QString &vr, vRoots) {
        if (path.startsWith(vr)) {
            QStringList vPath;
            vPath.append(vr);
            int tailStart = vr.length() + (vr.endsWith("/") ? 0 : 1);
            vPath.append(path.mid(tailStart).split('/'));
            return vPath;
        }
    }
    return path.split('/');
}

void FileBrowser::keyPressEvent(QKeyEvent *ev)
{
    ev->accept();
}

void FileBrowser::keyReleaseEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        treeView.keyReleaseEvent(ev);
        break;
    case Qt::Key_Escape:
        reject();
        break;
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

    currentRealPath = currentPath.isEmpty() ? QString() : currentPath.join("/");

    if (currentRealPath.isEmpty() || QDir(currentRealPath).exists()) {
        updateTreeView();
    } else if (QFile(currentRealPath).exists()) {
        accept();
    } else {
        /* TODO error */
        currentRealPath = QString();
        reject();
    }
}

void FileBrowser::onStatusBarClicked(const int percentage, const int page)
{
    Q_UNUSED(percentage);
    treeView.jumpToPage(page);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void FileBrowser::onBreadCrumbActivated()
{
    /* TODO qobject_cast would be better but seems not to be usable */
    ui::OnyxPushButton *button = reinterpret_cast<ui::OnyxPushButton *>(sender());
    if (button == 0)
        return;
    currentPath = button->data().toStringList();
    qDebug() << currentPath;
    updateTreeView();
}
