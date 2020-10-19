#include "asco_handler.hpp"

#include <QRegularExpressionMatch>
#include <QDir>
#include <QHostInfo>
#include <QDebug>
#include <QThread>

ASCO_Handler::ASCO_Handler(QObject *parent) : QObject(parent)
{
    QThread::sleep(1);
    tmr_sim_done.reset(new QTimer);
    watch_sim_updates.reset(new QFileSystemWatcher);
    watch_sim_done.reset(new QFileSystemWatcher);
    s_hostname = QHostInfo::localHostName();
    b_sim_running = false;
    b_enabled = true;
    //connect internal signals and slots
    connect(watch_sim_updates.get(), &QFileSystemWatcher::fileChanged, this, &ASCO_Handler::sl_simulationUpdate);
    connect(watch_sim_done.get(), &QFileSystemWatcher::fileChanged, this, &ASCO_Handler::sl_simulationDone);

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
        s_measurements.clear();
        s_design_variables.clear();

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
            s_design_variables.append(var.s_name);
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
            s_measurements.append(meas.s_name);
            //read then next line since these are parsed line by line
            measurement = stream.readLine();
        }
        qDebug() << "Measurements: " << new_meas.size();
        qDebug() << "Variables: " << new_vars.size();
        //now tell ui to create an appropriate number of graphs
        d_best_cost = 1e30;
        emit sg_simulationStarted(new_vars, new_meas);

        openHostnameLogFile(true);
    }
    else
    {
        qDebug() << "Failed to open netlist file to parse simulation parameters: " << asco_cfg.fileName();
    }
}

void ASCO_Handler::parseHostnameLogFile()
{

    //build a custom regex to match the exact number of goals and parameters in the asco_netlist.cfg file that was parsed earlier
    QString regex_measurement("\\s+([-|\\+]*)(.+?):(.+?):");
    QString regex_parameter("\\s*(.+?):(.+?):");
    QString regex_full = "([-|\\+])cost:(.+?):";

    for (QString s : s_measurements)
    {
        regex_full += regex_measurement;
    }
    for (QString s : s_design_variables)
    {
        regex_full += regex_parameter;
    }
    re_log.setPattern(regex_full);
    //ready to match on the latest line read from the file

    char buffer[1024];
    //assumes the the file is already open and has been seeked to EOF
    int result = f_hostname_log.readLine(buffer, sizeof(buffer));
    if (result < 0)
    {
        if (!f_hostname_log.isOpen())
        {
            qWarning() << "The log file session is no longer valid. Reloading...";
            openHostnameLogFile(true);
            qWarning() << "Reload success: " << f_hostname_log.isOpen();
        }
    }
    QString newline(buffer);

    // qDebug() << "pos: " << f_hostname_log.pos();
    if (!newline.isEmpty())
    {
        int match_index = 0;
        QVector<double> values;
        newline = newline.trimmed();
        //we have our line! now match on it
        QRegularExpressionMatch match = re_log.match(newline);
        if (match.hasMatch())
        {

            //first two are the cost and whether or not all goals are met
            QString cost = match.captured(match_index + 1);
            double cost_value = match.captured(match_index + 2).toDouble();
            emit sg_updateCost(cost_value);
            match_index += 2;
            // emit(w_cost->sg_appendDataPoint(cost_value));
            values.clear();
            for (QString &s : s_measurements)
            {
                QString good = match.captured(match_index + 1);
                QString name = match.captured(match_index + 2);
                double value = match.captured(match_index + 3).toDouble();
                match_index += 3;
                values.append(value);
            }
            //notify slots of new measurements
            emit sg_updateMeasurements(s_measurements, values);

            values.clear();
            for (QString &s : s_design_variables)
            {
                QString name = match.captured(match_index + 1);
                double value = match.captured(match_index + 2).toDouble();
                match_index += 2;
                values.append(value);
            }
            //notify slots of new design variable values
            emit sg_updateDesignVariables(s_design_variables, values);

            // parse the sim data file and emit the selected data
            bool new_best = cost_value < d_best_cost;
            if (new_best)
            {
                qDebug() << "best was: " << d_best_cost << "now is: " << cost_value;
                d_best_cost = cost_value;
            }

            parseDatFile(new_best, false);
        }
    }
}

