#include "plot.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qevent.h>
#include <QMutex>
#include <QRadioButton>
#include <qwt_scale_engine.h>






Plot::Plot(QWidget *parent):
    QwtPlot(parent),
    d_paintedPoints(0),
    d_interval(0.0, 10.0),
    d_timerId(-1),
    intervalLastPoints(10),
    elapsedLastPoints(0),
    updateOnTime(true),
    XScaleMode(All)
{
    d_directPainter = new QwtPlotDirectPainter();

    setAutoReplot(false);

    canvas()->setPaintAttribute(QwtPlotCanvas::BackingStore, false);

    initGradient();

    plotLayout()->setAlignCanvasToScales(true);
    //scaleEngineLeft = new QwtLinearScaleEngine();
    setAxisTitle(QwtPlot::xBottom, "Time [s]");
    setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue());
    //setAxisScale(QwtPlot::yLeft, 0, 10.0);

   // plot.axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    this->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating,true);
//    setAxisScaleEngine(QwtPlot::yLeft, scaleEngineLeft);


    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::green, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(this);

    d_origin = new QwtPlotMarker();
    d_origin->setLineStyle(QwtPlotMarker::Cross);
    d_origin->setValue(d_interval.minValue() + d_interval.width() / 2.0, 0.0);
    d_origin->setLinePen(QPen(Qt::red, 0.0, Qt::DashLine));
    d_origin->attach(this);

    d_curve = new QwtPlotCurve();
    d_curve->setStyle(QwtPlotCurve::Lines);
    d_curve->setPen(QPen(Qt::green, 2.0));

#if 0
    d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
#endif

#if 0
    d_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
#endif

    d_curve->setData(new CurveData());
    d_curve->attach(this);
    this->setAxisAutoScale(QwtPlot::yLeft, true);
    this->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating,true);
}


Plot::~Plot()
{
    delete d_directPainter;
}


void Plot::initGradient()
{
    QPalette pal = canvas()->palette();

    QLinearGradient gradient( 0.0, 0.0, 1.0, 0.0 );
    gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
    gradient.setColorAt(0.0, QColor( 0, 49, 110 ) );
    gradient.setColorAt(1.0, QColor( 0, 87, 174 ) );

    pal.setBrush(QPalette::Window, QBrush(gradient));

    canvas()->setPalette(pal);
}

void Plot::start()
{
    d_clock.start();
    d_clock.restart();
    d_timerId = startTimer(35);
}

void Plot::stop()
{
    d_clock.restart();
    d_timerId = 0;
    //d_curve->setData(QwtArray<QwtDoublePoint>());
}

void Plot::replot()
{
    CurveData *data = static_cast<CurveData *>( d_curve->data());
    QwtPlot::replot();
    d_paintedPoints = data->size();

    d_directPainter->reset();
}

void Plot::setIntervalLength(double interval)
{
    if(interval > 0.0 && interval!=d_interval.width())
    {
        d_interval.setMaxValue(d_interval.minValue() + interval);
        setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue());
        replot();
    }
}


void Plot::updateCurve()
{
    CurveData *data = (CurveData *)d_curve->data();

    int totalPoints = data->size();

    if(totalPoints > d_paintedPoints)
    {
        d_directPainter->drawSeries(d_curve, d_paintedPoints-1, totalPoints-1);
        d_paintedPoints = totalPoints;
    }
}



void Plot::appendPoint(QPointF point)
{
    CurveData *data = static_cast<CurveData *>(d_curve->data());
    data->append(point);
    int totalPoints = data->size();

    if(totalPoints > d_paintedPoints)
    {
        d_directPainter->drawSeries(d_curve, d_paintedPoints-1, totalPoints-1);
        d_paintedPoints = totalPoints;
    }

    autoScaleY(data);

    if(XScaleMode != Frame) incrementInterval();
    replot();
}

