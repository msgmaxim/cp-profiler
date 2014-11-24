#include "qtgist.hh"

#include "nodevisitor.hh"
#include "nodecursor.hh"

void
Gist::createNewCanvas(void) {
    QGridLayout* layout = new QGridLayout(this);

    canvasDialog = new QDialog(this);

    canvasDialog->setLayout(layout);

    QVBoxLayout* nc_layout = new QVBoxLayout();

    QAbstractScrollArea* scrollArea = new QAbstractScrollArea(canvasDialog);
    canvasTwo = new TreeCanvas(layout, reciever, scrollArea->viewport());

    connect(scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            canvasTwo, SLOT(scroll(void)));
    connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
            canvasTwo, SLOT(scroll(void)));

    layout->addWidget(scrollArea, 0, 0, -1, 1);
    layout->addWidget(canvasTwo->scaleBar, 1,1, Qt::AlignHCenter);

    scrollArea->viewport()->setLayout(nc_layout);

    nc_layout->addWidget(canvasTwo);
    
    // connect(canvasTwo->_builder, SIGNAL(doneBuilding(void)), reciever, SLOT(updateCanvas(void)));
    connect(canvasTwo->_builder, SIGNAL(doneBuilding(void)), canvasTwo, SLOT(finalizeCanvas(void)));
    
    canvasDialog->resize(500, 400);
   
    canvasDialog->show();
}

void 
Gist::initiateComparison(void) {

    /// Canvas related nonsense (incapsulate?)
    QGridLayout* layout = new QGridLayout(this);
    cmpDialog = new QDialog(this);
    cmpDialog->setLayout(layout);

    QVBoxLayout* nc_layout = new QVBoxLayout();
    QAbstractScrollArea* scrollArea = new QAbstractScrollArea(cmpDialog);

    cmpCanvas = new TreeCanvas(layout, reciever, scrollArea->viewport());

    connect(scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            cmpCanvas, SLOT(scroll(void)));
    connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
            cmpCanvas, SLOT(scroll(void)));

    layout->addWidget(scrollArea, 0, 0, -1, 1);
    layout->addWidget(cmpCanvas->scaleBar, 1,1, Qt::AlignHCenter);

    scrollArea->viewport()->setLayout(nc_layout);

    nc_layout->addWidget(cmpCanvas);

    TreeComparison::compare(canvas, canvasTwo, cmpCanvas);

    cmpDialog->resize(500, 400);
    cmpDialog->show();
    
}

void
Gist::initInterface(void) {

    layout = new QGridLayout(this);

    scrollArea = new QAbstractScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setAutoFillBackground(true);

    myPalette = new QPalette(scrollArea->palette());

    myPalette->setColor(QPalette::Window, Qt::white);
    scrollArea->setPalette(*myPalette);

    setLayout(layout);
}

Gist::Gist(QWidget* parent) : QWidget(parent) {

    initInterface();
    addActions();
    
    reciever = new RecieverThread(this);

    canvas = new TreeCanvas(layout, reciever, scrollArea->viewport());
    canvas->setPalette(*myPalette);
    canvas->setObjectName("canvas");

    layout->addWidget(scrollArea, 0,0,-1,1);
    layout->addWidget(canvas->scaleBar, 1,1, Qt::AlignHCenter);
    
    connectCanvas(canvas);

    connect(canvas->_builder, SIGNAL(doneBuilding(void)), canvas, SLOT(finalizeCanvas(void)));
    connect(initComparison, SIGNAL(triggered()), this, SLOT(initiateComparison()));

    connect(scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            canvas, SLOT(scroll(void)));
    connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
            canvas, SLOT(scroll(void)));

//    connect(canvas, SIGNAL(solution(const Space*)),
//            this, SIGNAL(solution(const Space*)));

    // create new TreeCanvas when reciever gets new data
    connect(reciever, SIGNAL(newCanvasNeeded()), this, SLOT(createNewCanvas(void)));


    nodeStatInspector = new NodeStatInspector(this);

    reciever->recieve(canvas);
    canvas->show();

    resize(500, 400);

    QVBoxLayout* sa_layout = new QVBoxLayout();
    sa_layout->setContentsMargins(0,0,0,0);
    sa_layout->addWidget(canvas);

    scrollArea->viewport()->setLayout(sa_layout);

    


    // enables on_<sender>_<signal>() mechanism
    QMetaObject::connectSlotsByName(this);
}


// void
// Gist::resetCanvas(TreeCanvas* canvas, TreeBuilder* builder, bool isRestarts) {
//     canvas->reset(isRestarts);
//     builder->reset(canvas->data, canvas->na);
// }

