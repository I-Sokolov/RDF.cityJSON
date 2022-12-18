#pragma once



struct VECTOR
{
	double			x;
	double			y;
	double			z;
};

struct PLANE
{
	double			a;
	double			b;
	double			c;
	double			d;
};

struct INDEX_LL
{
	__int64			index;
	INDEX_LL		* next;
};

struct FACE
{
	PLANE			plane;
	INDEX_LL		* indices;

	FACE			* openings;
};



void	PlaneFromPoints(
				PLANE			* pOut,
				const VECTOR	* pV1,
				const VECTOR	* pV2,
				const VECTOR	* pV3
			);

void	Vec3Subtract(
				VECTOR			* pOut,
				const VECTOR	* pV1,
				const VECTOR	* pV2
			);

double	Vec3Normalize(
				VECTOR			* pInOut
			);

__int64	CreateInstanceMatrix(
				double			* matrix
			);
