#ifndef _PLOT_H
#define _PLOT_H

#include <qwt_plot.h>
#include <qwt_interval.h>
#include <qwt_system_clock.h>
#include <QFile>
#include "curvedata.h"

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    virtual ~Plot();

    void start();
    virtual void replot();
    virtual void timerEvent(QTimerEvent *);

    void stop();

    QwtSystemClock d_clock; //shit there

public Q_SLOTS:
    void intervalChanged(int val);
    void setIntervalLength(double);
    void appendPoint(QPointF point);
        void setScaleMode();
    void onUpdateOnTime(bool);



protected:
    virtual void resizeEvent(QResizeEvent *);

private:
    void initGradient();
    void autoScaleY(CurveData *data);

    void updateCurve();
    void incrementInterval();

    QwtPlotMarker *d_origin;
    QwtPlotCurve *d_curve;
    int d_paintedPoints;

    QwtPlotDirectPainter *d_directPainter;

    QwtInterval d_interval;
    int d_timerId;

    enum XScale{Frame=0, All=1, LastPoints=2};

    //QwtScaleEngine *scaleEngineLeft;

    qreal intervalLastPoints;
    int XScaleMode;

    qreal elapsedLastPoints;
    bool updateOnTime;
};


#endif // _PLOT_H



#if 0 //old
#ifndef _PLOT_H
#define _PLOT_H

#include <qwt_plot.h>
#include <qwt_interval.h>
#include <qwt_system_clock.h>
#include <QFile>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    virtual ~Plot();

    void start();
    virtual void replot();
    virtual void timerEvent(QTimerEvent *);

public Q_SLOTS:
    void setIntervalLength(double);
    void appendPoint(QPointF point);
    double dclock_elapsed ();

protected:
    virtual void resizeEvent(QResizeEvent *);

private:
    void initGradient();
    void updateCurve();

    void incrementInterval();

    QwtPlotMarker *d_origin;
    QwtPlotCurve *d_curve;
    int d_paintedPoints;

    QwtPlotDirectPainter *d_directPainter;

    QwtInterval d_interval;
    int d_timerId;

    QwtSystemClock d_clock;
};


#endif // _PLOT_H

#endif
