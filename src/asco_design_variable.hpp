
#pragma once
#include "asco_parameter.h"

class ASCO_Design_Variable : public ASCO_Parameter
{
public:
    explicit ASCO_Design_Variable(QWidget *parent = nullptr);
    virtual ~ASCO_Design_Variable();

    void setProperties(const ASCO_Design_Variable_Properties & new_props);

public slots:
    virtual void sl_setData(const QVector<double> & independent, const QVector<double> & dependent);


protected:
        // UI
        QwtPlotCurve * curv_min;
        QwtPlotCurve * curv_max;

        //vars
        QVector<double> v_max_xdata;
        QVector<double> v_min_xdata;
        QVector<double> v_max_ydata;
        QVector<double> v_min_ydata;


};