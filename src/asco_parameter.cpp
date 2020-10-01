#include "asco_parameter.h"


ASCO_Parameter::ASCO_Parameter(QWidget *parent)
{

    o_properties = new ASCO_Parameter_Properties();

    //set the layout
    lay_mainLayout = new QVBoxLayout();
    setLayout(lay_mainLayout);
    
    //create the label and align it
    lbl_label = new QLabel(tr(""), this);
    lbl_label->setAlignment(Qt::AlignCenter);
    lay_mainLayout->addWidget(lbl_label);    
    
    //create the plot
    plt_plot = new QwtPlot(this);
    zoomzoom = new QwtPlotZoomer( plt_plot->canvas() ); 
    plt_plot->replot();
    plt_plot->axisAutoScale(0);
    plt_plot->axisAutoScale(1);
    lay_mainLayout->addWidget(plt_plot);
    

    //initialize the buffers
    int buffer_size = 100000;
    v_xdata.reserve(buffer_size);
    for(int i = 0; i<buffer_size;i++)
        v_xdata.push_back(i);

    v_ydata.reserve(buffer_size);

    //create the curves and assign the raw buffers
    curv_data = new QwtPlotCurve();
    curv_data->setTitle( "Data" );
    curv_data->setPen( QPen( Qt::white, 2 ) ),
    curv_data->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    curv_data->setRawSamples(v_xdata.data(),v_ydata.data(),v_ydata.size());
    curv_data->attach(plt_plot);


    //make the widget retain the space even when it is hidden
    QSizePolicy sp_retain = sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    setSizePolicy(sp_retain);


//connect signals and slots
    connect(zoomzoom, SIGNAL(zoomed(const QRectF&)),this,SLOT(sl_zoomed (const QRectF &)));

    connect(this,&ASCO_Parameter::sg_setData, this, &ASCO_Parameter::sl_setData);
    connect(this,&ASCO_Parameter::sg_appendDataPoint, this, &ASCO_Parameter::sl_appendDataPoint);

}

void ASCO_Parameter::setTitle(const QString& title) 
{
    lbl_label->setText(title);
}

    void ASCO_Parameter::sl_appendDataPoint(const double & data_point) 
{
    v_ydata.append(data_point);
    curv_data->setRawSamples(v_xdata.data(),v_ydata.data(),v_ydata.size());
    plt_plot->replot();
    plt_plot->axisAutoScale(QwtPlot::xBottom);
    plt_plot->axisAutoScale(QwtPlot::yLeft);
}



void ASCO_Parameter::sl_setData(const QVector<double> & independent, const QVector<double> & dependent) 
{
    v_xdata = independent;
    v_ydata = dependent;
    curv_data->setRawSamples(v_xdata.data(),v_ydata.data(),v_ydata.size());
    plt_plot->replot();
    plt_plot->axisAutoScale(QwtPlot::xBottom);
    plt_plot->axisAutoScale(QwtPlot::yLeft);
}

void ASCO_Parameter::sl_zoomed(const QRectF &rect) 
{
    if(!zoomzoom->zoomRectIndex()){
            plt_plot->setAxisAutoScale(QwtPlot::xBottom,true);
            plt_plot->setAxisAutoScale(QwtPlot::yLeft,true);
            zoomzoom->setZoomBase(true);
    }
}




ASCO_Measurement::ASCO_Measurement(QWidget *parent) : ASCO_Parameter(parent)
{

}

void ASCO_Measurement::setProperties(const ASCO_Measurement_Properties& new_props) 
{
    delete o_properties;
    ASCO_Measurement_Properties * properties = new ASCO_Measurement_Properties();
    properties->d_limit = new_props.d_limit;
    properties->s_compare = new_props.s_compare;  
    properties->s_name = new_props.s_name;
    setTitle(new_props.s_name);
    o_properties = properties;
}


ASCO_Design_Variable::ASCO_Design_Variable(QWidget *parent) : ASCO_Parameter(parent)
{

}

void ASCO_Design_Variable::setProperties(const ASCO_Design_Variable_Properties & new_props) 
{
    delete o_properties;
    ASCO_Design_Variable_Properties * properties = new ASCO_Design_Variable_Properties();
    properties->d_initial = new_props.d_initial;
    properties->d_max = new_props.d_max;
    properties->d_min = new_props.d_min;
    properties->s_interpolate = new_props.s_interpolate;
    properties->s_name = new_props.s_name;
    setTitle(new_props.s_name);
    o_properties = properties;
}
