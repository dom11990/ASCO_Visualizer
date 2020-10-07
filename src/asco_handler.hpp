
#pragma once


#include "asco_design_variable_properties.hpp"
#include "asco_measurement_properties.hpp"


#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QFile>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVector>





class ASCO_Handler : public QObject
{

	Q_OBJECT

public:
	ASCO_Handler(QObject *parent = 0);
	virtual ~ASCO_Handler();

private:
	void parseNetlistFile();

signals:
	void sg_simulationStarted(const QVector<ASCO_Design_Variable_Properties> &design_variables, const QVector<ASCO_Measurement_Properties>& measurements);
	void sg_simulationUpdate(const QString &path);
	void sg_simulationFinished();
	void sg_availableResults(const QMap<QString, QStringList> &results);

	//emitted whenever a new simulation was run during the optimization
	void sg_updateMeasurement(const QString &measurement, const double &value);
	void sg_updateDesignVariable(const QString &measurement, const double &value);
	void sg_updateResult(const QVector<double> &x, const QVector<double> &y);

	//emitted once when a new run of the optimizer is detected
	



public slots:
	void newQucsDir(const QString &path);

private slots:
	void sl_simulationUpdate(const QString &path);

	//members

private:
	QScopedPointer<QTimer> tmr_sim_done;
	QScopedPointer<QFileSystemWatcher> watch_sim_updates;
	QScopedPointer<QFileSystemWatcher> watch_sim_start;
	QString s_qucs_dir;
	QString s_host_log_path;
	QString s_asco_config_path;
	QString s_dat_path;
	QString s_hostname;
	bool b_sim_running;

	//QScopedPointer<Qucs_Dat> o_qucs_dat;
};