#ifndef __RDF_LTD__PG_DAE_GENERIC_H
#define __RDF_LTD__PG_DAE_GENERIC_H



struct NEWPARAM
{
	char				* sid;
	char				* value;

	NEWPARAM			* next;
};


bool	equal(
				char	* strI,
				char	* strII
			);

void	UpdateString(
				char	* str
			);


#endif
