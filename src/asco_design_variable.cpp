#include "asco_design_variable.hpp"

ASCO_Design_Variable::ASCO_Design_Variable(QWidget *parent) : ASCO_Parameter(parent)
{
    //maxe the max curve and assign the raw buffers
    v_max_xdata.resize(2);
    v_max_xdata[0] = 0;
    v_max_ydata.resize(2);
    curv_max = new QwtPlotCurve();
    curv_max->setTitle("Max");
    curv_max->setPen(QPen(Qt::darkRed, 1));
    curv_max->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curv_max->attach(plt_plot);
        
    //create the min curve and assign the raw buffers
    v_min_xdata.resize(2);
    v_min_xdata[0] = 0;
    v_min_ydata.resize(2);

    curv_min = new QwtPlotCurve();
    curv_min->setTitle("Min");
    curv_min->setPen(QPen(Qt::darkRed, 1));
    curv_min->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    QBrush brush(Qt::darkGreen,Qt::Dense7Pattern);
    curv_min->setBrush(brush);
    curv_min->attach(plt_plot);



}

ASCO_Design_Variable::~ASCO_Design_Variable()
{

}

void ASCO_Design_Variable::setProperties(const ASCO_Design_Variable_Properties &new_props)
{
    delete o_properties;
    ASCO_Design_Variable_Properties *properties = new ASCO_Design_Variable_Properties();
    properties->d_initial = new_props.d_initial;
    properties->d_max = new_props.d_max;
    properties->d_min = new_props.d_min;
    properties->s_interpolate = new_props.s_interpolate;
    properties->s_name = new_props.s_name;
    setTitle(new_props.s_name);
    o_properties = properties;
}



void ASCO_Design_Variable::sl_setData(const QVector<double> & independent, const QVector<double> & dependent) 
{
    //update the limit curve to keep up with the data
    ASCO_Design_Variable_Properties *properties = dynamic_cast<ASCO_Design_Variable_Properties *>(o_properties);
    v_max_ydata[0] = properties->d_max;
    v_max_ydata[1] = properties->d_max;
    v_min_ydata[0] = properties->d_min;
    v_min_ydata[1] = properties->d_min;
    //set the fill area, min to max
    curv_min->setBaseline(properties->d_max);

    //keep the second x coordinate moving with the displayed data
    //if is needed in the event that the user cleared the graph and the v_ydata is empty
    if(v_ydata.size()){
        v_max_xdata[1] = v_xdata.at(v_ydata.size()-1);
        v_min_xdata[1] = v_xdata.at(v_ydata.size()-1);
    }else{
        v_max_xdata[1] = v_max_xdata.at(0);
        v_min_xdata[1] = v_min_xdata.at(0);
    }  
    curv_max->setRawSamples(v_max_xdata.data(), v_max_ydata.data(), v_max_ydata.size());
    curv_min->setRawSamples(v_min_xdata.data(), v_min_ydata.data(), v_min_ydata.size());

    ASCO_Parameter::sl_setData(independent,dependent);
}