void Plot::autoScaleY(CurveData *data)
{
    /* AutoScale Y */
    float yMin=data->sample(0).y(), yMax=data->sample(0).y();
    int totalPoints = data->size();

    double k = 0;
    if(XScaleMode == Plot::LastPoints)
    {
        for(; k<totalPoints; k++)
            if(data->sample(k).x() >= (d_clock.elapsed()/1000.0-intervalLastPoints))
            {
                yMin = yMax = data->sample(k).y();
                break;
            }
    }
    if(XScaleMode == Plot::Frame)
    {
        for(; k<totalPoints; k++)
            if(data->sample(k).x() >= d_interval.minValue())
            {
                yMin = yMax = data->sample(k).y();
                break;
            }
    }

    for(double i=k; i<totalPoints; i++)
    {
        if(XScaleMode == Plot::LastPoints)
        {
            if(data->sample(i).x() >= (d_clock.elapsed()/1000.0-intervalLastPoints))
            {
                if(data->sample(i).y() < yMin) yMin = data->sample(i).y();
                if(data->sample(i).y() > yMax) yMax = data->sample(i).y();
            }
        }
        if(XScaleMode == Plot::Frame)
        {
            if(data->sample(i).x() >= d_interval.minValue())
            {
                if(data->sample(i).y() < yMin) yMin = data->sample(i).y();
                if(data->sample(i).y() > yMax) yMax = data->sample(i).y();
            }
        }
        if(XScaleMode == Plot::All)
        {
            if(data->sample(i).y() < yMin) yMin = data->sample(i).y();
            if(data->sample(i).y() > yMax) yMax = data->sample(i).y();
        }
    }
    setAxisScale(QwtPlot::yLeft, yMin, yMax);
}


void Plot::incrementInterval()
{
    //CurveData *data = static_cast<CurveData *>(d_curve->data());

    if(XScaleMode == Frame)
    {
        d_interval = QwtInterval(d_interval.maxValue(), d_interval.maxValue() + d_interval.width());
        //data->clearStaleVal(d_interval.minValue());
    }
    if(XScaleMode == All)
    {
        d_interval = QwtInterval(0, /*d_interval.maxValue() +*/ d_clock.elapsed()/1000.0);
    }
    if(XScaleMode == LastPoints)
    {
        d_interval = QwtInterval(d_clock.elapsed()/1000.0 - intervalLastPoints, d_clock.elapsed()/1000.0);
        //data->clearStaleVal(d_interval.minValue());
    }

    if(XScaleMode == Plot::Frame)
        setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue());
    if(XScaleMode == Plot::All)
        setAxisScale(QwtPlot::xBottom, 0/*d_interval.minValue()*/, d_interval.maxValue());
    if(XScaleMode == Plot::LastPoints)
        setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue());

    d_origin->setValue(d_interval.minValue()+d_interval.width()/2.0, 0.0); //red dash-dot line
    d_paintedPoints = 0;

    replot();
}



void Plot::timerEvent(QTimerEvent *event)
{
    if( event->timerId() == d_timerId)
    {
        updateCurve();

        if(XScaleMode == Plot::Frame)
        {
            if((d_clock.elapsed()/1000.0) > d_interval.maxValue())
                if(updateOnTime)incrementInterval();
        }
        if(XScaleMode == Plot::All)
        {
            static qreal elapsed2 = 0;
            if((d_clock.elapsed()/1000.0-elapsed2) > 0.1)
            {
                if(updateOnTime) incrementInterval();
                elapsed2 = d_clock.elapsed()/1000.0;
            }
        }
        if(XScaleMode == Plot::LastPoints)
        {
            if((d_clock.elapsed()/1000.0-intervalLastPoints-elapsedLastPoints) > 0.01)
            {
                if(updateOnTime) incrementInterval();
                elapsedLastPoints = d_clock.elapsed()/1000.0-intervalLastPoints;
            }
        }
        return;
    }

    QwtPlot::timerEvent(event);
}


