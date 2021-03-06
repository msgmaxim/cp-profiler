/*  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "pixelview.hh"
#include <chrono>
#include <cmath>

using namespace std::chrono;

/// ******* PIXEL_TREE_DIALOG ********

PixelTreeDialog::PixelTreeDialog(TreeCanvas* tc)
  : QDialog(tc)
{

  this->resize(600, 400);

  /// set Title
  this->setWindowTitle(QString::fromStdString(tc->getTitle()));
  qDebug() << "title: " << tc->getTitle().c_str();

  setLayout(&layout);
  layout.addWidget(&scrollArea);
  layout.addLayout(&controlLayout);

  controlLayout.addWidget(&scaleDown);
  controlLayout.addWidget(&scaleUp);

  scaleUp.setText("+");
  scaleDown.setText("-");

  QLabel* compLabel = new QLabel("compression");
  compLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  controlLayout.addWidget(compLabel);

  controlLayout.addWidget(&compressionSB);
  compressionSB.setMinimum(1);
  compressionSB.setMaximum(10000);

  canvas = new PixelTreeCanvas(&scrollArea, tc);

  connect(&scaleDown, SIGNAL(clicked()), canvas, SLOT(scaleDown()));
  connect(&scaleUp, SIGNAL(clicked()), canvas, SLOT(scaleUp()));

  QObject::connect(&compressionSB, SIGNAL(valueChanged(int)),
    canvas, SLOT(compressionChanged(int)));


  setAttribute(Qt::WA_QuitOnClose, true);
  setAttribute(Qt::WA_DeleteOnClose, true);

}

PixelTreeDialog::~PixelTreeDialog(void) {
  delete canvas;
}

PixelTreeCanvas::~PixelTreeCanvas(void) {
  freePixelList(pixelList);
  delete _image;

  delete [] time_arr;
  delete [] domain_arr;
  delete [] domain_red_arr;
}

/// ***********************************


/// ******** PIXEL_TREE_CANVAS ********

PixelTreeCanvas::PixelTreeCanvas(QWidget* parent, TreeCanvas* tc)
  : QWidget(parent), _tc(tc), _na(tc->na)
{

  _sa = static_cast<QAbstractScrollArea*>(parentWidget());
  _vScrollBar = _sa->verticalScrollBar();

  _nodeCount = tc->stats.solutions + tc->stats.failures
                       + tc->stats.choices + tc->stats.undetermined;

  max_depth = tc->stats.maxDepth;

  // if (_tc->getData()->isRestarts()) {
  //   max_depth++; /// consider the first, dummy node
  //   _nodeCount++;
  // }

  _image = nullptr;

  /// scrolling business
  _sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  _sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  _sa->setAutoFillBackground(true);
  
  drawPixelTree();
  actuallyDraw();


  pixmap.fromImage(*_image);
  qlabel.setPixmap(pixmap);
  qlabel.show();

}

void
PixelTreeCanvas::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  actuallyDraw();
  painter.drawImage(0, 0, *_image);
  
}


void
PixelTreeCanvas::constructTree(void) {
  /// the depth is max_depth
  /// the width is _nodeCount / approx_size
  freePixelList(pixelList);

  vlines = ceil((float)_nodeCount / approx_size);

  std::cerr << "vlines: " << vlines << "\n";
  delete [] time_arr;
  time_arr = new float[vlines]; //  TODO: "Uninitialised value was created by a heap allocation"

  delete [] domain_arr;
  domain_arr = new float[vlines]; //  TODO: "Uninitialised value was created by a heap allocation"

  delete [] domain_red_arr;
  domain_red_arr = new float[vlines]; //  TODO: "Uninitialised value was created by a heap allocation"

  pixelList.resize(vlines);

  /// get a root
  VisualNode* root = (*_na)[0];

  vline_idx    = 0;
  node_idx     = 0;
  group_size   = 0;
  group_domain = 0;
  group_domain_red    = 0;
  group_size_nonempty = 0;

  alpha_factor = 100.0 / approx_size;

  exploreNext(root, 1);

  flush();

}

void
PixelTreeCanvas::freePixelList(std::vector<std::list<PixelData*>>& pixelList) {
  for (auto& l : pixelList) {
    for (auto& nodeData : l) {
      delete nodeData;
    }
    l.clear();
  }
  pixelList.clear();
}

void
PixelTreeCanvas::actuallyDraw() {
  delete _image;

  _sa->horizontalScrollBar()->setRange(0, vlines * _step - _sa->width() + 100);
  _sa->verticalScrollBar()->setRange(0, max_depth * _step +
    4 * (HIST_HEIGHT + MARGIN + _step) - _sa->height()); // 4 histograms

  int xoff = _sa->horizontalScrollBar()->value();
  int yoff = _sa->verticalScrollBar()->value();

  unsigned int leftmost_x = xoff;
  unsigned int rightmost_x = xoff + _sa->width();

  pt_height = max_depth * _step_y;

  if (rightmost_x > pixelList.size() * _step) {
    rightmost_x = pixelList.size() * _step;
  }

  int img_height = MARGIN + 
                   pt_height + _step +
                   MARGIN +
                   HIST_HEIGHT + _step + /// Time Histogram
                   MARGIN +
                   HIST_HEIGHT + _step + /// Domain Histogram
                   MARGIN +
                   HIST_HEIGHT + _step + /// Domain Reduction Histogram
                   MARGIN +
                   HIST_HEIGHT + _step + /// Node Rate Histogram
                   MARGIN;

  _image = new QImage(rightmost_x - leftmost_x + _step, img_height, QImage::Format_RGB888);

  _image->fill(qRgb(255, 255, 255));

  this->resize(_image->width(), _image->height());

  unsigned leftmost_vline = leftmost_x / _step;
  unsigned rightmost_vline = rightmost_x / _step;

  int* intencity_arr = new int[max_depth + 1];

  int node_idx = 0;

  for (unsigned int vline = leftmost_vline; vline < rightmost_vline; vline++) {
    if (!pixelList[vline].empty()) {

      memset(intencity_arr, 0, (max_depth + 1)* sizeof(int));

      for (auto& pixel : pixelList[vline]) {


        int xpos = (vline  - leftmost_vline) * _step;
        int ypos = pixel->depth() * _step_y - yoff;


        intencity_arr[pixel->depth()]++;

        /// draw pixel itself:
        if (ypos > 0) {
          if (!pixel->node()->isSelected()) {
            int alpha = intencity_arr[pixel->depth()] * alpha_factor;
            drawPixel(xpos, ypos, QColor::fromHsv(150, 100, 100 - alpha).rgba());
          } else {
            // drawPixel(xpos, ypos, qRgb(255, 0, 0));
            drawPixel(xpos, ypos, qRgb(255, 0, 255));
          }
        }
        
        /// draw green vertical line if solved:
        if (pixel->node()->getStatus() == SOLVED) {

          for (unsigned j = 0; j < pt_height - yoff; j++)
            if (_image->pixel(xpos, j) == qRgb(255, 255, 255))
              for (unsigned i = 0; i < _step; i++)
                _image->setPixel(xpos + i, j, qRgb(0, 255, 0));

        }

        node_idx++;
      }
      
    }
  }

  delete [] intencity_arr;

  /// All Histograms

  // drawTimeHistogram(leftmost_vline, rightmost_vline);

  // drawDomainHistogram(leftmost_vline, rightmost_vline);

  // drawDomainReduction(leftmost_vline, rightmost_vline);

  // drawNodeRate(leftmost_vline, rightmost_vline);
  
}

void
PixelTreeCanvas::drawPixelTree(void) {

  high_resolution_clock::time_point time_begin = high_resolution_clock::now();

  constructTree();

  high_resolution_clock::time_point time_end = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(time_end - time_begin);
  std::cout << "Pixel Tree construction took: " << time_span.count() << " seconds." << std::endl;

}

void
PixelTreeCanvas::flush(void) {

  if (group_size == 0)
    return;

  if (group_size_nonempty == 0) {
    group_domain      = -1;
    group_domain_red  = -1;
    group_time        = -1;
  } else {
    group_domain        = group_domain / group_size_nonempty;
    group_domain_red    = group_domain_red / group_size_nonempty;
  }

  domain_arr[vline_idx]     = group_domain;
  domain_red_arr[vline_idx] = group_domain_red;
  time_arr[vline_idx]       =  group_time;

}

void
PixelTreeCanvas::exploreNext(VisualNode* node, unsigned depth) {

    //  Data* data = _tc->getData();
  DbEntry* entry = _tc->getEntry(node->getIndex(*_na));
  DbEntry* parent = nullptr;

  assert(depth <= max_depth);

  if (vline_idx >= pixelList.size()) return;

  pixelList[vline_idx].push_back(new PixelData(node_idx, node, depth));

  if (!entry) {
      // qDebug() << "entry (idx " << node->getIndex(*_na) << ") does not exist";
  } else {

    group_size_nonempty++;

    if (entry->parent_sid != ~0u) {
      parent = _tc->getEntry(node->getParent());

      if (parent) /// need this for restarts
        group_domain_red += parent->domain - entry->domain;
    }

    group_time   += entry->node_time;
    group_domain += entry->domain;


  }

  group_size++;

  if (group_size == approx_size) {
    /// get average domain size for the group
    if (group_size_nonempty == 0) {
      group_domain      = -1;
      group_domain_red  = -1;
      group_time        = -1;
    } else {
      group_domain        = group_domain / group_size_nonempty;
      group_domain_red    = group_domain_red / group_size_nonempty;
      
    }


    time_arr[vline_idx]       = group_time;
    domain_arr[vline_idx]     = group_domain;
    domain_red_arr[vline_idx] = group_domain_red;
    
    vline_idx++;
    std::cerr << "increased vline_idx to " << vline_idx << " where vlines is " << vlines << "\n";

    group_size = 0;
    group_time   = 0;
    group_domain = 0;
    group_domain_red = 0;
    group_size_nonempty = 0;

  }

  node_idx++;

  uint kids = node->getNumberOfChildren();
  for (uint i = 0; i < kids; ++i) {
    exploreNext(node->getChild(*_na, i), depth + 1);
  }


}

/// Draw time histogram underneath the pixel tree
void
PixelTreeCanvas::drawTimeHistogram(unsigned l_vline, unsigned r_vline) {

  drawHistogram(0, time_arr, l_vline, r_vline, qRgb(150, 150, 40));
}

void
PixelTreeCanvas::drawDomainHistogram(unsigned l_vline, unsigned r_vline) {
  drawHistogram(1, domain_arr, l_vline, r_vline, qRgb(150, 40, 150));
}

void
PixelTreeCanvas::drawDomainReduction(unsigned l_vline, unsigned r_vline) {
  drawHistogram(2, domain_red_arr, l_vline, r_vline, qRgb(40, 150, 150));
}

void
PixelTreeCanvas::drawHistogram(int idx, float* data, unsigned l_vline, unsigned r_vline, int color) {


  /// coordinates for the top-left corner
  int init_x = 0;
  int yoff = _sa->verticalScrollBar()->value();
  int y = (pt_height + _step) + MARGIN + idx * (HIST_HEIGHT + MARGIN + _step) - yoff;

  /// work out maximum value
  int max_value = 0;

  for (unsigned i = 0; i < vlines; i++) {
    if (data[i] > max_value) max_value = data[i];
  }

  if (max_value <= 0) return; /// no data for this histogram

  float coeff = (float)HIST_HEIGHT / max_value;

  int zero_level = y + HIST_HEIGHT + _step;

  for (unsigned i = l_vline; i < r_vline; i++) {
    int val = data[i] * coeff;

    /// horizontal line for 0 level
    for (unsigned j = 0; j < _step; j++)
      _image->setPixel(init_x + (i - l_vline) * _step + j,
                       zero_level, 
                       qRgb(150, 150, 150));

    // qDebug() << "data[" << i << "]: " << data[i];

    // if (data[i] < 0) continue;

    for (int v = val; v >= 0; v--) {
      drawPixel(init_x + (i - l_vline) * _step,
                y + HIST_HEIGHT - v,
                color);
    }
    

  }


}

void
PixelTreeCanvas::drawNodeRate(unsigned l_vline, unsigned r_vline) {
    Data* data = _tc->getExecution()->getData();
  std::vector<float>& node_rate = data->node_rate;
  std::vector<int>& nr_intervals = data->nr_intervals;

  int start_x = 0;
  int start_y = (pt_height + _step) + MARGIN + 3 * (HIST_HEIGHT + MARGIN + _step);

  float max_node_rate = *std::max_element(node_rate.begin(), node_rate.end());

  float coeff = (float)HIST_HEIGHT / max_node_rate;

  int zero_level = start_y + HIST_HEIGHT + _step;

  // / this is very slow
  for (unsigned i = l_vline; i < r_vline; i++) {
    for (unsigned j = 0; j < _step; j++)
      _image->setPixel(start_x + (i - l_vline) * _step + j,
                       zero_level, 
                       qRgb(150, 150, 150));
  }

  for (unsigned i = 1; i < nr_intervals.size(); i++) {
    float value = node_rate[i - 1] * coeff;
    unsigned i_begin = ceil((float)nr_intervals[i-1] / approx_size);
    unsigned i_end   = ceil((float)nr_intervals[i]   / approx_size);

    /// draw this interval?
    if (i_end < l_vline || i_begin > r_vline)
      continue;

    if (i_begin < l_vline) i_begin = l_vline;
    if (i_end   > r_vline) i_end   = r_vline;

    /// this is very slow
    // for (unsigned x = i_begin; x < i_end; x++) {

    //   for (int v = value; v >= 0; v--) {
    //     drawPixel(start_x + (x - l_vline) * _step,
    //               start_y + HIST_HEIGHT - v,
    //               qRgb(40, 40, 150));
    //   }
    // }
  }
}

void
PixelTreeCanvas::scaleUp(void) {
  _step++;
  _step_y++;
  actuallyDraw();
  repaint();
}

void
PixelTreeCanvas::scaleDown(void) {
  if (_step <= 1) return;
  _step--;
  _step_y--;
  actuallyDraw();
  repaint();
}

void
PixelTreeCanvas::compressionChanged(int value) {
  qDebug() << "compression is set to: " << value;
  approx_size = value;
  drawPixelTree();
  repaint();
}

void
PixelTreeCanvas::drawPixel(int x, int y, int color) {
  if (y < 0)
    return; /// TODO: fix later

  for (unsigned i = 0; i < _step; i++)
    for (unsigned j = 0; j < _step_y; j++)
      _image->setPixel(x + i, y + j, color);

}

void
PixelTreeCanvas::mousePressEvent(QMouseEvent* me) {

  int xoff = _sa->horizontalScrollBar()->value();
  int yoff = _sa->verticalScrollBar()->value();

  unsigned x = me->x() + xoff;
  unsigned y = me->y() + yoff;

  /// check boundaries:
  if (y > pt_height) return;

  // which node?

  unsigned vline = x / _step;

  selectNodesfromPT(vline);

  actuallyDraw();
  repaint();

}

void
PixelTreeCanvas::selectNodesfromPT(unsigned vline) {

  struct Actions {

  private:
    NodeAllocator* _na;
    TreeCanvas* _tc;
    int node_id;
    
    bool _done;

  public:

    void selectOne(VisualNode* node) {
      _tc->setCurrentNode(node);
      _tc->centerCurrentNode();
    }

    void selectGroup(VisualNode* node) {
      node->dirtyUp(*_na);

      VisualNode* next = node;

      while (!next->isRoot() && next->isHidden()) {
        next->setHidden(false);
        next = next->getParent(*_na);
      }
    }

  public:

    Actions(NodeAllocator* na, TreeCanvas* tc)
    : _na(na), _tc(tc), _done(false) {}

  };

  qDebug() << "selecting vline: " << vline;
  if (pixelList.size() <= vline) {
    qDebug() << "no such vline";
  }

  Actions actions(_na, _tc);
  void (Actions::*apply)(VisualNode*);

  /// unset currently selected nodes
  for (auto& node : nodes_selected) {
    node->setSelected(false);
  }

  nodes_selected.clear();

  std::list<PixelData*>& vline_list = pixelList[vline];

  if (vline_list.size() == 1) {
    apply = &Actions::selectOne;
  } else {
    apply = &Actions::selectGroup;

    /// hide everything except root
    _tc->hideAll();
    (*_na)[0]->setHidden(false);
  }

  for (auto& pixel : vline_list) {
    (actions.*apply)(pixel->node());
    pixel->node()->setSelected(true);
    nodes_selected.push_back(pixel->node());
  }

  
  _tc->update();

}

/// ***********************************