void
Gist::resizeEvent(QResizeEvent*) {
    canvas->resizeToOuter();
}

//void
//Gist::addInspector(Inspector* i0, QAction*& nas, QAction*& nad,
//                   QAction*&nam) {
//    if (doubleClickInspectorGroup->
//            actions().indexOf(nullDoubleClickInspector) != -1) {
//        doubleClickInspectorGroup->removeAction(nullDoubleClickInspector);
//        solutionInspectorGroup->removeAction(nullSolutionInspector);
//        moveInspectorGroup->removeAction(nullMoveInspector);
//    }
//    canvas->addSolutionInspector(i0);
//    canvas->addDoubleClickInspector(i0);
//    canvas->addMoveInspector(i0);

//    nas = new QAction(i0->name().c_str(), this);
//    nas->setCheckable(true);
//    solutionInspectorGroup->addAction(nas);
//    solutionInspectorMenu->clear();
//    solutionInspectorMenu->addActions(solutionInspectorGroup->actions());

//    nad = new QAction(i0->name().c_str(), this);
//    nad->setCheckable(true);
//    doubleClickInspectorGroup->addAction(nad);
//    doubleClickInspectorMenu->clear();
//    doubleClickInspectorMenu->addActions(
//                doubleClickInspectorGroup->actions());

//    nam = new QAction(i0->name().c_str(), this);
//    nam->setCheckable(true);
//    moveInspectorGroup->addAction(nam);
//    moveInspectorMenu->clear();
//    moveInspectorMenu->addActions(
//                moveInspectorGroup->actions());
    
//    QAction* ia = new QAction(i0->name().c_str(), this);
//    inspectGroup->addAction(ia);
//    QAction* ibfpa = new QAction(i0->name().c_str(), this);
//    inspectBeforeFPGroup->addAction(ibfpa);

//    if (inspectGroup->actions().size() < 10) {
//        ia->setShortcut(QKeySequence(QString("Ctrl+")+
//                                     QString("").setNum(inspectGroup->actions().size())));
//        ibfpa->setShortcut(QKeySequence(QString("Ctrl+Alt+")+
//                                        QString("").setNum(inspectBeforeFPGroup->actions().size())));
//    }
//}

//void
//Gist::addSolutionInspector(Inspector* ins) {
//    QAction* nas;
//    QAction* nad;
//    QAction* nam;
//    if (doubleClickInspectorGroup->
//            actions().indexOf(nullDoubleClickInspector) == -1) {
//        QList<QAction*> is = solutionInspectorGroup->actions();
//        for (int i=0; i<is.size(); i++) {
//            canvas->activateSolutionInspector(i,false);
//            is[i]->setChecked(false);
//        }
//    }
//    addInspector(ins, nas,nad,nam);
//    nas->setChecked(true);
//    selectSolutionInspector(nas);
//}

//void
//Gist::addDoubleClickInspector(Inspector* ins) {
//    QAction* nas;
//    QAction* nad;
//    QAction* nam;
//    if (doubleClickInspectorGroup->
//            actions().indexOf(nullDoubleClickInspector) == -1) {
//        QList<QAction*> is = doubleClickInspectorGroup->actions();
//        for (int i=0; i<is.size(); i++) {
//            canvas->activateDoubleClickInspector(i,false);
//            is[i]->setChecked(false);
//        }
//    }
//    addInspector(ins, nas,nad,nam);
//    nad->setChecked(true);
//    selectDoubleClickInspector(nad);
//}

//void
//Gist::addMoveInspector(Inspector* ins) {
//    QAction* nas;
//    QAction* nad;
//    QAction* nam;
//    if (doubleClickInspectorGroup->
//            actions().indexOf(nullDoubleClickInspector) == -1) {
//        QList<QAction*> is = moveInspectorGroup->actions();
//        for (int i=0; i<is.size(); i++) {
//            canvas->activateMoveInspector(i,false);
//            is[i]->setChecked(false);
//        }
//    }
//    addInspector(ins, nas,nad,nam);
//    nam->setChecked(true);
//    selectMoveInspector(nam);
//}

//void
//Gist::addComparator(Comparator* c) {
//    if (comparatorGroup->actions().indexOf(nullComparator) == -1) {
//        QList<QAction*> is = comparatorGroup->actions();
//        for (int i=0; i<is.size(); i++) {
//            canvas->activateComparator(i,false);
//            is[i]->setChecked(false);
//        }
//    } else {
//        comparatorGroup->removeAction(nullComparator);
//    }
//    canvas->addComparator(c);

