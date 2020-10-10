#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QFuture>
#include <QFile>
#include <QCloseEvent>
#include <QScopedPointer>
#include <QRecursiveMutex>
#include <QMutex>
#include <QFileSystemWatcher>


#include "qwt_plot.h"
#include "asco_measurement.hpp"
#include "asco_design_variable.hpp"
#include "qucs_dat.hpp"
#include "asco_handler.hpp"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef enum Parameter_Type_e
    {
        Design_Variable,
        Measurement
    } Parameter_Type_e;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool threadFileRead(const QString &s_filepath);

    void closeEvent(QCloseEvent *event);

signals:
    void sg_newQucsDir(const QString &s_dir);
    void sg_newFileLine(const QString &s_dir);
    void sg_recreateDisplayers(const QVector<ASCO_Design_Variable_Properties> &params, const QVector<ASCO_Measurement_Properties> &measurements);
    void sg_appendDataPoint(const double &data_point);
    void sg_selectIndependent(const QString &indep);

    //ASCO Handler
    void sg_getResult(const QString &s_active_independent, const QString &s_active_dependent);
    void sg_selectDataToEmit(const QString &independent_variable, const QString &dependent_variable);
    void sg_setEnable(const bool &enable);

private slots:

    // UI
    void on_btn_Pause_clicked();
    void on_btn_ClearGraphs_clicked();
    void on_changeQucsDirButton_clicked();
    void on_cb_indepVariables_currentIndexChanged(int index);
    void on_cb_depVariables_currentIndexChanged(int index);
    void on_le_pathDisplay_textChanged(const QString &arg1);

    void sl_actionExit_triggered(bool checked);
    void sl_actionAbout_triggered(bool checked);
    

    // ASCO Handler
    void sl_simulationStarted(const QVector<ASCO_Design_Variable_Properties> &vars, const QVector<ASCO_Measurement_Properties> &meas);
    void sl_simulationDone();
    void sl_updateMeasurements(const QStringList &measurements, const QVector<double> &values);
    void sl_updateDesignVariables(const QStringList &design_variables, const QVector<double> &values);
    void sl_updateCost(const double &cost);
    void sl_updateResult(const QVector<double> &independent, const QVector<double> &dependent);
    void sl_availableResults(const QMap<QString, QStringList> &results);

private:
    QString FindQucsDir();
    bool SetQucsDir(QString directory);
    void stopFileThread();

    //UI elements
    Ui::MainWindow *ui;
    ASCO_Parameter *w_cost;
    QMap<QString, ASCO_Design_Variable *> mw_asco_design_variable;
    QMap<QString, ASCO_Measurement *> mw_asco_measurement;

    QString hostname;
    QString qucs_dir;
    
    
    
    QString s_active_independent;
    QString s_active_dependent;

    QMap<QString, QStringList> sim_results;

    //thread
    QScopedPointer<QThread> pt_handler;
    QScopedPointer<ASCO_Handler> p_handler;
    
};

#endif // MAINWINDOW_H
