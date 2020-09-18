#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QDir>

#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <QFile>
#include <QRegularExpression>

#include "qwt_plot_curve.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qucs_dir = FindQucsDir();
    ui->pathDisplay->setText(qucs_dir);
    //TODO emit here tu update the pathDisplay
    plot_buffer_size = 100;

    qRegisterMetaType<QVector<asco_parameter_t> >("QVector<asco_parameter_t>");

    connect(this, &MainWindow::sg_newQucsDir, ui->pathDisplay, &QLineEdit::setText);
    connect(this, &MainWindow::sg_newFileLine, this, &MainWindow::sl_newFileLine);
    connect(this, &MainWindow::sg_recreateDisplayers,this,&MainWindow::sl_recreateDisplayers);
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
    dialog.setFilter(QDir::AllEntries | QDir::Hidden);
    dialog.tr("Select qucs working directory");
    QString new_path;
    if(dialog.exec()){
        qDebug() << "yay";
        new_path = dialog.selectedFiles().first();
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
        return true;
    }
    return false;
}


bool MainWindow::threadFileRead(const QString& s_filepath)
{
    mi_run = 1;

    QFile asco_cfg(QDir(qucs_dir).filePath("asco_netlist.cfg"));
    if(asco_cfg.open(QIODevice::ReadOnly))
    {

        //Read out the asco_netlist.cfg file and get all the parameters and optimization goals

        QString cfg_file_contents = asco_cfg.readAll();


        //extract the parameters
        QRegularExpression regex("#\\sParameters\\s#\\n(?:.+\\n)+#");
        regex.setPattern("Parameter \\d+:#(.+?)#:(.+?:)(.+?:)(.+?:)(.+?:)(.+?)\\s");
        QRegularExpressionMatchIterator it_match = regex.globalMatch(cfg_file_contents);
//        qDebug() << match.capturedTexts();

        QVector<asco_parameter_t> new_asco_parameter;

        while(it_match.hasNext()){
            qDebug() << "Have a match:";
            QRegularExpressionMatch match = it_match.next();
            asco_parameter_t param;
            param.id = QString(match.captured(0)).toInt();
            param.name = match.captured(1);
            param.initial = atof(match.captured(2).toStdString().c_str());
            param.min = atof(match.captured(3).toStdString().c_str());
            param.max= atof(match.captured(4).toStdString().c_str());
            param.interpolate = match.captured(5);
            param.optimize = match.captured(6);
            new_asco_parameter.append(param);
        }

        //todo extract the goals


        //now tell ui to create an appropriate number of graphs
        emit sg_recreateDisplayers(new_asco_parameter);


    }else{
        qDebug() << "Failed to open asco_netlist.cfg";
        return false;
    }


    while (mi_run)
    {
        QThread::msleep(1000);
        emit sg_newFileLine(s_filepath);
    }

    return true;
}



void MainWindow::sl_recreateDisplayers(QVector<asco_parameter_t> params)
{
    v_asco_parameter = params;
    QLayoutItem *item;
    while(item = ui->scrollAreaWidgetContents->layout()->takeAt(0)){
            qDebug() << "deleting an item";
            delete item->widget();
    }

    for(asco_parameter_t param : v_asco_parameter){
        qDebug() << "creating an item";
        Displayer* o_displayer = new Displayer(this);
        ui->scrollAreaWidgetContents->layout()->addWidget(o_displayer);
        o_displayer->lbl_label->setText(param.name);
    }
}


void MainWindow::sl_newFileLine(const QString& s_dir)
{
    QVector<double> xData;
    QVector<double> yData;

//    auto curve = new QwtPlotCurve;


    //build a custom regex to match the exact number of goals and parameters in the asco_netlist.cfg file that was parsed earlier
    QString regex_goal("\\s+?([-|\\+])(.+?):(.+?):");
    QString regex_parameter("\\s*(.+?):(.+?):");
    QString full_regex_string("([-|\\+])cost:(.+?):");

    for(auto s : v_asco_goal){
        full_regex_string += regex_goal;
    }
    for(auto s : v_asco_parameter){
        full_regex_string += regex_parameter;
    }
    QRegularExpression regex(full_regex_string);
    //ready to match on the latest line read from the file

    char buffer[1024];
    QString newline;
    while(f_asco_log.readLine(buffer,1024)){
        newline = QString(buffer);
    }
    //we have our line! now match on it
    QRegularExpressionMatchIterator it_match = regex.globalMatch(newline);

    while(it_match.hasNext()){
        qDebug() << "Parsed newline:";
        QRegularExpressionMatch match = it_match.next();
        qDebug() << match.capturedLength();
        for(int i =0;i<match.capturedLength();i++){
            qDebug() << match.captured(i);
        }
    }


//    // make a plot curve from the data and attach it to the plot
//    curve->setSamples(xData, yData);
//    curve->attach(v_plots.at(0));
//    v_plots.at(0)->replot();
//    v_plots.at(0)->
    return;
}

void MainWindow::on_btn_Start_clicked()
{
    QString hostname = "debianvm";
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
    //todo replace with hostname
    f_asco_log.setFileName(QDir(qucs_dir).filePath("debianvm.log"));
    f_asco_log.open(QIODevice::ReadOnly);

    if(mi_run){
        mi_run = false;
        fut_started.waitForFinished();
        f_asco_log.close();
        ui->btn_Start->setEnabled(true);
    }
}
