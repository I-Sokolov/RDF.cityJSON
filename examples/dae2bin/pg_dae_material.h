#ifndef __RDF_LTD__PG_DAE_MATERIAL_H
#define __RDF_LTD__PG_DAE_MATERIAL_H


#include "pg_dae_generic.h"

#include	"../../include/engdef.h"


enum colorType	{
	blinn,
	constant,
	lambert,
	phong
};

struct CO__LOR
{
	char				* id;

	colorType			type;

	double				* emission;
	double				* ambient;
	double				* diffuse;
	double				* specular;

	double				* transparent;
	double				transparency;

	int64_t				rdfInstance;

	bool				hasTexture;

	char				* textureName;

	NEWPARAM			* newparam;
	CO__LOR				* next;
};

struct MA__TERIAL
{
	char				* id;

	CO__LOR				* color;

	MA__TERIAL			* next;
};



//
//	Color
//

CO__LOR		* new_COLOR(
					colorType	type
				);

CO__LOR		* GetColor(
					char		* id
				);

void		SetColor(
					int64_t		model,
					CO__LOR		* newColor
				);

//
//	Material
//

MA__TERIAL	* GetMaterial(
					char		* id
				);

void		SetMaterial(
					MA__TERIAL	* newMaterial
				);


#endif
