
#include "stdafx.h"
#include "optimize3D.h"

#include <cmath>

#include "ifcengine/include/engine.h"



extern	__int64		rdfModel;
extern	bool		DUPLICATE_FACES,
					MERGE_FACES;



__int64		MinimizeVertices(
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					__int64		vertexArraySize,			//	measured in number of elements
					__int64		* indexArray,				//	we expect polygons ending with -1
					__int64		indexArraySize				//	measured in number of elements
				)
{
	int	VERTEX_XYZ_SIZE = 3,
		VERTEX_TS_SIZE = 2;
	//
	//	Merge points
	//
	__int64	* pointMap = new __int64[vertexArraySize],
			mappedPoints = 0;

	for (int i = 0; i < vertexArraySize; i++) {
		pointMap[i] = -1;
	}

	double	epsilon = 0.001;// 0.000001;

	for (int i = 0; i < vertexArraySize; i++) {
		for (int j = i + 1; j < vertexArraySize; j++) {
			if ((std::fabs(vertexXYZArray[i * VERTEX_XYZ_SIZE + 0] - vertexXYZArray[j * VERTEX_XYZ_SIZE + 0]) < epsilon) &&
				(std::fabs(vertexXYZArray[i * VERTEX_XYZ_SIZE + 1] - vertexXYZArray[j * VERTEX_XYZ_SIZE + 1]) < epsilon) &&
				(std::fabs(vertexXYZArray[i * VERTEX_XYZ_SIZE + 2] - vertexXYZArray[j * VERTEX_XYZ_SIZE + 2]) < epsilon)) {
				if ((vertexTextureArray == 0) ||
					((std::fabs(vertexTextureArray[i * VERTEX_TS_SIZE + 0] - vertexTextureArray[j * VERTEX_TS_SIZE + 0]) < epsilon) &&
					 (std::fabs(vertexTextureArray[i * VERTEX_TS_SIZE + 1] - vertexTextureArray[j * VERTEX_TS_SIZE + 1]) < epsilon))) {
					if (pointMap[i] >= 0) {
						if (pointMap[j] == -1) {
							pointMap[j] = pointMap[i];
							mappedPoints++;
						}
	//////////					ASSERT(pointMap[i] == pointMap[j]);
						ASSERT(pointMap[i] < i);
					} else {
						if (pointMap[j] >= 0) {
//							ASSERT(pointMap[i] == -1 && pointMap[j] == -1);
//							pointMap[i] = pointMap[j];
//							mappedPoints++;
						} else {
							ASSERT(pointMap[i] == -1 && pointMap[j] == -1);
							pointMap[j] = i;
							mappedPoints++;
						}
					}
				}
			}
		}
	}

	ASSERT(mappedPoints >= 0 && mappedPoints < vertexArraySize);

	//
	//	Check index array for consitancy
	//
	int i = 0, polygonSize = 0;
	while (i < indexArraySize) {
		if (indexArray[i] >= 0) {
			ASSERT(indexArray[i] < vertexArraySize);
			int iAi = indexArray[i];
			if (pointMap[iAi] != -1) {
				ASSERT(pointMap[iAi] >= 0 && pointMap[iAi] < iAi);
				ASSERT(pointMap[pointMap[iAi]] == -1);
				indexArray[i] = pointMap[iAi];
			}
			polygonSize++;
		} else {
			ASSERT(polygonSize > 2);
			polygonSize = 0;
		}
		i++;
	}
	ASSERT(polygonSize == 0);

	//
	//	Now adjust buffers:
	//		vertexXYZArray
	//		vertexTSArray
	//		indexArray
	//

	//	adjust pointMap
	__int64 cnt = 0;
	i = 0;
	while (i < vertexArraySize) {
		if (pointMap[i] == -1) {
			pointMap[i] = cnt;
			cnt++;
		} else {
			pointMap[i] = -2;
		}
		i++;
	}
	ASSERT(cnt + mappedPoints == vertexArraySize);

	//		vertexXYZArray
	//		vertexTSArray
	i = 0;
	while (i < vertexArraySize) {
		if (pointMap[i] >= 0) {
			ASSERT(pointMap[i] < cnt);
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 0] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 0];
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 1] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 1];
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 2] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 2];
			if (vertexTextureArray) {
				vertexTextureArray[pointMap[i] * VERTEX_TS_SIZE + 0] = vertexTextureArray[i * VERTEX_TS_SIZE + 0];
				vertexTextureArray[pointMap[i] * VERTEX_TS_SIZE + 1] = vertexTextureArray[i * VERTEX_TS_SIZE + 1];
			}
		} else {
			ASSERT(pointMap[i] == -2);
		}
		i++;
	}

	//		indexArray
	i = 0;
	while (i < indexArraySize) {
		if (indexArray[i] >= 0) {
			ASSERT(pointMap[indexArray[i]] >= 0 && pointMap[indexArray[i]] < cnt);
			indexArray[i] = pointMap[indexArray[i]];
			ASSERT(indexArray[i] >= 0 && indexArray[i] < cnt);
		} else {
			ASSERT(indexArray[i] == -1);
		}
		i++;
	}

	return	cnt;
}

