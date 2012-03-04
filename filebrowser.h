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

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QObject>
#include <QList>

#include "tree_view.h"

#include "onyx/ui/buttons.h"
#include "onyx/ui/status_bar.h"


class FileBrowser : public QDialog
{
    Q_OBJECT
public:
    explicit FileBrowser(QWidget *parent = 0);
    ~FileBrowser();

    QString showLoadFile(const QString &path = QString());

private slots:
    void onItemActivated(const QModelIndex &);
    void onStatusBarClicked(const int, const int);
    void onBreadCrumbActivated();

public slots:

private:
    void updateBreadCrumbs();
    void updateTreeView();
    void updateModel();

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    QStringList realToVirtualPath(const QString &path);

private:
    QString currentRealPath;
    QStringList currentPath;
    QVBoxLayout layout;
    QHBoxLayout breadCrumbsLayout;
    QStandardItemModel model;
    obx::ObxTreeView treeView;
    ui::StatusBar statusBar;
};

#endif // FILEBROWSER_H