//    QAction* ncs = new QAction(c->name().c_str(), this);
//    ncs->setCheckable(true);
//    comparatorGroup->addAction(ncs);
//    comparatorMenu->clear();
//    comparatorMenu->addActions(comparatorGroup->actions());
//    ncs->setChecked(true);
//    selectComparator(ncs);
//}

Gist::~Gist(void) { delete canvas; }

void
Gist::prepareNewCanvas(void) {
    // canvas->hide();
//    canvasDialog.
//    canvasDialog.show();
    canvasTwo->show();

}

void
Gist::on_canvas_contextMenu(QContextMenuEvent* event) {
    contextMenu->popup(event->globalPos());
}

void
Gist::on_canvas_statusChanged(VisualNode* n, const Statistics& stats,
                              bool finished) {
    nodeStatInspector->node(*canvas->na,n,stats,finished);
    if (!finished) {
        inspect->setEnabled(false);
        inspectGroup->setEnabled(false);
        inspectBeforeFP->setEnabled(false);
        inspectBeforeFPGroup->setEnabled(false);
        compareNode->setEnabled(false);
        compareNodeBeforeFP->setEnabled(false);
        showNodeStats->setEnabled(false);
        stop->setEnabled(true);
        reset->setEnabled(false);
        navUp->setEnabled(false);
        navDown->setEnabled(false);
        navLeft->setEnabled(false);
        navRight->setEnabled(false);
        navRoot->setEnabled(false);
        navNextSol->setEnabled(false);
        navPrevSol->setEnabled(false);

        searchNext->setEnabled(false);
        searchAll->setEnabled(false);
        toggleHidden->setEnabled(false);
        hideFailed->setEnabled(false);
        unhideAll->setEnabled(false);
        labelBranches->setEnabled(false);
        labelPath->setEnabled(false);
        analyzeSimilarSubtrees->setEnabled(false);
        initComparison->setEnabled(false);

        toggleStop->setEnabled(false);
        unstopAll->setEnabled(false);

        center->setEnabled(false);
        exportPDF->setEnabled(false);
        exportWholeTreePDF->setEnabled(false);
        print->setEnabled(false);

        setPath->setEnabled(false);
        inspectPath->setEnabled(false);
        bookmarkNode->setEnabled(false);
        bookmarksGroup->setEnabled(false);
    } else {
        stop->setEnabled(false);
        reset->setEnabled(true);
        sndCanvas->setEnabled(true);

        if ( (n->isOpen() || n->hasOpenChildren()) && (!n->isHidden()) ) {
            searchNext->setEnabled(true);
            searchAll->setEnabled(true);
        } else {
            searchNext->setEnabled(false);
            searchAll->setEnabled(false);
        }
        if (n->getNumberOfChildren() > 0) {
            navDown->setEnabled(true);
            toggleHidden->setEnabled(true);
            hideFailed->setEnabled(true);
            unhideAll->setEnabled(true);
            unstopAll->setEnabled(true);
        } else {
            navDown->setEnabled(false);
            toggleHidden->setEnabled(false);
            hideFailed->setEnabled(false);
            unhideAll->setEnabled(false);
            unstopAll->setEnabled(false);
        }

        toggleStop->setEnabled(n->getStatus() == STOP ||
                               n->getStatus() == UNSTOP);

        showNodeStats->setEnabled(true);
        inspect->setEnabled(true);
        labelPath->setEnabled(true);
        analyzeSimilarSubtrees->setEnabled(true);
        if (n->getStatus() == UNDETERMINED) {
            inspectGroup->setEnabled(false);
            inspectBeforeFP->setEnabled(false);
            inspectBeforeFPGroup->setEnabled(false);
            compareNode->setEnabled(false);
            compareNodeBeforeFP->setEnabled(false);
            labelBranches->setEnabled(false);
        } else {
            inspectGroup->setEnabled(true);
            inspectBeforeFP->setEnabled(true);
            inspectBeforeFPGroup->setEnabled(true);
            compareNode->setEnabled(true);
            compareNodeBeforeFP->setEnabled(true);
            labelBranches->setEnabled(!n->isHidden());
        }

        navRoot->setEnabled(true);
        VisualNode* p = n->getParent(*canvas->na);
        if (p == NULL) {
            inspectBeforeFP->setEnabled(false);
            inspectBeforeFPGroup->setEnabled(false);
            navUp->setEnabled(false);
            navRight->setEnabled(false);
            navLeft->setEnabled(false);
        } else {
            navUp->setEnabled(true);
            unsigned int alt = n->getAlternative(*canvas->na);
            navRight->setEnabled(alt + 1 < p->getNumberOfChildren());
            navLeft->setEnabled(alt > 0);
        }

        VisualNode* root = n;
        while (!root->isRoot())
            root = root->getParent(*canvas->na);
        NextSolCursor nsc(n, false, *canvas->na);
        PreorderNodeVisitor<NextSolCursor> nsv(nsc);
        nsv.run();
        navNextSol->setEnabled(nsv.getCursor().node() != root);

        NextSolCursor psc(n, true, *canvas->na);
        PreorderNodeVisitor<NextSolCursor> psv(psc);
        psv.run();
        navPrevSol->setEnabled(psv.getCursor().node() != root);

        center->setEnabled(true);
        exportPDF->setEnabled(true);
        exportWholeTreePDF->setEnabled(true);
        print->setEnabled(true);

        setPath->setEnabled(true);
        inspectPath->setEnabled(true);

        bookmarkNode->setEnabled(true);
        bookmarksGroup->setEnabled(true);
    }
    emit statusChanged(stats,finished);
}

