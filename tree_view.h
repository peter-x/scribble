/*  Copyright (C) 2011  OpenBOOX
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TREE_VIEW_H_
#define TREE_VIEW_H_

#include "onyx/base/base.h"
#include <QtGui/QtGui>

namespace obx
{

class ObxTreeHeaderBar : public QWidget
{
    Q_OBJECT
public:
    ObxTreeHeaderBar(QWidget *parent, QStandardItemModel * model);
    ~ObxTreeHeaderBar();

public:
    void setModel(QStandardItemModel * model);
    void setColumnWidth(const QVector<int> &percentages);
    const QVector<int> & columnWidth() { return percentages_; }
    QRect columnRect(int column);
    QStandardItemModel * model() { return model_; }

public Q_SLOTS:
    void clear();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QStandardItemModel * model_;
    QVector<int> percentages_;
};

class ObxTreeView;
class ObxTreeViewItem : public QWidget
{
    Q_OBJECT
public:
    ObxTreeViewItem(QWidget *parent, ObxTreeView & view);
    ~ObxTreeViewItem();

public:
    void setData(QStandardItem *item);
    QStandardItem * data() { return data_; }

    bool isSelected() { return selected_; }
    void select(bool select);

    bool needRepaint() const { return need_repaint_;}

Q_SIGNALS:
    void pressed(ObxTreeViewItem *item, const QPoint & press);
    void released(ObxTreeViewItem *item, const QPoint & release);
    void moved(ObxTreeViewItem *item);
    void clicked(ObxTreeViewItem *item);

private:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

private:
    QStandardItem * data_;
    bool selected_;
    bool need_repaint_;
    ObxTreeView & view_;
};

class ObxTreeView : public QWidget
{
    Q_OBJECT
public:
    ObxTreeView(QWidget *parent, QStandardItemModel * model);
    ~ObxTreeView();

public:
    int  pages();
    bool jumpToPage(int page);
    int  currentPage();
    int  itemsPerPage();

    bool pageUp();
    bool pageDown();

    void showHeader(bool show);
    ObxTreeHeaderBar & header() { return header_bar_; }

    void setModel(QStandardItemModel * model);
    QStandardItem * item(QStandardItem *item, int col);

    int  selected();
    void setHovering(bool);
    bool hovering();

public Q_SLOTS:
    void clear();
    bool select(const QModelIndex & index);
    bool select(const QString & title);
    bool setColumnWidth(const QVector<int> &percentages);
    void keyReleaseEvent(QKeyEvent * event);

protected:
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *);

Q_SIGNALS:
    void activated(const QModelIndex & index);
    void positionChanged(int current, int total);
    void exceed(bool begin);

private Q_SLOTS:
    void onItemClicked(ObxTreeViewItem *);
    void onItemPressed(ObxTreeViewItem *, const QPoint &);
    void onItemReleased(ObxTreeViewItem *, const QPoint &);
    void onItemMoved(ObxTreeViewItem *);

private:
    void createLayout();
    void setupInternalModel(QStandardItemModel * model,
                            QStandardItem *item,
                            int level);
    void arrangeItems(int, int, int);
    void updateLayout(const int rows);
    bool selectItem(ObxTreeViewItem *item);
    QStandardItem * item(int row, int col);
    bool navigate(int offset);

    QString level(int row, int col);

    void updateTreeWidget();
    int first_visible();
    void reportPosition();

    void activate(int select = -1);

private:
    QVBoxLayout layout_;
    QStandardItemModel *model_;
    QGridLayout items_layout_;

    ObxTreeHeaderBar header_bar_;

    int selected_;      ///< Absolute position.
    int first_visible_; ///< Absolute position.
    int items_per_page_;
    QVector<QStandardItem *> all_items_;

    typedef shared_ptr<ObxTreeViewItem> ViewItemPtr;
    typedef vector<ViewItemPtr> ViewItemPtrs;
    typedef vector<ViewItemPtr>::iterator ViewItemPtrIter;
    ViewItemPtrs views_;

    ObxTreeViewItem *pressed_item_;
    QPoint pressed_point_;

    bool hovering_;
};

}

#endif  // TREE_VIEW_H_
