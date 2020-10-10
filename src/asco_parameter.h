#ifndef ASCO_PARAMETER_H
#define ASCO_PARAMETER_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QVBoxLayout>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>

#include "asco_parameter_properties.hpp"
#include "asco_measurement_properties.hpp"
#include "asco_design_variable_properties.hpp"

class ASCO_Parameter : public QWidget
{
    Q_OBJECT
public:
    explicit ASCO_Parameter(QWidget *parent = nullptr);
    virtual ~ASCO_Parameter();
    void setTitle(const QString &title);

public slots:

    virtual void sl_appendDataPoint(const double &data_point);
    virtual void sl_setData(const QVector<double> &independent, const QVector<double> &dependent);
    void sl_zoomed(const QRectF &rect);

    //Data
public:
    QVector<double> v_data;
    QString s_name;
    ASCO_Parameter_Properties *o_properties;

protected:
    //UI Elements
    QLabel *lbl_label;
    QwtPlot *plt_plot;
    QVBoxLayout *lay_mainLayout;
    QwtPlotCurve *curv_data;

    QVector<double> v_ydata;
    QVector<double> v_xdata;
    QwtPlotZoomer *zoomzoom;
};

#endif // ASCO_PARAMETER_H