double		GetMaxSize(
					INDEX_LL	* polygon,
					double		* vertexArray				//	we expect now (X, Y, Z) coordinates
				)
{
	double	size = 0;

	if (polygon) {
		double	minX = vertexArray[3 * polygon->index + 0], maxX = minX,
				minY = vertexArray[3 * polygon->index + 1], maxY = minY,
				minZ = vertexArray[3 * polygon->index + 2], maxZ = minZ;
		polygon = polygon->next;

		while (polygon)  {
			if (minX > vertexArray[3 * polygon->index + 0]) { minX = vertexArray[3 * polygon->index + 0]; }
			if (minY > vertexArray[3 * polygon->index + 1]) { minY = vertexArray[3 * polygon->index + 1]; }
			if (minZ > vertexArray[3 * polygon->index + 2]) { minZ = vertexArray[3 * polygon->index + 2]; }

			if (maxX < vertexArray[3 * polygon->index + 0]) { maxX = vertexArray[3 * polygon->index + 0]; }
			if (maxY < vertexArray[3 * polygon->index + 1]) { maxY = vertexArray[3 * polygon->index + 1]; }
			if (maxZ < vertexArray[3 * polygon->index + 2]) { maxZ = vertexArray[3 * polygon->index + 2]; }

			polygon = polygon->next;
		}

		size = (maxX - minX) + (maxY - minY) + (maxZ - minZ);
	}

	return	size;
}

