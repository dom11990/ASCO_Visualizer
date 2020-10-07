#include "asco_handler.hpp"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QHostInfo>
#include <QDebug>

ASCO_Handler::ASCO_Handler(QObject *parent) : QObject(parent)
{
    tmr_sim_done.reset(new QTimer);
    watch_sim_updates.reset(new QFileSystemWatcher);
    watch_sim_start.reset(new QFileSystemWatcher);
    s_hostname = QHostInfo::localHostName();
    b_sim_running = false;

    //connect internal signals and slots

    connect(watch_sim_updates.get(), &QFileSystemWatcher::fileChanged, this, &ASCO_Handler::sl_simulationUpdate);
}

ASCO_Handler::~ASCO_Handler()
{
}

void ASCO_Handler::parseNetlistFile()
{
    QFile asco_cfg(s_asco_config_path);
    if (asco_cfg.open(QIODevice::ReadOnly))
    {

        //Read out the asco_netlist.cfg file and get all the parameters and optimization goals

        QString cfg_file_contents = asco_cfg.readAll();
        asco_cfg.close();

        //extract the parameters
        QRegularExpression regex("Parameter (\\d+):#(.+?)#:(.+?):(.+?):(.+?):(.+?):(.+?)\\s");
        QRegularExpressionMatchIterator it_match = regex.globalMatch(cfg_file_contents);

        QVector<ASCO_Design_Variable_Properties> new_vars;
        qDebug() << "Extracting parameters and goals...";
        while (it_match.hasNext())
        {
            QRegularExpressionMatch match = it_match.next();
            ASCO_Design_Variable_Properties var;
            var.s_name = match.captured(2);
            var.d_initial = match.captured(3).toDouble();
            var.d_min = match.captured(4).toDouble();
            var.d_max = match.captured(5).toDouble();
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
        QString measurements(cfg_file_contents.mid(match.capturedStart(), match.capturedLength()));
        QTextStream stream(&measurements);

        regex.setPattern("(.+?):(?:.+?):(.+?):(.+)");
        QString measurement;
        //read out the # Measurement line
        stream.readLine();
        measurement = stream.readLine();
        QVector<ASCO_Measurement_Properties> new_meas;
        while (!measurement.isEmpty() && measurement.compare("#"))
        {
            match = regex.match(measurement);
            ASCO_Measurement_Properties meas;
            meas.s_name = match.captured(1);
            meas.s_compare = match.captured(2);
            meas.d_limit = match.captured(3).toDouble();
            new_meas.append(meas);
            // qDebug() << measurement;
            measurement = stream.readLine();
        }
        qDebug() << "Measurements: " << new_meas.size();
        qDebug() << "Variables: " << new_vars.size();
        //now tell ui to create an appropriate number of graphs
        emit sg_simulationStarted(new_vars, new_meas);
    }else{
        qDebug() << "Failed to open netlist file to parse simulation parameters: " << asco_cfg.fileName();
    }
}

void ASCO_Handler::newQucsDir(const QString &path)
{
    watch_sim_updates->removePaths(watch_sim_updates->files());

    //watch_sim_start->removePaths(watch_sim_updates->files());
    s_qucs_dir = path;
    QDir temp(s_qucs_dir);
    s_host_log_path = temp.filePath(s_hostname + ".log");
    s_dat_path = temp.filePath(s_hostname + ".dat");
    s_asco_config_path = QDir(s_qucs_dir).filePath("asco_netlist.cfg");

    watch_sim_updates->addPath(s_host_log_path);
    qDebug() << "handler got a new qucs dir: " << s_qucs_dir;
}

void ASCO_Handler::sl_simulationUpdate(const QString &path)
{
    //todo reset the timer
    //read the new data
    if (!b_sim_running)
    {
        //simulation wasnt running, now it is
        b_sim_running = true;
        parseNetlistFile();
    }
}
