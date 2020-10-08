
#pragma once


#include "asco_design_variable_properties.hpp"
#include "asco_measurement_properties.hpp"
#include "qucs_dat.hpp"

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QFile>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMutex>
#include <QRegularExpression>



class ASCO_Handler : public QObject
{

	Q_OBJECT

public:
	ASCO_Handler(QObject *parent = 0);
	virtual ~ASCO_Handler();

private:
	void parseNetlistFile();
	void parseHostnameLogFile();
	void parseDatFile(bool emit_variables = false);
	void openHostnameLogFile(bool seek_to_end = false);


signals:
	//emitted on simulation start to notify the UI what variables are available from the optimizer
	void sg_simulationStarted(const QVector<ASCO_Design_Variable_Properties> &design_variables, const QVector<ASCO_Measurement_Properties>& measurements);
	//emitted on simulation start to notify the UI what results (ex: S paramaeters) are available from the optimizer
	void sg_availableResults(const QMap<QString, QStringList> &results);
	
	//emitted when a new simulation completed but before the results are parsed
	void sg_simulationUpdate(const QString &path);
	
	//emitted when the optimizer stops running
	void sg_simulationFinished();
	
	//emitted whenever a new simulation was run during the optimization
	void sg_updateCost(const double &value);

	//emitted whenever a new simulation was run during the optimization
	void sg_updateDesignVariables(const QStringList &design_vars, const QVector<double> &values);

	//emitted whenever a new simulation was run during the optimization
	void sg_updateMeasurements(const QStringList &measurements, const QVector<double> &values);

	//emitted whenever a simulation finished
	void sg_updateResult(const QVector<double> &independent, const QVector<double> &dependent);
	



public slots:
	void sl_newQucsDir(const QString &path);
	void sl_selectDataToEmit(const QString& independent_variable, const QString &dependent_variable);
	//emits the sg_updateResult signals containing the requested data, if it exists
	void sl_getResult(const QString &s_active_independent, const QString &s_active_dependent);

	void sl_setEnable(const bool& enable);

private slots:
	void sl_simulationUpdate(const QString &path);

	//members

private:
	QScopedPointer<QTimer> tmr_sim_done;
	QScopedPointer<QFileSystemWatcher> watch_sim_updates;
	QScopedPointer<QFileSystemWatcher> watch_sim_start;
	QString s_qucs_dir;
	QString s_hostname_log_path;
	QString s_asco_config_path;
	QString s_dat_path;
	QString s_hostname;
	QString s_independent_variable;
	QString s_dependent_variable;
	QStringList s_measurements;
	QStringList s_design_variables;
	QRegularExpression re_log;
	QFile f_hostname_log;
	bool b_sim_running;
	bool b_enabled;
	
	QMutex mutex_qucs_dat;
	QScopedPointer<Qucs_Dat> o_qucs_dat;
};