__int64		MergeFaces(
					double		* vertexArray,				//	we expect now (X, Y, Z) coordinates
					__int64		vertexArraySize,			//	measured in number of elements
					__int64		* indexArray,				//	we expect polygons ending with -1
					__int64		indexArraySize				//	measured in number of elements
				)
{
	int	noFaces = 0, faceSize = 0;
	for (int j = 0; j < indexArraySize; j++) {
		if (indexArray[j] == -1) {
			ASSERT(faceSize > 2);
			faceSize = 0;
			noFaces++;
		} else {
			ASSERT(indexArray[j] >= 0 && indexArray[j] < vertexArraySize);
			faceSize++;
		}
	}
	ASSERT(faceSize == 0);

	FACE	** faces = (FACE **) new __int64[noFaces];

	int i = 0, facesCnt = 0, prevI = 0;
	while (i < indexArraySize) {
		if (indexArray[i] == -1) {
			faces[facesCnt] = new FACE;
			faces[facesCnt]->plane.a = 0;
			faces[facesCnt]->plane.b = 0;
			faces[facesCnt]->plane.c = 1;
			faces[facesCnt]->plane.d = 0;
			faces[facesCnt]->indices = nullptr;
			faces[facesCnt]->openings = nullptr;
			INDEX_LL	** ppIndices = &faces[facesCnt]->indices;
			while (prevI < i) {
				(*ppIndices) = new INDEX_LL;
				(*ppIndices)->index = indexArray[prevI];
				(*ppIndices)->next = nullptr;

				ASSERT((*ppIndices)->index >= 0 && (*ppIndices)->index < vertexArraySize);

				ppIndices = &(*ppIndices)->next;
				prevI++;
			}

			(*ppIndices) = new INDEX_LL;
			(*ppIndices)->index = faces[facesCnt]->indices->index;
			(*ppIndices)->next = nullptr;
			prevI++;

			VECTOR		vec1, vec2, vec3;
			INDEX_LL	* iLL1 = faces[facesCnt]->indices, * iLL2 = iLL1->next, * iLL3 = iLL2->next;
			vec1.x = vertexArray[3 * iLL1->index + 0];
			vec1.y = vertexArray[3 * iLL1->index + 1];
			vec1.z = vertexArray[3 * iLL1->index + 2];
			vec2.x = vertexArray[3 * iLL2->index + 0];
			vec2.y = vertexArray[3 * iLL2->index + 1];
			vec2.z = vertexArray[3 * iLL2->index + 2];
			vec3.x = vertexArray[3 * iLL3->index + 0];
			vec3.y = vertexArray[3 * iLL3->index + 1];
			vec3.z = vertexArray[3 * iLL3->index + 2];

			PlaneFromPoints(&faces[facesCnt]->plane, &vec1, &vec2, &vec3);

			facesCnt++;
		}
		i++;
	}
	ASSERT(noFaces == facesCnt);

	//
	//	Merge planes
	//
	
	i = 0;
	while (i < facesCnt) {
		int j = 0;
		while (j < i) {
			if ((std::fabs(faces[j]->plane.a - faces[i]->plane.a) < 0.0001) &&
				(std::fabs(faces[j]->plane.b - faces[i]->plane.b) < 0.0001) &&
				(std::fabs(faces[j]->plane.c - faces[i]->plane.c) < 0.0001) &&
				(std::fabs(faces[j]->plane.d - faces[i]->plane.d) < 0.0001)) {
				INDEX_LL	* refJ = faces[j]->indices;
				while (refJ  && refJ->next  &&  faces[j]->indices) {
					__int64	jInd_I = refJ->index, jInd_II = refJ->next->index;

					INDEX_LL	* refI = faces[i]->indices;
					while (refI  && refI->next) {
						__int64	iInd_I = refI->index, iInd_II = refI->next->index;

						if (iInd_I == jInd_II  &&  iInd_II == jInd_I) {
							//
							//	Merge FACES!!!
							//
							INDEX_LL	**ppMyRef = &faces[i]->indices;

							__int64	* list = new __int64[indexArraySize], k = 0;
							refI = refI->next;
							while (refI  &&  refI->next) {
								list[k++] = refI->index;
								refI = refI->next;
							}
							ASSERT(refI->index == faces[i]->indices->index);
							refI = faces[i]->indices;
							while (refI  &&  refI->index != iInd_I) {
								list[k++] = refI->index;
								refI = refI->next;
							}
							ASSERT(refI->index == iInd_I);

							refJ = refJ->next;
							while (refJ  &&  refJ->next) {
								list[k++] = refJ->index;
								refJ = refJ->next;
							}
							ASSERT(refJ->index == faces[j]->indices->index);
							refJ = faces[j]->indices;
							while (refJ  &&  refJ->index != jInd_I) {
								list[k++] = refJ->index;
								refJ = refJ->next;
							}
							ASSERT(refJ->index == jInd_I);

							__int64	n = 0;
							INDEX_LL	** ppRef = &faces[i]->indices;
							while (n < k) {
								(*ppRef) = new INDEX_LL;
								(*ppRef)->index = list[n];
								(*ppRef)->next = nullptr;
								ppRef = &(*ppRef)->next;
								n++;
							}
							(*ppRef) = new INDEX_LL;
							(*ppRef)->index = list[0];
							(*ppRef)->next = nullptr;



							delete[]  list;

							//		...
//		(*ppMyRef) = new REF;
//		(*ppMyRef)->index = ..
//		(*ppMyRef)->next = nullptr;
//		ppMyRef = &(*ppMyRef)->next;

faces[j]->indices = nullptr;	//	*/
refI = nullptr;
						}
 else {
	 refI = refI->next;
 }
					}

					refJ = refJ->next;
				}
			}
			j++;
		}
		i++;
	}	//	*/
	//	...
	//	...
	//	...

	//
	//	Polygons are ready, check if there are no hidden inner openings
	//
	i = 0;
	while (i < facesCnt) {
		FACE	* face = faces[i];
		bool	found = false;
		if (face  &&  face->indices) {
			INDEX_LL	* indI = face->indices;
			while (indI) {
				INDEX_LL	* indII = face->indices;
				while (indII != indI) {
					if ((indI->next  &&  indII->next) &&
						(indI->index == indII->next->index) &&
						(indI->next->index == indII->index)) {
						//
						//	Found inner opening
						//		split up faces
						//		decide what is the outer face
						//
						INDEX_LL	* polygonI = face->indices,
							*polygonII = indII->next;
						indII->next = indI->next->next;
						indI->next = 0;

						double	polygonI_size = GetMaxSize(polygonI, vertexArray),
							polygonII_size = GetMaxSize(polygonII, vertexArray);

						FACE	* opening = new FACE;
						opening->plane.a = face->plane.a;
						opening->plane.b = face->plane.b;
						opening->plane.c = face->plane.c;
						opening->plane.d = face->plane.d;
						opening->openings = face->openings;

						if (polygonI_size > polygonII_size) {
							ASSERT(face->indices == polygonI);
							opening->indices = polygonII;
						}
						else {
							face->indices = polygonII;
							opening->indices = polygonI;
						}

						face->openings = opening;
						indI = 0;
						indII = 0;

						found = true;
					}
					else {
						indII = indII->next;
					}
				}

				if (indI) {
					indI = indI->next;
				}
			}
		}

		if (found == false) {
			i++;
		}
	}

	//
	//	Put them back
	//

	//	__int64	* indices = new __int64[2 * indexArraySize];
	__int64	m = 0;
	i = 0;
	while (i < facesCnt) {
		FACE	* face = faces[i];
		if (face  &&  face->indices) {
			if ((face->indices) &&
				(face->indices->next) &&
				(face->indices->next->next)) {
				INDEX_LL	* ref = face->indices;
				while (ref  &&  ref->next) {
					indexArray[m++] = ref->index;
					ref = ref->next;
				}
				ASSERT(ref->index == face->indices->index);
				indexArray[m++] = -1;

				FACE	* opening = face->openings;
				while (opening) {
					if ((opening->indices) &&
						(opening->indices->next) &&
						(opening->indices->next->next)) {
						INDEX_LL	* ref = opening->indices;
						while (ref  &&  ref->next) {
							indexArray[m++] = ref->index;
							ref = ref->next;
						}
						ASSERT(ref->index == opening->indices->index);
						indexArray[m++] = -2;
					}
					opening = opening->openings;
				}
			}
		}

		i++;
	}	//	*/

	ASSERT(m <= indexArraySize);

	//
	//	...
	//

	for (int k = 0; k < noFaces; k++) {
		if (faces[k]) {
			//	STILL TO ADD = REMOVE LINKED LIST OF INDICES
			delete faces[k];
		}
	}
	delete[] faces;

	return	m;
}

