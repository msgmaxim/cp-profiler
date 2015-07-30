/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SOLVER_TREE_DIALOG_HH
#define SOLVER_TREE_DIALOG_HH

#include <QStatusBar>
#include "base_tree_dialog.hh"

class Gist;
class ReceiverThread;

class SolverTreeDialog : public BaseTreeDialog {
Q_OBJECT

private:

  /// Status bar label for number of solutions
  QLabel* depthLabel;
  QLabel* solvedLabel;
  QLabel* failedLabel;
  QLabel* choicesLabel;
  QLabel* openLabel;

public:

  SolverTreeDialog(ReceiverThread* receiver, const CanvasType type, Gist* gist);

  ~SolverTreeDialog();


private Q_SLOTS:
  void statusChanged(VisualNode*, const Statistics& stats, bool finished);

};


#endif