void ASCO_Handler::parseDatFile(bool is_best, bool emit_variables)
{

    QScopedPointer<Qucs_Dat> new_file_to_parse(new Qucs_Dat);
    new_file_to_parse->Parse_File(s_dat_path);

    QMap<QString, QStringList> vars;
    for (QString indep : new_file_to_parse->getIndependentVariables())
    {
        vars[indep] = new_file_to_parse->getDependentVariables(indep);
    }
    if (emit_variables)
    {
        qDebug() << vars;
        emit sg_availableResults(vars);
    }

    //see if we have a valid combination of indep+dep variables, if we do, emit the data
    if (new_file_to_parse->exists(s_independent_variable, s_dependent_variable))
    {
        QVector<double> x_data, y_data;
        new_file_to_parse->getData(s_independent_variable, s_dependent_variable, x_data, y_data);
        emit sg_updateResult(x_data, y_data);

        if(is_best){
            emit sg_updateResultBest(x_data,y_data);
        }
    }
    else
    {
        qDebug() << "no valid data selected to emit";
    }

    if (is_best)
    {
        //update the best data so future queries will work
        o_qucs_dat_best.reset(new Qucs_Dat(*o_qucs_dat));
    }
    o_qucs_dat.swap(new_file_to_parse);
}

void ASCO_Handler::openHostnameLogFile(bool seek_to_end)
{
    //remove the old watcher file path
    watch_sim_updates->removePaths(watch_sim_updates->files());
    //close the old file
    f_hostname_log.close();
    //open the log file for parsing later, move the position to the end of the file
    f_hostname_log.setFileName(s_hostname_log_path);
    if (!f_hostname_log.open(QIODevice::ReadOnly))
    {
        qWarning() << f_hostname_log.fileName() << " cannot be opened for parsing!";
        return;
    }
    if (seek_to_end)
    {
        //seek to the end of the file
        f_hostname_log.seek(f_hostname_log.size());
    }
    //opening the file was successful, add a watch
    watch_sim_updates->addPath(s_hostname_log_path);
}

void ASCO_Handler::sl_selectDataToEmit(const QString &independent_variable, const QString &dependent_variable)
{
    if (o_qucs_dat->exists(independent_variable, dependent_variable))
    {
        s_independent_variable = independent_variable;
        s_dependent_variable = dependent_variable;
    }
}

void ASCO_Handler::sl_getResult(const QString &s_active_independent, const QString &s_active_dependent)
{
    QVector<double> x_data, y_data;
    if (o_qucs_dat->getData(s_independent_variable, s_dependent_variable, x_data, y_data))
    {
        emit sg_updateResult(x_data, y_data);
        //see if we have a best value stored
        if (!o_qucs_dat_best.isNull())
        {
            o_qucs_dat_best->getData(s_independent_variable, s_dependent_variable, x_data, y_data);
            emit sg_updateResultBest(x_data, y_data);
        }
    }
}

void ASCO_Handler::sl_setEnable(const bool &enable)
{
    b_enabled = enable;
}

void ASCO_Handler::sl_newQucsDir(const QString &path)
{
    s_qucs_dir = path;
    QDir temp(s_qucs_dir);
    s_hostname_log_path = temp.filePath(s_hostname + ".log");
    s_dat_path = temp.filePath(s_hostname + ".dat");
    s_asco_config_path = QDir(s_qucs_dir).filePath("asco_netlist.cfg");

    qDebug() << "handler got a new qucs dir: " << s_qucs_dir;
    //watch the directory for simulation starts

    watch_sim_done->removePaths(watch_sim_done->files());
    watch_sim_done->addPath(QDir(s_qucs_dir).filePath("log.txt"));

    //also watch the log file (if it exists)
    watch_sim_updates->removePaths(watch_sim_updates->files());
    watch_sim_updates->addPath(s_hostname_log_path);
}

void ASCO_Handler::sl_simulationUpdate(const QString &path)
{
    //todo reset the timer
    //read the new data
    if (b_enabled)
    {
        if (!b_sim_running)
        {
            //simulation wasnt running, now it is
            b_sim_running = true;
            //emits sg_simulationStarted with all the parameters and measurements
            parseNetlistFile();
            parseDatFile(false, true);
        }
        parseHostnameLogFile();
    }
}

void ASCO_Handler::sl_simulationDone(const QString &path)
{
    b_sim_running = false;
    emit sg_simulationDone();
    qDebug() << "simulation done";
}
