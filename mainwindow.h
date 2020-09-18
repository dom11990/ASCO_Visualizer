#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFuture>
#include <QFile>

#include "displayer.h"
#include "qwt_plot.h"






namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool threadFileRead(const QString& s_filepath);

    typedef struct{
        int id;
        QString name;
        double initial;
        double min;
        double max;
        QString interpolate;
        QString optimize;
    }asco_parameter_t;

    typedef struct{
        //in_band_gain:---:GE:-5
        int id;
        QString name;
        QString compare;
        double limit;
    }asco_goal_t;


signals:
    void sg_newQucsDir(const QString& s_dir);
    void sg_newFileLine(const QString& s_dir);
    void sg_recreateDisplayers(QVector<asco_parameter_t> params);


private slots:
    void on_changeQucsDirButton_clicked();
    void sl_newFileLine(const QString& s_dir);
    void sl_recreateDisplayers(QVector<asco_parameter_t> params);
    void on_btn_Start_clicked();

    void on_btn_Stop_clicked();

private:
    QString FindQucsDir();
    bool SetQucsDir(QString directory);



    //members
    Ui::MainWindow *ui;
    QVector<Displayer *> o_displayers;
    QVector<asco_parameter_t> v_asco_parameter;
    QVector<asco_goal_t> v_asco_goal;
    QVector<QwtPlot *> v_plots;
    QVector<QVector<double>> vv_ydata;
    QVector<double> v_xdata;
    int plot_buffer_size;




    QString qucs_dir;
    QFuture<bool> fut_started;
    QAtomicInt mi_run;

    //thread
    QFile f_asco_log;

};


#endif // MAINWINDOW_H
