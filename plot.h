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

