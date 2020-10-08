#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QHostInfo>
#include <QDir>

#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <QFile>
#include <QRegularExpression>

#include "qwt_plot_curve.h"
#include "qwt_plot.h"
#include "asco_design_variable_properties.hpp"
#include "asco_measurement_properties.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{

    //initialize variables
    o_qucs_dat.reset(new Qucs_Dat());

    //initialize UI
    ui->setupUi(this);

    pt_handler.reset(new QThread);
    p_handler.reset(new ASCO_Handler);
    qDebug() << "before" << p_handler.get();
    p_handler->moveToThread(pt_handler.get());
    qDebug() << "after" << p_handler.get();
    connect(this, &MainWindow::sg_newQucsDir, p_handler.get(), &ASCO_Handler::sl_newQucsDir);
    connect(this, &MainWindow::sg_getResult, p_handler.get(), &ASCO_Handler::sl_getResult);
    connect(this, &MainWindow::sg_selectDataToEmit, p_handler.get(), &ASCO_Handler::sl_selectDataToEmit);
    connect(this, &MainWindow::sg_setEnable, p_handler.get(), &ASCO_Handler::sl_setEnable);

    connect(p_handler.get(), &ASCO_Handler::sg_simulationStarted, this, &MainWindow::sl_recreateDisplayers);
    connect(p_handler.get(), &ASCO_Handler::sg_availableResults, this, &MainWindow::sl_availableResults);

    connect(p_handler.get(), &ASCO_Handler::sg_updateCost, this, &MainWindow::sl_updateCost);
    connect(p_handler.get(), &ASCO_Handler::sg_updateDesignVariables, this, &MainWindow::sl_updateDesignVariables);
    connect(p_handler.get(), &ASCO_Handler::sg_updateMeasurements, this, &MainWindow::sl_updateMeasurements);
    connect(p_handler.get(), &ASCO_Handler::sg_updateResult, this, &MainWindow::sl_updateResult);
    
    

    pt_handler->start();
    qDebug() << "thread running:" << pt_handler->isRunning();

    // ui->w_sim_display->hide();

    qucs_dir = FindQucsDir();
    ui->le_pathDisplay->setText(qucs_dir);
    //TODO emit here tu update the le_pathDisplay

    //update default values
    hostname = QHostInfo::localHostName();

    qRegisterMetaType<QVector<ASCO_Design_Variable_Properties>>("QVector<ASCO_Design_Variable_Properties>");
    qRegisterMetaType<QVector<ASCO_Measurement_Properties>>("QVector<ASCO_Measurement_Properties>");
    qRegisterMetaType<QMap<QString,QStringList>>("QMap<QString,QStringList>");

    // connect(this, &MainWindow::sg_recreateDisplayers, this, &MainWindow::sl_recreateDisplayers);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::sl_actionExit_triggered);
    //connect(this, &MainWindow::sg_newIndependentVariables, this, &MainWindow::sl_newIndependentVariables);

    // connect(this,&MainWindow::sg_newPlotPoints,this,&MainWindow::sl_newPlotPoints);
}

MainWindow::~MainWindow()
{
    mi_run = 0;
    pt_handler->quit();
    qDebug() << "Waiting for asco handler to stop...";
    pt_handler->wait();
    // delete ui;
}

void MainWindow::sl_actionExit_triggered(bool checked)
{
    qDebug("Quitting");
    exit(0);
}

void MainWindow::sl_actionAbout_triggered(bool checked)
{
    qDebug("Clicked about");
}

void MainWindow::on_changeQucsDirButton_clicked()
{
    QFileDialog dialog;
    dialog.setFilter(QDir::AllDirs | QDir::Hidden);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.tr("Select qucs working directory");
    QString new_path;
    if (dialog.exec())
    {
        qDebug() << "yay";
        new_path = dialog.selectedFiles().first();
        ui->le_pathDisplay->setText(new_path);
    }

    qDebug() << "Selected path: " << new_path;
    qDebug() << "Attempting to set new path" << new_path;
}

QString MainWindow::FindQucsDir()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QDir home_dir(paths.first());
    if (home_dir.exists(".qucs"))
    {
        home_dir.cd(".qucs");
    }
    return home_dir.absolutePath();
}

bool MainWindow::SetQucsDir(QString directory)
{
    QDir new_path(directory);
    if (new_path.exists() && !directory.isEmpty())
    {
        qucs_dir = new_path.absolutePath();
        emit sg_newQucsDir(qucs_dir);
        qDebug() << qucs_dir << " appears to be a valid qucs dir";
        return true;
    }
    return false;
}


void MainWindow::closeEvent(QCloseEvent *event)
{


    event->accept();
}

