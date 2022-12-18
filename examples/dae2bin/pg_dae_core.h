#ifndef __RDF_LTD__PG_DAE_CORE_H
#define __RDF_LTD__PG_DAE_CORE_H


#include	"../../include/engine.h"


void	InitializeGeometry_DAE(
				int64_t			model,
				const char		* inputFileName
			);

void	InitializeGeometry_DAE(
				int64_t			model,
				const wchar_t	* inputFileName
			);


#endif