void Plot::resizeEvent(QResizeEvent *event)
{
    d_directPainter->reset();
    QwtPlot::resizeEvent(event);
}


void Plot::setScaleMode()
{
    QRadioButton *clickedButton = qobject_cast<QRadioButton *>(sender());

    if(clickedButton->text() == "All")
    {
        XScaleMode = Plot::All;
        CurveData *data = static_cast<CurveData *>(d_curve->data());
        autoScaleY(data);
        incrementInterval();
        replot();
    }
    else if(clickedButton->text() == "Frame")
    {
        d_interval = QwtInterval(d_clock.elapsed()/1000.0 - intervalLastPoints, d_clock.elapsed()/1000.0);
        XScaleMode = Plot::Frame;
    }
    else if(clickedButton->text() == "Smooth")
        XScaleMode = Plot::LastPoints;
}

void Plot::intervalChanged(int val)
{
    intervalLastPoints = val;

    if(XScaleMode == Plot::Frame)
        d_interval = QwtInterval(d_clock.elapsed()/1000.0 - intervalLastPoints, d_clock.elapsed()/1000.0);
    if(XScaleMode == Plot::LastPoints)
    {
        incrementInterval();
        elapsedLastPoints = d_clock.elapsed()/1000.0-intervalLastPoints;
    }
}

void Plot::onUpdateOnTime(bool state)
{
    updateOnTime = state;
}






#if 0 //OLD
#include "plot.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qevent.h>
#include <QMutex>

#include "curvedata.h"


Plot::Plot(QWidget *parent):
    QwtPlot(parent),
    d_paintedPoints(0),
    d_interval(0.0, 50.0),
    d_timerId(-1)
{

    d_directPainter = new QwtPlotDirectPainter();

    setAutoReplot(false);

    // The backing store is important, when working with widget
    // overlays ( f.e rubberbands for zooming ).
    // Here we don't have them and the internal
    // backing store of QWidget is good enough.

    canvas()->setPaintAttribute(QwtPlotCanvas::BackingStore, false);

    initGradient();

    plotLayout()->setAlignCanvasToScales(true);

    setAxisTitle(QwtPlot::xBottom, "Time [s]");
    setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue());
    setAxisScale(QwtPlot::yLeft, 0, 10.0);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::green, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(this);

    d_origin = new QwtPlotMarker();
    d_origin->setLineStyle(QwtPlotMarker::Cross);
    d_origin->setValue(d_interval.minValue() + d_interval.width() / 2.0, 0.0);
    d_origin->setLinePen(QPen(Qt::red, 0.0, Qt::DashLine));
    d_origin->attach(this);


    d_curve = new QwtPlotCurve();
    d_curve->setStyle(QwtPlotCurve::Lines);
    d_curve->setPen(QPen(Qt::green,3));
#if 1
    d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
#endif
#if 1
    d_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
#endif
    d_curve->setData(new CurveData());
    d_curve->attach(this);
}


Plot::~Plot()
{
    delete d_directPainter;
}


void Plot::initGradient()
{
    QPalette pal = canvas()->palette();

#if QT_VERSION >= 0x040400
    QLinearGradient gradient( 0.0, 0.0, 1.0, 0.0 );
    gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
    gradient.setColorAt(0.0, QColor( 0, 49, 110 ) );
    gradient.setColorAt(1.0, QColor( 0, 87, 174 ) );

    pal.setBrush(QPalette::Window, QBrush(gradient));
#else
    pal.setBrush(QPalette::Window, QBrush( color ));
#endif

    canvas()->setPalette(pal);
}

void Plot::start()
{
    d_clock.start();
    d_timerId = startTimer(10);
}

void Plot::replot()
{
    CurveData *data = static_cast<CurveData *>( d_curve->data());
    QwtPlot::replot();
    d_paintedPoints = data->size();
}

