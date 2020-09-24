#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
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


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qucs_dir = FindQucsDir();
    ui->le_pathDisplay->setText(qucs_dir);
    //TODO emit here tu update the le_pathDisplay

    //update default values
    hostname = QHostInfo::localHostName();

    qRegisterMetaType<QVector<ASCO_Design_Variable_Properties> >("QVector<ASCO_Design_Variable_Properties>");
    qRegisterMetaType<QVector<ASCO_Measurement_Properties> >("QVector<ASCO_Measurement_Properties>");

    connect(this, &MainWindow::sg_newQucsDir, ui->le_pathDisplay, &QLineEdit::setText);
    connect(this, &MainWindow::sg_newFileLine, this, &MainWindow::sl_newFileLine);
    connect(this, &MainWindow::sg_recreateDisplayers,this,&MainWindow::sl_recreateDisplayers);
    // connect(this,&MainWindow::sg_newPlotPoints,this,&MainWindow::sl_newPlotPoints);
}

MainWindow::~MainWindow()
{
    mi_run = 0;
    delete ui;
}

void MainWindow::on_changeQucsDirButton_clicked()
{

    qDebug() << "C++ Style Info Message";
    qDebug( "C Style Info Message" );
    QFileDialog dialog;
    dialog.setFilter(QDir::AllDirs | QDir::Hidden);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.tr("Select qucs working directory");
    QString new_path;
    if(dialog.exec()){
        qDebug() << "yay";
        new_path = dialog.selectedFiles().first();
        ui->le_pathDisplay->setText(new_path);
    }


    qDebug() << "Selected path: " << new_path;
    qDebug() << "Attempting to set new path" << new_path;
    //SetQucsDir(new_path);

}

QString MainWindow::FindQucsDir(){
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QDir home_dir(paths.first());
     qDebug() << "exists: " << home_dir.exists(".qucs");
    if(home_dir.exists(".qucs")){
        home_dir.cd(".qucs");
    }
    return home_dir.absolutePath();
}

bool MainWindow::SetQucsDir(QString directory){
    QDir new_path(directory);
    if(new_path.exists() && !directory.isEmpty()){
        qucs_dir = new_path.absolutePath();
        emit sg_newQucsDir(qucs_dir);
        qDebug() << "appears to be a valid qucs dir";
        return true;
    }
    return false;
}

void MainWindow::stopFileThread() 
{
        if(mi_run){
        mi_run = false;
        fut_started.waitForFinished();
        f_asco_log.close();
        ui->btn_Start->setEnabled(true);
    }
}


bool MainWindow::threadFileRead(const QString& s_filepath)
{
    Q_UNUSED(s_filepath);

    mi_run = 1;

    QFile asco_cfg(QDir(qucs_dir).filePath("asco_netlist.cfg"));
    if(asco_cfg.open(QIODevice::ReadOnly))
    {

        //Read out the asco_netlist.cfg file and get all the parameters and optimization goals

        QString cfg_file_contents = asco_cfg.readAll();


        //extract the parameters
        QRegularExpression regex("Parameter (\\d+):#(.+?)#:(.+?):(.+?):(.+?):(.+?):(.+?)\\s");
        QRegularExpressionMatchIterator it_match = regex.globalMatch(cfg_file_contents);

        QVector<ASCO_Design_Variable_Properties> new_vars;
        qDebug() << "Extracting parameters and goals...";
        while(it_match.hasNext()){
            QRegularExpressionMatch match = it_match.next();
            ASCO_Design_Variable_Properties var;
            var.s_name = match.captured(2);
            var.d_initial = match.captured(3).toDouble();
            var.d_min = match.captured(4).toDouble();
            var.d_max= match.captured(5).toDouble();
            var.s_interpolate = match.captured(6);
            var.s_optimize = match.captured(7);
            new_vars.append(var);
        }


        //extract the measurements
        //# Measurements #
        //in_band_s11:---:LE:-10
        //LO_suppresion:---:LE:-40
        //low_band_supp:---:LE:-15
        //in_band_gain:---:GE:-5
        //#

        regex.setPattern("# +Measurements +#\\n([\\s\\S])+?#");
        QRegularExpressionMatch match = regex.match(cfg_file_contents);
        QString measurements(cfg_file_contents.mid(match.capturedStart(),match.capturedLength()));
        QTextStream stream(&measurements);



        regex.setPattern("(.+?):(?:.+?):(.+?):(.+)");
        QString measurement;
        //read out the # Measurement line
        stream.readLine();
        measurement = stream.readLine();
        QVector<ASCO_Measurement_Properties> new_meas;
        while(!measurement.isEmpty() && measurement.compare("#")){
            match = regex.match(measurement);
            ASCO_Measurement_Properties meas;
            meas.s_name = match.captured(1);
            meas.s_compare = match.captured(2);
            meas.d_limit = match.captured(3).toDouble();
            new_meas.append(meas);
            measurement = stream.readLine();
        }




        //now tell ui to create an appropriate number of graphs
        emit sg_recreateDisplayers(new_vars, new_meas);


    }else{
        qDebug() << "Failed to open asco_netlist.cfg";
        return false;
    }


    //open the log file and move to the end of the file since
    //sicne it does not get automatically cleared between runs
    f_asco_log.setFileName(QDir(qucs_dir).filePath(hostname + ".log"));
    qDebug() << "Opening log file: " << f_asco_log.open(QIODevice::ReadOnly);
    qDebug() << "File pos:" << f_asco_log.pos();
    // f_asco_log.seek(f_asco_log.size());
    qDebug() << "File pos:" << f_asco_log.pos();

    while (mi_run)
    {
        QThread::msleep(250);

    //    auto curve = new QwtPlotCurve;


        //build a custom regex to match the exact number of goals and parameters in the asco_netlist.cfg file that was parsed earlier
        QString regex_measurement("\\s+?([-|\\+])(.+?):(.+?):");
        QString regex_parameter("\\s*(.+?):(.+?):");
        QString full_regex_string("([-|\\+])cost:(.+?):");

        for(auto s : mw_asco_measurement){
            full_regex_string += regex_measurement;
        }
        for(auto s : mw_asco_design_variable){
            full_regex_string += regex_parameter;
        }
        QRegularExpression regex(full_regex_string);
        //ready to match on the latest line read from the file


        char buffer[1024];

        QString newline("");
        int linelength;
        do{
            linelength = f_asco_log.readLine(buffer,sizeof(buffer));
            if(linelength > 0)
                newline = QString(buffer);
        }while(linelength > 0);

        // qDebug() << "pos: " << f_asco_log.pos();
        if(!newline.isEmpty()){
            int match_index = 0;
            newline = newline.trimmed();
            //we have our line! now match on it
            QRegularExpressionMatch match = regex.match(newline);
            

            // QStringList list =  match.capturedTexts();
            // int l = 0;
            // qDebug() << newline;
            // qDebug() << "match size " << match.lastCapturedIndex();

            // for(QString s : list){
            //     qDebug() << "match " << l << ":" << s;
            //     l++;
            // }
                
            //first two are the cost and whether or not all goals are met
            QString cost = match.captured(match_index+1);
            double cost_value = match.captured(match_index+2).toDouble();
            match_index += 2;
            emit(w_cost->sg_appendDataPoint(cost_value));

            for (auto s : mw_asco_measurement){
                QString good = match.captured(match_index+1);
                QString name = match.captured(match_index+2);
                double value = match.captured(match_index+3).toDouble();
                match_index += 3;
                mw_asco_measurement[name]->sg_appendDataPoint(value);
            }

            for (auto &s : mw_asco_design_variable){
                QString name = match.captured(match_index+1);
                double value = match.captured(match_index+2).toDouble();
                match_index += 2;
                mw_asco_design_variable[name]->sg_appendDataPoint(value);
            }
        }




    }

    return true;
}


