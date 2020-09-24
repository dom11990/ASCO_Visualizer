
#pragma once
#include <QString>

#include "asco_parameter_properties.hpp"



class ASCO_Measurement_Properties : public ASCO_Parameter_Properties
{
	private:

	public:

		ASCO_Measurement_Properties();
		~ASCO_Measurement_Properties();

		QString s_name;
		QString s_compare;
        double d_limit;
};