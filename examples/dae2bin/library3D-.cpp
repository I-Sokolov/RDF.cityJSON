
#include "stdafx.h"
#include "library3D.h"

#include "ifcengine/include/engine.h"



extern	__int64		rdfModel;



void	PlaneFromPoints(
				PLANE			* pOut,
				const VECTOR	* pV1,
				const VECTOR	* pV2,
				const VECTOR	* pV3
			)
{
	VECTOR	dV1, dV2;//, pTst;
	Vec3Subtract(&dV1, pV1, pV2);
	Vec3Subtract(&dV2, pV2, pV3);
	pOut->a = dV1.y * dV2.z - dV1.z * dV2.y;
	pOut->b = dV1.z * dV2.x - dV1.x * dV2.z;
	pOut->c = dV1.x * dV2.y - dV1.y * dV2.x;
	double	size = Vec3Normalize((VECTOR*) pOut);
	if	(size == 0) {
//		ASSERT((pOut->a > -0.00000001  &&  pOut->a < 0.00000001)  &&  (pOut->b > -0.00000001  &&  pOut->b < 0.00000001)  &&  (pOut->c > -0.00000001  &&  pOut->c < 0.00000001));
		pOut->a = 0;
		pOut->b = 0;
		pOut->c = 1;
		//////////////////////////////!!!!!!!!!!ASSERT(false);
	}
	pOut->d = -(pV3->x * pOut->a + pV3->y * pOut->b + pV3->z * pOut->c);
}

void	Vec3Subtract(
				VECTOR			* pOut,
				const VECTOR	* pV1,
				const VECTOR	* pV2
			)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
}

double	Vec3Normalize(
				VECTOR	* pInOut
			)
{
	double	size, sqrtSize = 0,
			x = pInOut->x, y = pInOut->y, z = pInOut->z;

	size = x * x + y * y + z * z;

	if	(size > 0.0000000000000001) {
		sqrtSize = sqrt(size);
		pInOut->x = x / sqrtSize;
		pInOut->y = y / sqrtSize;
		pInOut->z = z / sqrtSize;
	} else {
		if	(pInOut->x) { pInOut->x = 1000; }
		if	(pInOut->y) { pInOut->y = 1000; }
		if	(pInOut->z) { pInOut->z = 1000; }
	}

	return	sqrtSize;
}

__int64	CreateInstanceMatrix(
				double	* matrix
			)
{
	__int64	owlClassMatrix = GetClassByName(rdfModel, "Matrix"),
			rdfProperty_11 = GetPropertyByName(rdfModel, "_11"),
			rdfProperty_12 = GetPropertyByName(rdfModel, "_12"),
			rdfProperty_13 = GetPropertyByName(rdfModel, "_13"),
			rdfProperty_21 = GetPropertyByName(rdfModel, "_21"),
			rdfProperty_22 = GetPropertyByName(rdfModel, "_22"),
			rdfProperty_23 = GetPropertyByName(rdfModel, "_23"),
			rdfProperty_31 = GetPropertyByName(rdfModel, "_31"),
			rdfProperty_32 = GetPropertyByName(rdfModel, "_32"),
			rdfProperty_33 = GetPropertyByName(rdfModel, "_33"),
			rdfProperty_41 = GetPropertyByName(rdfModel, "_41"),
			rdfProperty_42 = GetPropertyByName(rdfModel, "_42"),
			rdfProperty_43 = GetPropertyByName(rdfModel, "_43");

	__int64	instanceMatrix = CreateInstance(owlClassMatrix, nullptr);

	SetDataTypeProperty(instanceMatrix, rdfProperty_11, &matrix[0], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_12, &matrix[4], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_13, &matrix[8], 1);
	ASSERT(matrix[12] == 0);
	SetDataTypeProperty(instanceMatrix, rdfProperty_21, &matrix[1], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_22, &matrix[5], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_23, &matrix[9], 1);
	ASSERT(matrix[13] == 0);
	SetDataTypeProperty(instanceMatrix, rdfProperty_31, &matrix[2], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_32, &matrix[6], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_33, &matrix[10], 1);
	ASSERT(matrix[14] == 0);
	SetDataTypeProperty(instanceMatrix, rdfProperty_41, &matrix[3], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_42, &matrix[7], 1);
	SetDataTypeProperty(instanceMatrix, rdfProperty_43, &matrix[11], 1);
	ASSERT(matrix[15] == 1);

	return	instanceMatrix;
}