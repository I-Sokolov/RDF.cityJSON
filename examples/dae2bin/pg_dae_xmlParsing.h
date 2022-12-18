#ifndef __RDF_LTD__PG_DAE_XML_PARSING_H
#define __RDF_LTD__PG_DAE_XML_PARSING_H


#include "pg_dae_generic.h"


struct	STRUCT_KEY
{
	char		* name;
	STRUCT_KEY	* next;
};

struct	STRUCT_ATTR
{
	STRUCT_KEY	* key;
	char		* value;

	STRUCT_ATTR	* next;
};

struct	STRUCT_ITEM
{
	STRUCT_KEY	* key;
	char		* value;

	STRUCT_ATTR	* attr;

	STRUCT_ITEM	* child;
	STRUCT_ITEM	* next;
};



STRUCT_KEY	* GetKey(
					char			* name
				);

STRUCT_ITEM	* parseXML(
					const char		* fileName
				);

STRUCT_ITEM	* parseXML(
					const wchar_t	* fileName
				);


#endif