void MainWindow::closeEvent (QCloseEvent *event)
{

    stopFileThread();
    event->accept();
}


void MainWindow::sl_recreateDisplayers(const QVector<ASCO_Design_Variable_Properties>& vars, const QVector<ASCO_Measurement_Properties>& meas)
{
    // delete all current widgets
    QLayoutItem *item;
    do{
        item = ui->scrollAreaWidgetContents->layout()->takeAt(0);
        if(item){
            delete item->widget();
    }
    }while(item);

    //create the cost widget
        w_cost = new ASCO_Parameter(this);
        connect(w_cost,&ASCO_Parameter::sg_appendDataPoint,w_cost,&ASCO_Parameter::sl_appendDataPoint);
        ui->scrollAreaWidgetContents->layout()->addWidget(w_cost);
        w_cost->setTitle(QString("cost function"));
        


    // create new asco parameter widgets and connect the slots
    for(ASCO_Design_Variable_Properties design_var : vars){
        ASCO_Design_Variable * new_var = new ASCO_Design_Variable(this);
        connect(new_var,&ASCO_Parameter::sg_appendDataPoint,new_var,&ASCO_Parameter::sl_appendDataPoint);
        ui->scrollAreaWidgetContents->layout()->addWidget(new_var);
        new_var->setProperties(design_var);
        mw_asco_design_variable[design_var.s_name] = new_var;
    }

    for(ASCO_Measurement_Properties measurement : meas){
        ASCO_Measurement * new_var = new ASCO_Measurement(this);
        connect(new_var,&ASCO_Parameter::sg_appendDataPoint,new_var,&ASCO_Parameter::sl_appendDataPoint);
        ui->scrollAreaWidgetContents->layout()->addWidget(new_var);
        new_var->setProperties(measurement);
        mw_asco_measurement[measurement.s_name] = new_var;
    }
}


void MainWindow::sl_newFileLine(const QString& s_dir)
{
    Q_UNUSED(s_dir);

    return;
}

void MainWindow::on_btn_Start_clicked()
{
    QDir dir(qucs_dir);
    if(!dir.exists(hostname + ".log"))
    {
        qDebug() << hostname+".log does not exist!";
        return;
    }

    if(!mi_run){
        ui->btn_Start->setEnabled(false);
        fut_started = QtConcurrent::run(this, &MainWindow::threadFileRead, QString());
    }
}

void MainWindow::on_btn_Stop_clicked()
{
    stopFileThread();
}

void MainWindow::on_le_pathDisplay_textChanged(const QString &arg1)
{
    qDebug() <<"slot on_le_pathDisplay_textChanged";
    SetQucsDir(arg1);

}