void
Gist::inspectWithAction(QAction* a) {
    canvas->inspectCurrentNode(true,inspectGroup->actions().indexOf(a));
}

void
Gist::inspectBeforeFPWithAction(QAction* a) {
    canvas->inspectCurrentNode(false,
                               inspectBeforeFPGroup->actions().indexOf(a));
}

bool
Gist::finish(void) {
    return canvas->finish();
}

//void
//Gist::selectDoubleClickInspector(QAction* a) {
//    canvas->activateDoubleClickInspector(
//                doubleClickInspectorGroup->actions().indexOf(a),
//                a->isChecked());
//}
//void
//Gist::selectSolutionInspector(QAction* a) {
//    canvas->activateSolutionInspector(
//                solutionInspectorGroup->actions().indexOf(a),
//                a->isChecked());
//}
//void
//Gist::selectMoveInspector(QAction* a) {
//    canvas->activateMoveInspector(
//                moveInspectorGroup->actions().indexOf(a),
//                a->isChecked());
//}
//void
//Gist::selectComparator(QAction* a) {
//    canvas->activateComparator(comparatorGroup->actions().indexOf(a),
//                               a->isChecked());
//}
void
Gist::selectBookmark(QAction* a) {
    int idx = bookmarksGroup->actions().indexOf(a);
    canvas->setCurrentNode(canvas->bookmarks[idx]);
    canvas->centerCurrentNode();
}

void
Gist::addBookmark(const QString& id) {
    if (bookmarksGroup->actions().indexOf(nullBookmark) != -1) {
        bookmarksGroup->removeAction(nullBookmark);
    }

    QAction* nb = new QAction(id, this);
    nb->setCheckable(true);
    bookmarksGroup->addAction(nb);
}

void
Gist::removeBookmark(int idx) {
    QAction* a = bookmarksGroup->actions()[idx];
    bookmarksGroup->removeAction(a);
    if (bookmarksGroup->actions().size() == 0) {
        bookmarksGroup->addAction(nullBookmark);
    }
}

void
Gist::populateBookmarksMenu(void) {
    bookmarksMenu->clear();
    bookmarksMenu->addAction(bookmarkNode);
    bookmarksMenu->addSeparator();
    bookmarksMenu->addActions(bookmarksGroup->actions());
}

void
Gist::populateInspectors(void) {
    inspectNodeMenu->clear();
    inspectNodeMenu->addAction(inspect);
    inspectNodeMenu->addSeparator();
    inspectNodeMenu->addActions(inspectGroup->actions());
    inspectNodeBeforeFPMenu->clear();
    inspectNodeBeforeFPMenu->addAction(inspectBeforeFP);
    inspectNodeBeforeFPMenu->addSeparator();
    inspectNodeBeforeFPMenu->addActions(inspectBeforeFPGroup->actions());
}

void
Gist::setAutoHideFailed(bool b) { canvas->setAutoHideFailed(b); }
void
Gist::setAutoZoom(bool b) { canvas->setAutoZoom(b); }
bool
Gist::getAutoHideFailed(void) { return canvas->getAutoHideFailed(); }
bool
Gist::getAutoZoom(void) { return canvas->getAutoZoom(); }
void
Gist::setRefresh(int i) { canvas->setRefresh(i); }
void
Gist::setRefreshPause(int i) { canvas->setRefreshPause(i); }
bool
Gist::getSmoothScrollAndZoom(void) {
    return canvas->getSmoothScrollAndZoom();
}
void
Gist::setSmoothScrollAndZoom(bool b) {
    canvas->setSmoothScrollAndZoom(b);
}
bool
Gist::getMoveDuringSearch(void) {
    return canvas->getMoveDuringSearch();
}
void
Gist::setMoveDuringSearch(bool b) {
    canvas->setMoveDuringSearch(b);
}
void
Gist::showStats(void) {
    // nodeStatInspector->showStats();
    // canvas->emitStatusChanged();
}



