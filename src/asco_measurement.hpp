
#pragma once
#include "asco_parameter.h"

class ASCO_Measurement : public ASCO_Parameter
{
    public:
        explicit ASCO_Measurement(QWidget *parent = nullptr);
        virtual ~ASCO_Measurement();

        void setProperties(const ASCO_Measurement_Properties& new_props);

    public slots:
        virtual void sl_setData(const QVector<double> & independent, const QVector<double> & dependent);
    

    protected:
        QwtPlotCurve * curv_limit;
        QVector<double> v_limit_ydata;
        QVector<double> v_limit_xdata; 

};

