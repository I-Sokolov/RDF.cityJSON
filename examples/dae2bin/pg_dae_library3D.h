#ifndef __RDF_LTD__PG_DAE_LIBRARY_3D_H
#define __RDF_LTD__PG_DAE_LIBRARY_3D_H


#include	"../../include/engdef.h"


struct VE__CTOR
{
	double			x;
	double			y;
	double			z;
};

struct PL__ANE
{
	double			a;
	double			b;
	double			c;
	double			d;
};

struct INDEX_LL
{
	int64_t			index;
	INDEX_LL		* next;
};

struct FA__CE
{
	PL__ANE			plane;
	INDEX_LL		* indices;

	FA__CE			* openings;
};



void	PlaneFromPoints(
				PL__ANE			* pOut,
				const VE__CTOR	* pV1,
				const VE__CTOR	* pV2,
				const VE__CTOR	* pV3
			);

void	Vec3Subtract(
				VE__CTOR			* pOut,
				const VE__CTOR	* pV1,
				const VE__CTOR	* pV2
			);

double	Vec3Normalize(
				VE__CTOR			* pInOut
			);

int64_t	CreateInstanceMatrix(
				int64_t			rdfModel,
				double			* matrix
			);


#endif
