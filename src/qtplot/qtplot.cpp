#include "qtplot.h"

#include <unistd.h>
#include <ctime>

qui* qui::quiInstance = 0;

qui* qui::getInstance(QWidget* parent){
  if(quiInstance == 0){
    quiInstance = new qui(parent);
  }
  return quiInstance;
}



qui::qui(QWidget* parent):QwtPlot(parent){
  log = new console();
  
  fftBin = new double[1024];
  time = new double[1024];
  
  std::memset(fftBin,0,sizeof(double)*1024);
  std::memset(time,0,sizeof(double)*1024);
  
}


qui::~qui(){
  if(enabled)
    this->stop();
  
  delete[] fftBin;
  delete[] time;
  delete log;
}

void qui::stop(){
  enabled = false;
  queueMonitor.join();
}


void qui::init(){
  enabled = true;
  
  this->initWindow();
  queueMonitor = std::thread(std::bind(&qui::watchQueue, this));
  
}


void qui::watchQueue(){
  while(enabled){
     
     //watch queue for new detections
     while(detQueue.empty() and enabled){usleep(100);};
     
     while(!detQueue.empty()){
       dets = detQueue.front();
       detQueue.pop();  
    }
  }
}


std::queue<std::vector<radar::cfarDet>>* qui::getQueue(){
  return &detQueue;
};


void qui::initWindow(){
  // Disable polygon clipping
    QwtPainter::setDeviceClipping(false);

    // We don't need the cache here
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

#if QT_VERSION >= 0x040000
#ifdef Q_WS_X11
    /*
       Qt::WA_PaintOnScreen is only supported for X11, but leads
       to substantial bugs with Qt 4.2.x/Windows
     */
    canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
#endif
#endif

    alignScales();
    
    // Assign a title
    setTitle("Raw Detections");
    insertLegend(new QwtLegend(), QwtPlot::BottomLegend);

    // Insert new curves
    QwtPlotCurve *cRight = new QwtPlotCurve("Detections");

    // Set curve styles
    cRight->setStyle(QwtPlotCurve::NoCurve);
    cRight->setPaintAttribute(QwtPlotCurve::PaintFiltered);

    const QColor &c = Qt::white;
    cRight->setSymbol(QwtSymbol(QwtSymbol::XCross,
            QBrush(c), QPen(Qt::darkGreen), QSize(15, 15)) );
    cRight->attach(this);
    
    cRight->setRawData(time, fftBin, 1024);
    
//     const bool cacheMode = 
//         canvas()->testPaintAttribute(QwtPlotCanvas::PaintCached);
//     
// #if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
//     // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent 
//     // works on X11. This has an tremendous effect on the performance..
// 
//     canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
// #endif
// 
//     canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
//     cRight->draw(cRight->dataSize() - size, cRight->dataSize() - 1);
//     canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);
// 
// #if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
//     canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
// #endif
    
    // Axis 
    std::time_t curTime = std::time(0); 
    setAxisTitle(QwtPlot::xBottom, "Time");
    setAxisScale(QwtPlot::xBottom, curTime, curTime+50);

    setAxisTitle(QwtPlot::yLeft, "Values");
    setAxisScale(QwtPlot::yLeft, 0, 1024);
    
//     setTimerInterval(10.0); 
}


void qui::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    canvas()->setFrameStyle(QFrame::Box | QFrame::Plain );
    canvas()->setLineWidth(1);

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(i);
        if ( scaleWidget )
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw(i);
        if ( scaleDraw )
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
}

void qui::setTimerInterval(double ms)
{
    d_interval = qRound(ms);

    if ( d_timerId >= 0 )
    {
        killTimer(d_timerId);
        d_timerId = -1;
    }
    if (d_interval >= 0 )
        d_timerId = startTimer(d_interval);
}

//  Generate new values 
void qui::timerEvent(QTimerEvent *)
{

    for(uint i=0;i<dets.size();i++){
      fftBin[i] = (double)dets[i].startBin;
      time[i] = (double)dets[i].time;
    }
//     for(auto det : dets){
//       log->info(__FILENAME__,__LINE__,"Det freq: %llu \t Det bin: %d \t Det time: %d", det.freqHz,det.startBin,det.time);
//     }
    replot();
}
