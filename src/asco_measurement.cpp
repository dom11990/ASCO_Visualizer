#include "asco_measurement.hpp"

ASCO_Measurement::ASCO_Measurement(QWidget *parent) : ASCO_Parameter(parent)
{
    //create the data curve and assign the raw buffers
    v_limit_xdata.resize(2);
    v_limit_xdata[0] = 0;
    v_limit_ydata.resize(2);

    curv_limit = new QwtPlotCurve();
    curv_limit->setTitle("Limit");
    curv_limit->setPen(QPen(Qt::darkGreen, 2));
    curv_limit->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    QBrush brush(Qt::darkGreen, Qt::Dense7Pattern);
    curv_limit->setBrush(brush);

    curv_limit->attach(plt_plot);
}

ASCO_Measurement::~ASCO_Measurement()
{
    delete curv_limit;
}

void ASCO_Measurement::setProperties(const ASCO_Measurement_Properties &new_props)
{
    delete o_properties;
    ASCO_Measurement_Properties *properties = new ASCO_Measurement_Properties();
    properties->d_limit = new_props.d_limit;
    properties->s_compare = new_props.s_compare;
    properties->s_name = new_props.s_name;
    setTitle(new_props.s_name);

    curv_limit->setRawSamples(v_xdata.data(), v_ydata.data(), v_ydata.size());

    if (!QString::compare(properties->s_compare, "GE"))
    {
        //need to fill to pos infinity
        curv_limit->setBaseline(1e8);
        qInfo() << properties->s_name << " detected GE : " << properties->s_compare << properties->d_limit;
    }
    else if (!QString::compare(properties->s_compare, "LE"))
    {
        //need to fill to neg infinity, TODO: -1e8 seems to be the max, why??
        curv_limit->setBaseline(-1e8);
        qInfo() << properties->s_name << " detected LE : " << properties->s_compare << properties->d_limit;
    }

    o_properties = properties;
}
void ASCO_Measurement::sl_setData(const QVector<double> &independent, const QVector<double> &dependent)
{
    ASCO_Measurement_Properties *properties = dynamic_cast<ASCO_Measurement_Properties *>(o_properties);

    v_limit_ydata[0] = properties->d_limit;
    v_limit_ydata[1] = properties->d_limit;

    //keep the second x coordinate moving with the displayed data
    //if is needed in the event that the user cleared the graph and the v_ydata is empty
    if (v_ydata.size())
    {
        v_limit_xdata[1] = v_xdata.at(v_ydata.size() - 1);
    }
    else
    {
        v_limit_xdata[1] = v_limit_xdata.at(0);
    }
    curv_limit->setRawSamples(v_limit_xdata.data(), v_limit_ydata.data(), v_limit_ydata.size());

    ASCO_Parameter::sl_setData(independent, dependent);
}