void MainWindow::sl_recreateDisplayers(const QVector<ASCO_Design_Variable_Properties> &vars, const QVector<ASCO_Measurement_Properties> &meas)
{
    // delete all current widgets
    QLayoutItem *item;
    do
    {
        item = ui->scrollAreaWidgetContents->layout()->takeAt(0);
        if (item)
        {
            delete item->widget();
        }
    } while (item);

    //create the cost widget
    w_cost.reset(new ASCO_Parameter(this));
    // connect(w_cost, &ASCO_Parameter::sg_appendDataPoint, w_cost, &ASCO_Parameter::sl_appendDataPoint);
    ui->scrollAreaWidgetContents->layout()->addWidget(w_cost.get());
    w_cost->setTitle(QString("cost function"));

    // create new asco parameter widgets and connect the slots
    for (ASCO_Design_Variable_Properties design_var : vars)
    {
        qDebug() << "Creating Design Variable: " << design_var.s_name;
        ASCO_Design_Variable *new_var = new ASCO_Design_Variable(this);
        // connect(new_var, &ASCO_Parameter::sg_appendDataPoint, new_var, &ASCO_Parameter::sl_appendDataPoint);
        ui->scrollAreaWidgetContents->layout()->addWidget(new_var);
        new_var->setProperties(design_var);
        mw_asco_design_variable[design_var.s_name] = new_var;
    }

    for (ASCO_Measurement_Properties measurement : meas)
    {
        qDebug() << "Creating Measurement: " << measurement.s_name;
        ASCO_Measurement *new_var = new ASCO_Measurement(this);
        // connect(new_var, &ASCO_Parameter::sg_appendDataPoint, new_var, &ASCO_Parameter::sl_appendDataPoint);
        ui->scrollAreaWidgetContents->layout()->addWidget(new_var);
        new_var->setProperties(measurement);
        mw_asco_measurement[measurement.s_name] = new_var;
    }
    mi_displays_ready = 1;
}

void MainWindow::on_cb_indepVariables_currentIndexChanged(int index)
{
    s_active_independent = ui->cb_indepVariables->currentText();
    qDebug() << "active independent variable " << s_active_independent;
    ui->cb_depVariables->clear();
    ui->cb_depVariables->addItems(sim_results[s_active_independent]);
}

void MainWindow::on_cb_depVariables_currentIndexChanged(int index)
{
    s_active_dependent = ui->cb_depVariables->currentText();
    qDebug() << "active dependent variable " << s_active_dependent;
    emit sg_selectDataToEmit(s_active_independent, s_active_dependent);
    emit sg_getResult(s_active_independent,s_active_dependent);
}

void MainWindow::on_btn_Pause_clicked()
{
    //TODO
    if(ui->btn_Pause->text() == "Pause")
    {
        ui->btn_Pause->setText("Resume");
        emit(sg_setEnable(false));
    }else{
        ui->btn_Pause->setText("Pause");
        emit(sg_setEnable(true));
    }
    
    
}

void MainWindow::on_btn_ClearGraphs_clicked()
{
    //empty all the plots
    QVector<double> x,y;
    if(w_cost){
        w_cost->sg_setData(x,y);
    }
    
    for(auto s : mw_asco_design_variable){
        emit s->sg_setData(x,y);
    }
    for(auto s : mw_asco_measurement){
        emit s->sg_setData(x,y);
    }


}

void MainWindow::on_le_pathDisplay_textChanged(const QString &arg1)
{
    SetQucsDir(arg1);
}

void MainWindow::sl_updateMeasurements(const QStringList &measurements, const QVector<double> &values)
{
    for (int i = 0; i < measurements.size(); i++)
    {
        emit(mw_asco_measurement[measurements.at(i)]->sg_appendDataPoint(values.at(i)));
    }
}
void MainWindow::sl_updateDesignVariables(const QStringList &design_variables, const QVector<double> &values)
{
    for (int i = 0; i < design_variables.size(); i++)
    {
        emit(mw_asco_design_variable[design_variables.at(i)]->sg_appendDataPoint(values.at(i)));
    }
}

void MainWindow::sl_updateCost(const double &cost)
{
    emit(w_cost->sg_appendDataPoint(cost));
}

void MainWindow::sl_updateResult(const QVector<double> &independent, const QVector<double> &dependent)
{
    ui->w_sim_display->sg_setData(independent, dependent);
}

void MainWindow::sl_availableResults(const QMap<QString, QStringList> &results)
{
    sim_results = results;
    ui->cb_indepVariables->clear();
    ui->cb_indepVariables->addItems(sim_results.keys());
    ui->cb_indepVariables->setCurrentText(s_active_independent);
}