void Plot::setIntervalLength(double interval)
{
    if ( interval > 0.0 && interval != d_interval.width() )
    {
        d_interval.setMaxValue(d_interval.minValue() + interval);
        setAxisScale(QwtPlot::xBottom,
            d_interval.minValue(), d_interval.maxValue());

        replot();
    }
}

void Plot::updateCurve()
{
    CurveData *data = (CurveData *)d_curve->data();
    const int numPoints = data->size();
    /*
                Depending on the platform setting a clip might be an important
                performance issue. F.e. for Qt Embedded this reduces the
                part of the backing store that has to be copied out - maybe
                to an unaccelerated frame buffer device.
            */
    if (numPoints > d_paintedPoints)
    {
        const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
        if ( doClip )
        {
            /*
                Depending on the platform setting a clip might be an important
                performance issue. F.e. for Qt Embedded this reduces the
                part of the backing store that has to be copied out - maybe
                to an unaccelerated frame buffer device.
            */

            const QwtScaleMap xMap = canvasMap( d_curve->xAxis() );
            const QwtScaleMap yMap = canvasMap( d_curve->yAxis() );

            QRectF br = qwtBoundingRect( *data,
                d_paintedPoints - 1, numPoints - 1 );

            const QRect clipRect = QwtScaleMap::transform(xMap, yMap, br).toRect();
            d_directPainter->setClipRegion(clipRect);
        }

        d_directPainter->drawSeries(d_curve,
            d_paintedPoints - 1, numPoints - 1);
        d_paintedPoints = numPoints;
    }

}

void Plot::appendPoint(QPointF point)
{

    CurveData *data = static_cast<CurveData *>(d_curve->data());
    data->append(point);
    const int numPoints = data->size();
    if ( numPoints > d_paintedPoints )
    {
        const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
        if ( doClip )
        {
            /*
                Depending on the platform setting a clip might be an important
                performance issue. F.e. for Qt Embedded this reduces the
                part of the backing store that has to be copied out - maybe
                to an unaccelerated frame buffer device.
            */

            const QwtScaleMap xMap = canvasMap( d_curve->xAxis() );
            const QwtScaleMap yMap = canvasMap( d_curve->yAxis() );

            QRectF br = qwtBoundingRect( *data,
                d_paintedPoints - 1, numPoints - 1 );

            const QRect clipRect = QwtScaleMap::transform(xMap, yMap, br).toRect();
            d_directPainter->setClipRegion(clipRect);
        }

        d_directPainter->drawSeries(d_curve,
            d_paintedPoints - 1, numPoints - 1);
        d_paintedPoints = numPoints;
    }
    replot();
}

double Plot::dclock_elapsed()
{
    return d_clock.elapsed()/1000;
}

void Plot::incrementInterval()
{

    d_interval = QwtInterval(d_interval.maxValue(),
        d_interval.maxValue() + d_interval.width());

    CurveData *data = static_cast<CurveData *>( d_curve->data() );
    data->clearStaleVal(d_interval.minValue());
    // To avoid, that the grid is jumping, we disable
    // the autocalculation of the ticks and shift them
    // manually instead.

    QwtScaleDiv scaleDiv = *axisScaleDiv(QwtPlot::xBottom);
    scaleDiv.setInterval(d_interval);

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        QList<double> ticks = scaleDiv.ticks(i);
        for ( int j = 0; j < ticks.size(); j++ )
            ticks[j] += d_interval.width();
        scaleDiv.setTicks(i, ticks);
    }
    setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);
    d_origin->setValue(d_interval.minValue() + d_interval.width() / 2.0, 0.0);
    d_paintedPoints = 0;

    replot();
}

void Plot::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() == d_timerId )
    {
        updateCurve();

        const double elapsed = d_clock.elapsed() / 1000.0;
        if ( elapsed > d_interval.maxValue() )
            incrementInterval();

        return;
    }

    QwtPlot::timerEvent(event);
}

void Plot::resizeEvent(QResizeEvent *event)
{
    d_directPainter->reset();
    QwtPlot::resizeEvent(event);
}
#endif
