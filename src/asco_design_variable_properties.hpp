#pragma once

#include "asco_parameter_properties.hpp"

class ASCO_Design_Variable_Properties  : public ASCO_Parameter_Properties
{
	private:

	public:

		ASCO_Design_Variable_Properties();
		~ASCO_Design_Variable_Properties();

		double d_initial;
		double d_min;
		double d_max;
		QString s_interpolate;
		QString s_optimize;
};