void
Gist::addActions(void) {
    inspect = new QAction("Inspect", this);
    inspect->setShortcut(QKeySequence("Return"));
    
    inspectBeforeFP = new QAction("Inspect before fixpoint", this);
    inspectBeforeFP->setShortcut(QKeySequence("Ctrl+Return"));

    stop = new QAction("Stop search", this);
    stop->setShortcut(QKeySequence("Esc"));

    reset = new QAction("Reset", this);
    reset->setShortcut(QKeySequence("Ctrl+R"));

    initComparison = new QAction("Compare", this);
    /// TODO: make unavailable when no data

    sndCanvas = new QAction("Allow second canvas", this);
    sndCanvas->setCheckable(true);
    sndCanvas->setChecked(true);
    /// TODO: set a shortcut
    
    navUp = new QAction("Up", this);
    navUp->setShortcut(QKeySequence("Up"));
    
    navDown = new QAction("Down", this);
    navDown->setShortcut(QKeySequence("Down"));
    
    navLeft = new QAction("Left", this);
    navLeft->setShortcut(QKeySequence("Left"));
    
    navRight = new QAction("Right", this);
    navRight->setShortcut(QKeySequence("Right"));
    
    navRoot = new QAction("Root", this);
    navRoot->setShortcut(QKeySequence("R"));
    
    navNextSol = new QAction("To next solution", this);
    navNextSol->setShortcut(QKeySequence("Shift+Right"));
    
    navPrevSol = new QAction("To previous solution", this);
    navPrevSol->setShortcut(QKeySequence("Shift+Left"));
    
    searchNext = new QAction("Next solution", this);
    searchNext->setShortcut(QKeySequence("N"));
    
    searchAll = new QAction("All solutions", this);
    searchAll->setShortcut(QKeySequence("A"));
    
    toggleHidden = new QAction("Hide/unhide", this);
    toggleHidden->setShortcut(QKeySequence("H"));
    
    hideFailed = new QAction("Hide failed subtrees", this);
    hideFailed->setShortcut(QKeySequence("F"));
    
    unhideAll = new QAction("Unhide all", this);
    unhideAll->setShortcut(QKeySequence("U"));
    
    labelBranches = new QAction("Label/clear branches", this);
    labelBranches->setShortcut(QKeySequence("L"));
    
    labelPath = new QAction("Label/clear path", this);
    labelPath->setShortcut(QKeySequence("Shift+L"));

    analyzeSimilarSubtrees = new QAction("Analyze similar subtrees", this);
    analyzeSimilarSubtrees->setShortcut(QKeySequence("Shift+s"));
    
    toggleStop = new QAction("Stop/unstop", this);
    toggleStop->setShortcut(QKeySequence("X"));
    
    unstopAll = new QAction("Do not stop in subtree", this);
    unstopAll->setShortcut(QKeySequence("Shift+X"));   

    zoomToFit = new QAction("Zoom to fit", this);
    zoomToFit->setShortcut(QKeySequence("Z"));
    
    center = new QAction("Center current node", this);
    center->setShortcut(QKeySequence("C"));
    
    exportPDF = new QAction("Export subtree PDF...", this);
    exportPDF->setShortcut(QKeySequence("P"));
    
    exportWholeTreePDF = new QAction("Export PDF...", this);
    exportWholeTreePDF->setShortcut(QKeySequence("Ctrl+Shift+P"));
    
    print = new QAction("Print...", this);
    print->setShortcut(QKeySequence("Ctrl+P"));
    
    bookmarkNode = new QAction("Add/remove bookmark", this);
    bookmarkNode->setShortcut(QKeySequence("Shift+B"));
    
    compareNode = new QAction("Compare", this);
    compareNode->setShortcut(QKeySequence("V"));
    
    compareNodeBeforeFP = new QAction("Compare before fixpoint", this);
    compareNodeBeforeFP->setShortcut(QKeySequence("Ctrl+V"));

    nullBookmark = new QAction("<none>",this);
    nullBookmark->setCheckable(true);
    nullBookmark->setChecked(false);
    nullBookmark->setEnabled(false);

    bookmarksGroup = new QActionGroup(this);
    bookmarksGroup->setExclusive(false);
    bookmarksGroup->addAction(nullBookmark);
    connect(bookmarksGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(selectBookmark(QAction*)));

    bookmarksMenu = new QMenu("Bookmarks");
    connect(bookmarksMenu, SIGNAL(aboutToShow()),
            this, SLOT(populateBookmarksMenu()));

    setPath = new QAction("Set path", this);
    setPath->setShortcut(QKeySequence("Shift+P"));
    
    inspectPath = new QAction("Inspect path", this);
    inspectPath->setShortcut(QKeySequence("Shift+I"));  

    showNodeStats = new QAction("Node statistics", this);
    showNodeStats->setShortcut(QKeySequence("S"));
    connect(showNodeStats, SIGNAL(triggered()),
            this, SLOT(showStats()));

    nullSolutionInspector = new QAction("<none>",this);
    nullSolutionInspector->setCheckable(true);
    nullSolutionInspector->setChecked(false);
    nullSolutionInspector->setEnabled(false);
    solutionInspectorGroup = new QActionGroup(this);
    solutionInspectorGroup->setExclusive(false);
    solutionInspectorGroup->addAction(nullSolutionInspector);
//    connect(solutionInspectorGroup, SIGNAL(triggered(QAction*)),
//            this, SLOT(selectSolutionInspector(QAction*)));

    nullDoubleClickInspector = new QAction("<none>",this);
    nullDoubleClickInspector->setCheckable(true);
    nullDoubleClickInspector->setChecked(false);
    nullDoubleClickInspector->setEnabled(false);
    doubleClickInspectorGroup = new QActionGroup(this);
    doubleClickInspectorGroup->setExclusive(false);
    doubleClickInspectorGroup->addAction(nullDoubleClickInspector);
//    connect(doubleClickInspectorGroup, SIGNAL(triggered(QAction*)),
//            this, SLOT(selectDoubleClickInspector(QAction*)));

    nullMoveInspector = new QAction("<none>",this);
    nullMoveInspector->setCheckable(true);
    nullMoveInspector->setChecked(false);
    nullMoveInspector->setEnabled(false);
    moveInspectorGroup = new QActionGroup(this);
    moveInspectorGroup->setExclusive(false);
    moveInspectorGroup->addAction(nullMoveInspector);
//    connect(moveInspectorGroup, SIGNAL(triggered(QAction*)),
//            this, SLOT(selectMoveInspector(QAction*)));

    nullComparator = new QAction("<none>",this);
    nullComparator->setCheckable(true);
    nullComparator->setChecked(false);
    nullComparator->setEnabled(false);
    comparatorGroup = new QActionGroup(this);
    comparatorGroup->setExclusive(false);
    comparatorGroup->addAction(nullComparator);
//    connect(comparatorGroup, SIGNAL(triggered(QAction*)),
//            this, SLOT(selectComparator(QAction*)));

    solutionInspectorMenu = new QMenu("Solution inspectors");
    solutionInspectorMenu->addActions(solutionInspectorGroup->actions());
    doubleClickInspectorMenu = new QMenu("Double click inspectors");
    doubleClickInspectorMenu->addActions(
                doubleClickInspectorGroup->actions());
    moveInspectorMenu = new QMenu("Move inspectors");
    moveInspectorMenu->addActions(moveInspectorGroup->actions());
    comparatorMenu = new QMenu("Comparators");
    comparatorMenu->addActions(comparatorGroup->actions());

    inspectGroup = new QActionGroup(this);
    connect(inspectGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(inspectWithAction(QAction*)));
    inspectBeforeFPGroup = new QActionGroup(this);
    connect(inspectBeforeFPGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(inspectBeforeFPWithAction(QAction*)));

    inspectNodeMenu = new QMenu("Inspect");
    inspectNodeMenu->addAction(inspect);
    connect(inspectNodeMenu, SIGNAL(aboutToShow()),
            this, SLOT(populateInspectors()));

    inspectNodeBeforeFPMenu = new QMenu("Inspect before fixpoint");
    inspectNodeBeforeFPMenu->addAction(inspectBeforeFP);
    connect(inspectNodeBeforeFPMenu, SIGNAL(aboutToShow()),
            this, SLOT(populateInspectors()));
    populateInspectors();

    addAction(inspect);
    addAction(inspectBeforeFP);
    addAction(compareNode);
    addAction(compareNodeBeforeFP);
    addAction(stop);
    addAction(reset);
    addAction(sndCanvas);
    addAction(initComparison);
    addAction(navUp);
    addAction(navDown);
    addAction(navLeft);
    addAction(navRight);
    addAction(navRoot);
    addAction(navNextSol);
    addAction(navPrevSol);

    addAction(searchNext);
    addAction(searchAll);
    addAction(toggleHidden);
    addAction(hideFailed);
    addAction(unhideAll);
    addAction(labelBranches);
    addAction(labelPath);
    addAction(analyzeSimilarSubtrees);
    addAction(toggleStop);
    addAction(unstopAll);
    addAction(zoomToFit);
    addAction(center);
    addAction(exportPDF);
    addAction(exportWholeTreePDF);
    addAction(print);

    addAction(setPath);
    addAction(inspectPath);
    addAction(showNodeStats);

    contextMenu = new QMenu(this);
    contextMenu->addMenu(inspectNodeMenu);
    contextMenu->addMenu(inspectNodeBeforeFPMenu);
    contextMenu->addAction(compareNode);
    contextMenu->addAction(compareNodeBeforeFP);
    contextMenu->addAction(showNodeStats);
    contextMenu->addAction(center);

    contextMenu->addSeparator();

    contextMenu->addAction(searchNext);
    contextMenu->addAction(searchAll);

    contextMenu->addSeparator();

    contextMenu->addAction(toggleHidden);
    contextMenu->addAction(hideFailed);
    contextMenu->addAction(unhideAll);
    contextMenu->addAction(labelBranches);
    contextMenu->addAction(labelPath);
    contextMenu->addAction(analyzeSimilarSubtrees);

    contextMenu->addAction(toggleStop);
    contextMenu->addAction(unstopAll);

    contextMenu->addSeparator();

    contextMenu->addMenu(bookmarksMenu);
    contextMenu->addAction(setPath);
    contextMenu->addAction(inspectPath);

    contextMenu->addSeparator();

    contextMenu->addMenu(doubleClickInspectorMenu);
    contextMenu->addMenu(solutionInspectorMenu);
    contextMenu->addMenu(moveInspectorMenu);
}

