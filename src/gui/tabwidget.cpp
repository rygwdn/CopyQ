/*
    Copyright (c) 2013, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tabwidget.h"
#include "tabbar.h"
#include "tabtree.h"

#include <QAction>
#include <QPoint>
#include <QStackedWidget>

namespace {

void addTabAction(QWidget *widget, const QList<QKeySequence> &shortcuts,
                  const char *slot, bool widgetContext = true)
{
    QAction *act = new QAction(widget);
    act->setShortcutContext(widgetContext ? Qt::WidgetShortcut : Qt::WindowShortcut);
    act->setShortcuts(shortcuts);
    act->setPriority(QAction::HighPriority);
    widget->addAction(act);
    widget->connect( act, SIGNAL(triggered()), slot );
}

} // namespace

TabWidget::TabWidget(QWidget *parent)
    : QWidget(parent)
    , m_tabBar(NULL)
    , m_tabTree(NULL)
    , m_layout(NULL)
    , m_stackedWidget(NULL)
{
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(m_layout);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_stackedWidget = new QStackedWidget(this);
    m_layout->addWidget(m_stackedWidget);

    addTabAction(this, QList<QKeySequence>() << QKeySequence::NextChild,
                 SLOT(nextTab()), false);
    addTabAction(this, QList<QKeySequence>() << QKeySequence::PreviousChild,
                 SLOT(previousTab()), false);

    createTabBar();
}

QString TabWidget::getCurrentTabPath() const
{
    return isTreeModeEnabled() ? m_tabTree->getTabPath( m_tabTree->currentItem() ) : QString();
}

bool TabWidget::isTabGroup(const QString &tab) const
{
    return isTreeModeEnabled() && m_tabTree->isTabGroup( m_tabTree->findTreeItem(tab) );
}

bool TabWidget::isTreeModeEnabled() const
{
    return m_tabTree != NULL;
}

int TabWidget::currentIndex() const
{
    return m_stackedWidget->currentIndex();
}

QWidget *TabWidget::widget(int tabIndex)
{
    return m_stackedWidget->widget(tabIndex);
}

int TabWidget::count() const
{
    return m_stackedWidget->count();
}

QString TabWidget::tabText(int tabIndex) const
{
    return isTreeModeEnabled() ? m_tabTree->getTabPath( m_tabTree->findTreeItem(tabIndex) )
                               : m_tabBar->tabText(tabIndex);
}

void TabWidget::setTabText(int tabIndex, const QString &tabText)
{
    if ( isTreeModeEnabled() ) {
        m_tabTree->setTabText(tabIndex, tabText);
    } else {
        m_tabBar->setTabText(tabIndex, tabText);
    }
}

void TabWidget::insertTab(int tabIndex, QWidget *widget, const QString &tabText)
{
    bool firstTab = count() == 0;
    m_stackedWidget->insertWidget(tabIndex, widget);

    if ( isTreeModeEnabled() )
        m_tabTree->insertTab(tabText, tabIndex, firstTab);
    else
        m_tabBar->insertTab(tabIndex, tabText);

    if (firstTab)
        emit currentChanged(0, -1);
}

void TabWidget::removeTab(int tabIndex)
{
    if (tabIndex == currentIndex())
        setCurrentIndex(0);

    QWidget *w = m_stackedWidget->widget(tabIndex);
    m_stackedWidget->removeWidget(w);
    delete w;

    if ( isTreeModeEnabled() )
        m_tabTree->removeTab(tabIndex);
    else
        m_tabBar->removeTab(tabIndex);
}

void TabWidget::setTabPosition(QBoxLayout::Direction direction)
{
    m_layout->setDirection(direction);
}

void TabWidget::setCollapsedTabs(const QStringList &collapsedTabs)
{
    if ( isTreeModeEnabled() )
        m_tabTree->setCollapseTabs(collapsedTabs);
}

QStringList TabWidget::collapsedTabs() const
{
    return isTreeModeEnabled() ? m_tabTree->collapsedTabs() : QStringList();
}

QStringList TabWidget::tabs() const
{
    QStringList tabs;

    for( int i = 0; i < count(); ++i )
        tabs.append( tabText(i) );

    return tabs;
}

void TabWidget::moveTab(int from, int to)
{
    if ( isTreeModeEnabled() )
        m_tabTree->moveTab(from, to);
    else
        m_tabBar->moveTab(from, to);

    bool isCurrent = currentIndex() == from;

    m_stackedWidget->insertWidget(to, m_stackedWidget->widget(from));

    if (isCurrent)
        setCurrentIndex(to);
}

void TabWidget::setCurrentIndex(int tabIndex)
{
    QWidget *w = currentWidget();
    const int current = (isTreeModeEnabled() && w != NULL && w->isHidden()) ? -1 : currentIndex();

    if (tabIndex == current)
        return;

    if (tabIndex != -1) {
        m_stackedWidget->setCurrentIndex(tabIndex);

        w = currentWidget();
        if (w == NULL)
            return;

        w->show();
        if (isTreeModeEnabled() ? m_tabTree->hasFocus() : m_tabBar->hasFocus())
            w->setFocus();

        if ( isTreeModeEnabled() )
            m_tabTree->setCurrentTabIndex(tabIndex);
        else
            m_tabBar->setCurrentIndex(tabIndex);
    } else if (w != NULL) {
        if (w->hasFocus())
            isTreeModeEnabled() ? m_tabTree->setFocus() : m_tabBar->setFocus();
        w->hide();
    }

    emit currentChanged(tabIndex, current);
}

void TabWidget::nextTab()
{
    if ( isTreeModeEnabled() ) {
        m_tabTree->nextTreeItem();
    } else {
        const int tab = (currentIndex() + 1) % count();
        setCurrentIndex(tab);
    }
}

void TabWidget::previousTab()
{
    if ( isTreeModeEnabled() ) {
        m_tabTree->previousTreeItem();
    } else {
        const int size = count();
        const int tab = (size + currentIndex() - 1) % size;
        setCurrentIndex(tab);
    }
}

void TabWidget::setTabBarDisabled(bool disabled)
{
    if ( isTreeModeEnabled() )
        m_tabTree->setDisabled(disabled);
    else
        m_tabBar->setDisabled(disabled);
}

void TabWidget::setTabBarHidden(bool hidden)
{
    if ( isTreeModeEnabled() )
        m_tabTree->setHidden(hidden);
    else
        m_tabBar->setHidden(hidden);
}

void TabWidget::setTreeModeEnabled(bool enabled)
{
    if (isTreeModeEnabled() == enabled)
        return;

    const QStringList tabs = this->tabs();

    if (enabled) {
        delete m_tabBar;
        m_tabBar = NULL;

        createTabTree();
        for (int i = 0; i < tabs.size(); ++i)
            m_tabTree->insertTab(tabs[i], i, i == 0);
    } else {
        delete m_tabTree;
        m_tabTree = NULL;

        createTabBar();
        for (int i = 0; i < tabs.size(); ++i)
            m_tabBar->insertTab(i, tabs[i]);
    }
}

void TabWidget::onTreeItemSelected(bool isGroup)
{
    QWidget *w = currentWidget();
    if (w == NULL)
        return;

    if (isGroup) {
        w->hide();
    } else {
        w->show();
        w->setFocus();
    }
}

void TabWidget::onTabMoved(int from, int to)
{
    m_stackedWidget->insertWidget(to, m_stackedWidget->widget(from));
}

void TabWidget::createTabBar()
{
    m_tabBar = new TabBar(this);
    m_layout->addWidget(m_tabBar);

    m_tabBar->setExpanding(false);
    m_tabBar->setMovable(true);

    connect( m_tabBar, SIGNAL(tabMenuRequested(QPoint, int)),
             this, SIGNAL(tabMenuRequested(QPoint, int)) );
    connect( m_tabBar, SIGNAL(tabRenamed(QString,int)),
             this, SIGNAL(tabRenamed(QString,int)) );
    connect( m_tabBar, SIGNAL(tabCloseRequested(int)),
             this, SIGNAL(tabCloseRequested(int)) );
    connect( m_tabBar, SIGNAL(currentChanged(int)),
             this, SLOT(setCurrentIndex(int)) );
    connect( m_tabBar, SIGNAL(tabMoved(int, int)),
             this, SLOT(onTabMoved(int, int)) );
}

void TabWidget::createTabTree()
{
    m_tabTree = new TabTree(this);
    m_layout->addWidget(m_tabTree);

    m_tabTree->setObjectName("tab_tree");

    // More consistent behavior for selecting items using arrow keys.
    addTabAction(m_tabTree, QList<QKeySequence>() << Qt::Key_Down << Qt::Key_Right,
                 SLOT(nextTreeItem()));
    addTabAction(m_tabTree, QList<QKeySequence>() << Qt::Key_Up << Qt::Key_Left,
                 SLOT(previousTreeItem()));

    connect( m_tabTree, SIGNAL(tabMenuRequested(QPoint,QString)),
             this, SIGNAL(tabMenuRequested(QPoint,QString)) );
    connect( m_tabTree, SIGNAL(tabMoved(QString,QString,QString)),
             this, SIGNAL(tabMoved(QString,QString,QString)) );
    connect( m_tabTree, SIGNAL(currentTabChanged(int)),
             this, SLOT(setCurrentIndex(int)) );
}
