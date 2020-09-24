#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QFuture>
#include <QFile>
#include <QCloseEvent>

#include "displayer.h"
#include "qwt_plot.h"
#include "asco_parameter.h"





namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef enum Parameter_Type_e{
        Design_Variable,
        Measurement
    }Parameter_Type_e;



    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool threadFileRead(const QString& s_filepath);


    void closeEvent (QCloseEvent *event);



signals:
    void sg_newQucsDir(const QString& s_dir);
    void sg_newFileLine(const QString& s_dir);
    void sg_recreateDisplayers(const QVector<ASCO_Design_Variable_Properties>& params, const QVector<ASCO_Measurement_Properties>& measurements);
    void sg_appendDataPoint(const double & data_point);


private slots:
    void on_changeQucsDirButton_clicked();
    void sl_newFileLine(const QString& s_dir);
    void sl_recreateDisplayers(const QVector<ASCO_Design_Variable_Properties>& vars, const QVector<ASCO_Measurement_Properties>& meas);


    void on_btn_Start_clicked();

    void on_btn_Stop_clicked();

    void on_le_pathDisplay_textChanged(const QString &arg1);

private:
    QString FindQucsDir();
    bool SetQucsDir(QString directory);
    void stopFileThread();



    //UI elements
    Ui::MainWindow *ui;
    ASCO_Parameter * w_cost;
    QMap<QString, ASCO_Design_Variable*> mw_asco_design_variable;
    QMap<QString, ASCO_Measurement *> mw_asco_measurement;
    


    QString hostname;
    QString qucs_dir;
    QFuture<bool> fut_started;
    QAtomicInt mi_run;

    //thread
    QFile f_asco_log;

};


#endif // MAINWINDOW_H