void
Gist::connectCanvas(TreeCanvas* tc, TreeCanvas* old_tc) {
    if (old_tc) {
        disconnect(reciever, SIGNAL(startWork(void)), old_tc->_builder, SLOT(startBuilding(void)));
        disconnect(inspect, SIGNAL(triggered()), old_tc, SLOT(inspectCurrentNode()));
        disconnect(inspectBeforeFP, SIGNAL(triggered()), old_tc, SLOT(inspectBeforeFP(void)));
        disconnect(stop, SIGNAL(triggered()), old_tc, SLOT(stopSearch()));
        disconnect(reset, SIGNAL(triggered()), old_tc, SLOT(reset()));
        disconnect(sndCanvas, SIGNAL(triggered()), old_tc, SLOT(compareTrees()));
        disconnect(navUp, SIGNAL(triggered()), old_tc, SLOT(navUp()));
        disconnect(navDown, SIGNAL(triggered()), old_tc, SLOT(navDown()));
        disconnect(navLeft, SIGNAL(triggered()), old_tc, SLOT(navLeft()));
        disconnect(navRight, SIGNAL(triggered()), old_tc, SLOT(navRight()));
        disconnect(navRoot, SIGNAL(triggered()), old_tc, SLOT(navRoot()));
        disconnect(navNextSol, SIGNAL(triggered()), old_tc, SLOT(navNextSol()));
        disconnect(navPrevSol, SIGNAL(triggered()), old_tc, SLOT(navPrevSol()));
        disconnect(toggleHidden, SIGNAL(triggered()), old_tc, SLOT(toggleHidden()));
        disconnect(hideFailed, SIGNAL(triggered()), old_tc, SLOT(hideFailed()));
        disconnect(labelBranches, SIGNAL(triggered()), old_tc, SLOT(labelBranches()));
        disconnect(unhideAll, SIGNAL(triggered()), old_tc, SLOT(unhideAll()));
        disconnect(labelPath, SIGNAL(triggered()), old_tc, SLOT(labelPath()));
        disconnect(analyzeSimilarSubtrees, SIGNAL(triggered()), old_tc, SLOT(analyzeSimilarSubtrees()));
        disconnect(toggleStop, SIGNAL(triggered()), old_tc, SLOT(toggleStop()));
        disconnect(unstopAll, SIGNAL(triggered()), old_tc, SLOT(unstopAll()));
        disconnect(zoomToFit, SIGNAL(triggered()), old_tc, SLOT(zoomToFit()));
        disconnect(center, SIGNAL(triggered()), old_tc, SLOT(centerCurrentNode()));
        disconnect(exportWholeTreePDF, SIGNAL(triggered()), old_tc, SLOT(exportWholeTreePDF()));
        disconnect(exportPDF, SIGNAL(triggered()), old_tc, SLOT(exportPDF()));
        disconnect(print, SIGNAL(triggered()), old_tc, SLOT(print()));
        disconnect(bookmarkNode, SIGNAL(triggered()), old_tc, SLOT(bookmarkNode()));
        disconnect(compareNode, SIGNAL(triggered()), old_tc, SLOT(startCompareNodes()));
        disconnect(compareNodeBeforeFP, SIGNAL(triggered()), old_tc, SLOT(startCompareNodesBeforeFP()));
        disconnect(old_tc, SIGNAL(addedBookmark(const QString&)), this, SLOT(addBookmark(const QString&)));
        disconnect(old_tc, SIGNAL(removedBookmark(int)), this, SLOT(removeBookmark(int)));
        disconnect(setPath, SIGNAL(triggered()), old_tc, SLOT(setPath()));
        disconnect(inspectPath, SIGNAL(triggered()), old_tc, SLOT(inspectPath()));
    }
    
    connect(reciever, SIGNAL(startWork(void)), tc->_builder, SLOT(startBuilding(void)));
    connect(inspect, SIGNAL(triggered()), tc, SLOT(inspectCurrentNode()));
    connect(inspectBeforeFP, SIGNAL(triggered()), tc, SLOT(inspectBeforeFP(void)));
    connect(stop, SIGNAL(triggered()), tc, SLOT(stopSearch()));
    connect(reset, SIGNAL(triggered()), tc, SLOT(reset()));
    connect(sndCanvas, SIGNAL(triggered()), tc, SLOT(compareTrees()));
    connect(navUp, SIGNAL(triggered()), tc, SLOT(navUp()));
    connect(navDown, SIGNAL(triggered()), tc, SLOT(navDown()));
    connect(navLeft, SIGNAL(triggered()), tc, SLOT(navLeft()));
    connect(navRight, SIGNAL(triggered()), tc, SLOT(navRight()));
    connect(navRoot, SIGNAL(triggered()), tc, SLOT(navRoot()));
    connect(navNextSol, SIGNAL(triggered()), tc, SLOT(navNextSol()));
    connect(navPrevSol, SIGNAL(triggered()), tc, SLOT(navPrevSol()));
    connect(toggleHidden, SIGNAL(triggered()), tc, SLOT(toggleHidden()));
    connect(hideFailed, SIGNAL(triggered()), tc, SLOT(hideFailed()));
    connect(labelBranches, SIGNAL(triggered()), tc, SLOT(labelBranches()));
    connect(unhideAll, SIGNAL(triggered()), tc, SLOT(unhideAll()));
    connect(labelPath, SIGNAL(triggered()), tc, SLOT(labelPath()));
    connect(analyzeSimilarSubtrees, SIGNAL(triggered()), tc, SLOT(analyzeSimilarSubtrees()));
    connect(toggleStop, SIGNAL(triggered()), tc, SLOT(toggleStop()));
    connect(unstopAll, SIGNAL(triggered()), tc, SLOT(unstopAll()));
    connect(zoomToFit, SIGNAL(triggered()), tc, SLOT(zoomToFit()));
    connect(center, SIGNAL(triggered()), tc, SLOT(centerCurrentNode()));
    connect(exportWholeTreePDF, SIGNAL(triggered()), tc, SLOT(exportWholeTreePDF()));
    connect(exportPDF, SIGNAL(triggered()), tc, SLOT(exportPDF()));
    connect(print, SIGNAL(triggered()), tc, SLOT(print()));
    connect(bookmarkNode, SIGNAL(triggered()), tc, SLOT(bookmarkNode()));
    connect(compareNode, SIGNAL(triggered()), tc, SLOT(startCompareNodes()));
    connect(compareNodeBeforeFP, SIGNAL(triggered()), tc, SLOT(startCompareNodesBeforeFP()));
    connect(tc, SIGNAL(addedBookmark(const QString&)), this, SLOT(addBookmark(const QString&)));
    connect(tc, SIGNAL(removedBookmark(int)), this, SLOT(removeBookmark(int)));
    connect(setPath, SIGNAL(triggered()), tc, SLOT(setPath()));
    connect(inspectPath, SIGNAL(triggered()), tc, SLOT(inspectPath()));
}
