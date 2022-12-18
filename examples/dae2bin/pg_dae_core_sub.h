#ifndef __RDF_LTD__PG_DAE_CORE_SUB_H
#define __RDF_LTD__PG_DAE_CORE_SUB_H


#include	"../../include/engine.h"


struct	MA__TRIX
{
	double		_11;
	double		_12;
	double		_13;
	double		_21;
	double		_22;
	double		_23;
	double		_31;
	double		_32;
	double		_33;
	double		_41;
	double		_42;
	double		_43;
};	//	*/


void	InitializeGeometry_DAE_sub(
				int64_t			model,
				const char		* inputFileName
			);

void	InitializeGeometry_DAE_sub(
				int64_t			model,
				const wchar_t	* inputFileName
			);


#endif
