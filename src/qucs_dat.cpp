#include "qucs_dat.hpp"
#include <QRegularExpression>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>


Qucs_Dat::Qucs_Dat()
{
    dir_temp.setPath("/mnt/ramdisk");
}

Qucs_Dat::~Qucs_Dat()
{
}

Qucs_Dat::Qucs_Dat(const Qucs_Dat &obj) 
{
    this->m_data = obj.m_data;
}

bool Qucs_Dat::Parse_File(const QString& path)
{ 
    
    m_data = Qucs_Map_Data_t();
    if(path.isEmpty()){
        return false;
    }
    
    bool retval;
    QFileInfo source(path);
    QFile f_file;
    QDir folder(source.dir());

    f_file.setFileName(folder.absoluteFilePath(source.fileName() + "-tmp"));
    if(dir_temp.exists()){
        //then use the ramdisk folder
        f_file.setFileName(dir_temp.filePath(source.fileName()));
    }
    
    QFile::copy(path, f_file.fileName());
    
    
    if (f_file.open(QIODevice::ReadOnly))
    {
        //parse the file line by line
        while (f_file.pos() < f_file.size())
        {
            QString line = f_file.readLine();

            if (line.contains("<indep", Qt::CaseSensitive))
            {
                //this is an independent variable
                QRegularExpression regex("<indep (.+) (\\d+)>");
                QRegularExpressionMatch match = regex.match(line);
                if (match.lastCapturedIndex() != 2)
                {
                    // qDebug() << "match count: " << match.lastCapturedIndex();
                    qFatal(("Line: " + line + " did not have 2 captures as is expected for an independent!").toStdString().c_str());
                }
                QString var_name = match.captured(1);
                int var_size = match.captured(2).toInt();
                QVector<Qucs_Numeric_Data_t> sim_data;
                //allocate space for the incoming data
                sim_data.reserve(var_size);
                Parse_Data(f_file, sim_data);

                //read out all the numbers, now store this new array
                m_data[var_name][var_name] = sim_data;
            }
            //parse dependent variables
            if (line.contains("<dep", Qt::CaseSensitive))
            {
                //this is a dependent variable
                QRegularExpression regex("<dep (.+) (.+)>");
                QRegularExpressionMatch match = regex.match(line);
                if (match.lastCapturedIndex() != 2)
                {
                    qFatal(("Line: " + line + " did not have 2 captures as is expected for an independent!").toStdString().c_str());
                }
                QString var_name = match.captured(1);
                QString var_dependency = match.captured(2);

                int var_size = m_data[var_dependency][var_dependency].size();
                QVector<Qucs_Numeric_Data_t> sim_data;
                //allocate space for the incoming data
                sim_data.reserve(var_size);
                Parse_Data(f_file, sim_data);

                //read out all the numbers, now store this new array
                QMap<QString, QVector<Qucs_Numeric_Data_t>> *element;
                if (m_data.find(var_dependency) != m_data.end())
                {
                    //other variables also have this dependency, so we need to not overwrite them
                    element = &m_data[var_dependency];
                    (*element)[var_name] = sim_data;
                }
                else
                {
                    m_data[var_dependency][var_name] = sim_data;
                }
            }
        }
        f_file.close();
    }else{
        //unable to open the file
        QString error_msg("Unable to open data file at" + f_file.fileName() + " for parsing." );
        qFatal(error_msg.toStdString().c_str());
        return false;
    }
    f_file.remove();
    return true;
}

QStringList Qucs_Dat::getDependentVariables(const QString& indepdenent_var) 
{
    QStringList result = m_data[indepdenent_var].keys();
    //remove the values of the independent variable which is stored undre m_data["indep_name"]["indep_name"]
    result.removeOne(indepdenent_var);
    return result;
}

QStringList Qucs_Dat::getIndependentVariables(int min_size) 
{
    QStringList result = m_data.keys();
    
    for (QString var : result){
        if(m_data[var].keys().count() < min_size){
            result.removeOne(var);        
        }
    }
    return result;
}

bool Qucs_Dat::getData(const QString & indep_name, const QString & dep_name, QVector<Qucs_Numeric_Data_t>& indep_data, QVector<Qucs_Numeric_Data_t>& dep_data) 
{
    if(indep_name.isEmpty() || dep_name.isEmpty()){
        return false;
    }
        

    if(m_data.find(indep_name) != m_data.end()){
        if(m_data[indep_name].find(dep_name) != m_data[indep_name].end()){
            indep_data = m_data[indep_name][indep_name];
            dep_data = m_data[indep_name][dep_name];
            return true;
        }
        
    }
    return false;   
}

bool Qucs_Dat::exists(const QString &indep_name, const QString & dep_name) 
{
    if(m_data.find(indep_name) != m_data.end()){
       //see if this combincation of indep + dependent variable exists
       return m_data[indep_name].find(dep_name) != m_data[indep_name].end();
    }
    //couldnt even find the independent variable
    return false;

}

void Qucs_Dat::Parse_Data(QFile &file, QVector<Qucs_Numeric_Data_t>& vector)
{
    QString line = file.readLine();
    while (!line.contains("</indep>") && !line.contains("</dep>") && file.pos() < file.size())
    {
        //check if this is a complex number
        if (line.contains("j"))
        {
            int j_pos = line.lastIndexOf("j");
            double imag = line.mid(j_pos).mid(1).toDouble(); //remove the j
            double real = line.mid(0, j_pos).toDouble();
            Qucs_Numeric_Data_t to_append;
            //take the magnitude of the complex number
            to_append = std::abs(std::complex<Qucs_Numeric_Data_t>(real,imag));
            vector.append(Qucs_Numeric_Data_t(to_append));
        }
        else
        {
            double real = line.toDouble();
            Qucs_Numeric_Data_t to_append(real);
            vector.append(Qucs_Numeric_Data_t(to_append));
        }
        //get the next line
        line = file.readLine();
    }
}