void		MergeFaces__new(
					__int64		owlInstanceBoundaryRepresentationNormal,
					__int64		owlInstanceBoundaryRepresentationInverted,
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					__int64		vertexArraySize,			//	measured in number of elements
					__int64		* indexArray,				//	we expect polygons ending with -1
					__int64		indexArraySize				//	measured in number of elements
				)
{
	bool		inverted = false;

	vertexArraySize = MinimizeVertices(
							vertexXYZArray,
							vertexTextureArray,
							vertexArraySize,
							indexArray,
							indexArraySize
						);

	if (MERGE_FACES) {
		indexArraySize = MergeFaces(
								vertexXYZArray,
								vertexArraySize,
								indexArray,
								indexArraySize
							);
	}		

	__int64	owlClassBoundaryRepresentation = GetClassByName(rdfModel, "BoundaryRepresentation"),
			rdfPropertyIndices = GetPropertyByName(rdfModel, "indices"),
			rdfPropertyTextureCoordinates = GetPropertyByName(rdfModel, "textureCoordinates"),
			rdfPropertyVertices = GetPropertyByName(rdfModel, "vertices");
	
	SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyVertices, vertexXYZArray, 3 * vertexArraySize);
	SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyVertices, vertexXYZArray, 3 * vertexArraySize);
	if (vertexTextureArray) {
		SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyTextureCoordinates, vertexTextureArray, 2 * vertexArraySize);
		SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyTextureCoordinates, vertexTextureArray, 2 * vertexArraySize);
	}

	//
	//	Remove single points and single lines
	//
	__int64 m = 0, elements = 0, first = 0;
	while (m < indexArraySize) {
		if (indexArray[m] >= 0) {
			ASSERT(indexArray[m] < vertexArraySize);
			elements++;
		} else {
			ASSERT(indexArray[m] == -1 || indexArray[m] == -2);
			if (elements < 3) {
				//
				//	we do not check if this is the parent and maybe children are ignored
				//
				m -= elements;
				elements++;
				indexArraySize -= elements;
				
				__int64 n = m;
				while (n < indexArraySize) {
					indexArray[n] = indexArray[n + elements];
					n++;
				}
			}

			elements = 0;
			first = m + 1;
		}
		m++;
	}

	if (DUPLICATE_FACES == false && inverted == false) {
		SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyIndices, indexArray, indexArraySize);
	} else {
		m = 0;
		__int64	* extendedIndexArray = new __int64[2 * indexArraySize];
		elements = 0;
		first = 0;
		while (m < indexArraySize) {
			extendedIndexArray[m + 0] = indexArray[m];
			extendedIndexArray[m + indexArraySize] = indexArray[m];
			if (indexArray[m] >= 0) {
				ASSERT(indexArray[m] < vertexArraySize);
				elements++;
			}
			else {
				ASSERT(indexArray[m] == -1 || indexArray[m] == -2);
				ASSERT(elements >= 3);

				__int64	last = m - 1;
				while (first < last) {
					__int64	tmp = extendedIndexArray[first];
					extendedIndexArray[first] = extendedIndexArray[last];
					extendedIndexArray[last] = tmp;
					ASSERT(extendedIndexArray[first] >= 0 && extendedIndexArray[first] < vertexArraySize);
					ASSERT(extendedIndexArray[last] >= 0 && extendedIndexArray[last] < vertexArraySize);
					first++;
					last--;
				}

				elements = 0;
				first = m + 1;
			}
			m++;
		}
		ASSERT(elements == 0);


		if (inverted) {
			SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyIndices, &extendedIndexArray[0],				indexArraySize);
			SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal,   rdfPropertyIndices, &extendedIndexArray[indexArraySize], indexArraySize);
		} else {
			SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyIndices, &extendedIndexArray[0], indexArraySize * 2);
			SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyIndices, &extendedIndexArray[0], indexArraySize * 2);
		}
	}
}
