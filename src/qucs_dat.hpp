
#pragma once
#include <QFile>
#include <QMap>
#include <QVector>
#include <complex>


typedef double Qucs_Numeric_Data_t;
typedef QMap<QString, QMap<QString, QVector<Qucs_Numeric_Data_t>>> Qucs_Map_Data_t;

class Qucs_Dat
{
private:


public:
	Qucs_Dat();
	~Qucs_Dat();
	Qucs_Dat (const Qucs_Dat &obj);

	bool Parse_File(const QString& path);
	QStringList getDependentVariables(const QString& indepdenent_var);
	QStringList getIndependentVariables(int min_size = 2);
	bool getData(const QString & indep_name, const QString & dep_name, QVector<double>& indep_data, QVector<double>& dep_data);
	bool exists(const QString &indep_name, const QString & dep_name);
private:
	void Parse_Data(QFile &file, QVector<Qucs_Numeric_Data_t>& vector);
	Qucs_Map_Data_t m_data;


};