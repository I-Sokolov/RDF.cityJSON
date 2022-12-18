#ifndef __RDF_LTD__PG_DAE_IMAGE_H
#define __RDF_LTD__PG_DAE_IMAGE_H


#include "pg_dae_generic.h"


struct IMAGE
{
	char				* id;

	char				* fileName;

	IMAGE				* next;
};


void	SetImage(
				IMAGE		* newImage
			);

IMAGE	* GetImage(
				char		* id,
				NEWPARAM	* newParam
			);


#endif
