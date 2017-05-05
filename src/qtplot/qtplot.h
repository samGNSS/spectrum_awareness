#ifndef __QT_QUI_PLOTTER__
#define __QT_QUI_PLOTTER__

#include <stdint.h>
#include <cstdio>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../util/radarDataTypes.h"
#include "../util/logger/consoleLog.h"

//qt and qwt
#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_painter.h>
#include <qwt-qt4/qwt_plot_canvas.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_scale_engine.h>
#include <qwt-qt4/qwt_scale_widget.h>
#include <qwt-qt4/qwt_plot_zoomer.h>
#include <qwt-qt4/qwt_plot_panner.h>
#include <qwt-qt4/qwt_plot_magnifier.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_symbol.h>
#include <qwt-qt4/qwt_legend.h>
#include <qwt-qt4/qwt_plot_picker.h>
#include <QString>
#include <QCoreApplication>

#include <qwt-qt4/qwt_legend_item.h>



class qui{
public:
  qui();
  ~qui();
  void init();
  void stop();
  void watchQueue();
  
  std::queue<std::vector<radar::cfarDet>>* getQueue();
  
  
  
private:
  
  std::queue<std::vector<radar::cfarDet>> detQueue;
  std::vector<radar::cfarDet> dets;
  
  console* log;
  
  bool enabled;
  
  std::thread queueMonitor;
  std::mutex disPlayMtx;
  std::condition_variable waitForDets;
  
  
};

#endif