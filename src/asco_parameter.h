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
    void setTitle(const QString& title);




signals:
    void sg_appendDataPoint(const double & data_point);
    void sg_setData(const QVector<double> & independent, const QVector<double> & dependent);

public slots:

    void sl_appendDataPoint(const double & data_point);
    void sl_setData(const QVector<double> & independent, const QVector<double> & dependent);
    void sl_zoomed (const QRectF &rect);

    

//Data
public:
    QVector<double> v_data;
    QString s_name;
    ASCO_Parameter_Properties * o_properties;

private:
    //UI Elements
    QLabel* lbl_label;
    QwtPlot* plt_plot;
    QVBoxLayout *lay_mainLayout;
    QwtPlotCurve * curv_data;
    QVector<double> v_ydata;
    QVector<double> v_xdata;   
    QwtPlotZoomer * zoomzoom;
};


class ASCO_Measurement : public ASCO_Parameter
{
    public:
        explicit ASCO_Measurement(QWidget *parent = nullptr);
        void setProperties(const ASCO_Measurement_Properties& new_props);

    private:
        QwtPlotCurve * curv_limit;

};



class ASCO_Design_Variable : public ASCO_Parameter
{
public:
    explicit ASCO_Design_Variable(QWidget *parent = nullptr);
    void setProperties(const ASCO_Design_Variable_Properties & new_props);

private:
        QwtPlotCurve * curv_min;
        QwtPlotCurve * curv_max;
        QwtPlotCurve * curv_initial;
};



#endif // ASCO_PARAMETER_H
