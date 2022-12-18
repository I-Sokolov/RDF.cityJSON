
#include "stdafx.h"
#include "pg_dae_optimize3D.h"

#include <cmath>
#include <assert.h>

#include	"../../include/engine.h"



//extern	int64_t		rdfModel;
extern	bool		DUPLICATE_FACES,
					MERGE_FACES;



int64_t		MinimizeVertices(
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					int64_t		vertexArraySize,			//	measured in number of elements
					int64_t		* indexArray,				//	we expect polygons ending with -1
					int64_t		indexArraySize				//	measured in number of elements
				)
{
	int	VERTEX_XYZ_SIZE = 3,
		VERTEX_TS_SIZE = 2;
	//
	//	Merge points
	//
	int64_t	* pointMap = new int64_t[(int_t) vertexArraySize],
			mappedPoints = 0;

	int i = 0;
	while (i < vertexArraySize) {
		pointMap[i] = -1;
		i++;
	}

	double	epsilon = 0.001;// 0.000001;

	i = 0;
	while  (i < vertexArraySize) {
		int j = i + 1;
		while (j < vertexArraySize) {
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
	//////////					assert(pointMap[i] == pointMap[j]);
						assert(pointMap[i] < i);
					} else {
						if (pointMap[j] >= 0) {
//							assert(pointMap[i] == -1 && pointMap[j] == -1);
//							pointMap[i] = pointMap[j];
//							mappedPoints++;
						} else {
							assert(pointMap[i] == -1 && pointMap[j] == -1);
							pointMap[j] = i;
							mappedPoints++;
						}
					}
				}
			}
			j++;
		}
		i++;
	}

	assert(mappedPoints >= 0 && mappedPoints < vertexArraySize);

	//
	//	Check index array fo r consitancy
	//
	i = 0;
	int polygonSize = 0;
	while (i < indexArraySize) {
		if (indexArray[i] >= 0) {
			assert(indexArray[i] < vertexArraySize);
			int64_t iAi = indexArray[i];
			if (pointMap[(int) iAi] != -1) {
				assert(pointMap[(int) iAi] >= 0 && pointMap[(int) iAi] < iAi);
				assert(pointMap[(int) pointMap[(int) iAi]] == -1);
				indexArray[i] = pointMap[(int) iAi];
			}
			polygonSize++;
		} else {
			assert(polygonSize > 2);
			polygonSize = 0;
		}
		i++;
	}
	assert(polygonSize == 0);

	//
	//	Now adjust buffers:
	//		vertexXYZArray
	//		vertexTSArray
	//		indexArray
	//

	//	adjust pointMap
	int64_t cnt = 0;
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
	assert(cnt + mappedPoints == vertexArraySize);

	//		vertexXYZArray
	//		vertexTSArray
	i = 0;
	while (i < vertexArraySize) {
		if (pointMap[i] >= 0) {
			assert(pointMap[i] < cnt);
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 0] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 0];
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 1] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 1];
			vertexXYZArray[pointMap[i] * VERTEX_XYZ_SIZE + 2] = vertexXYZArray[i * VERTEX_XYZ_SIZE + 2];
			if (vertexTextureArray) {
				vertexTextureArray[pointMap[i] * VERTEX_TS_SIZE + 0] = vertexTextureArray[i * VERTEX_TS_SIZE + 0];
				vertexTextureArray[pointMap[i] * VERTEX_TS_SIZE + 1] = vertexTextureArray[i * VERTEX_TS_SIZE + 1];
			}
		}
		else {
			assert(pointMap[i] == -2);
		}
		i++;
	}

	//		indexArray
	i = 0;
	while (i < indexArraySize) {
		if (indexArray[i] >= 0) {
			assert(pointMap[indexArray[i]] >= 0 && pointMap[indexArray[i]] < cnt);
			indexArray[i] = pointMap[indexArray[i]];
			assert(indexArray[i] >= 0 && indexArray[i] < cnt);
		} else {
			assert(indexArray[i] == -1 || indexArray[i] == -2);
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

int	GetIndicesCnt(FA__CE * face)
{
	int			cnt = 0;
	INDEX_LL	* ind = face->indices;
	while (ind) {
		cnt++;
		ind = ind->next;
	}
	return	cnt;
}

int64_t		MergeFaces(
					double		* vertexArray,				//	we expect now (X, Y, Z) coordinates
					int64_t		vertexArraySize,			//	measured in number of elements
					int64_t		* indexArray,				//	we expect polygons ending with -1
					int64_t		indexArraySize				//	measured in number of elements
				)
{
	int	noFaces = 0, faceSize = 0;
	int j = 0;
	while (j < indexArraySize) {
		if (indexArray[j] == -1) {
			assert(faceSize > 2);
			faceSize = 0;
			noFaces++;
		} else if (indexArray[j] == -2) {
			assert(faceSize > 2);
			faceSize = 0;
		} else {
			assert(indexArray[j] >= 0 && indexArray[j] < vertexArraySize);
			faceSize++;
		}
		j++;
	}
	assert(faceSize == 0);

	FA__CE	** faces = (FA__CE **) new FA__CE*[noFaces];

	int i = 0, facesCnt = 0, prevI = 0;
	while (i < indexArraySize) {
		if (indexArray[i] == -1) {
			faces[facesCnt] = new FA__CE;
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

				assert((*ppIndices)->index >= 0 && (*ppIndices)->index < vertexArraySize);

				ppIndices = &(*ppIndices)->next;
				prevI++;
			}

			(*ppIndices) = new INDEX_LL;
			(*ppIndices)->index = faces[facesCnt]->indices->index;
			(*ppIndices)->next = nullptr;
			prevI++;

			VE__CTOR		vec1, vec2, vec3;
			INDEX_LL	* iLL1 = faces[facesCnt]->indices, *iLL2 = iLL1->next, *iLL3 = iLL2->next;
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
		else if (indexArray[i] == -2) {
			FA__CE	* opening = new FA__CE;
			opening->plane.a = faces[facesCnt - 1]->plane.a;
			opening->plane.b = faces[facesCnt - 1]->plane.b;
			opening->plane.c = faces[facesCnt - 1]->plane.c;
			opening->plane.d = faces[facesCnt - 1]->plane.d;
			opening->indices = nullptr;
			opening->openings = faces[facesCnt - 1]->openings;
			faces[facesCnt - 1]->openings = opening;
			INDEX_LL	** ppIndices = &opening->indices;
			while (prevI < i) {
				(*ppIndices) = new INDEX_LL;
				(*ppIndices)->index = indexArray[prevI];
				(*ppIndices)->next = nullptr;

				assert((*ppIndices)->index >= 0 && (*ppIndices)->index < vertexArraySize);

				ppIndices = &(*ppIndices)->next;
				prevI++;
			}

			(*ppIndices) = new INDEX_LL;
			(*ppIndices)->index = opening->indices->index;
			(*ppIndices)->next = nullptr;
			prevI++;
		}
		i++;
	}
	assert(noFaces == facesCnt);

	//
	//	Merge planes
	//


	i = 0;
	while (i < facesCnt) {
		int j = 0;
		while (j < i) {
			if ((std::fabs(faces[j]->plane.a - faces[i]->plane.a) < 0.001) &&
				(std::fabs(faces[j]->plane.b - faces[i]->plane.b) < 0.001) &&
				(std::fabs(faces[j]->plane.c - faces[i]->plane.c) < 0.001)) {
//			if ((std::fabs(faces[j]->plane.a - faces[i]->plane.a) < 0.0001) &&
//				(std::fabs(faces[j]->plane.b - faces[i]->plane.b) < 0.0001) &&
//				(std::fabs(faces[j]->plane.c - faces[i]->plane.c) < 0.0001) &&
//				(std::fabs(faces[j]->plane.d - faces[i]->plane.d) < 0.0001)) {
				INDEX_LL	* refJ = faces[j]->indices;
				while (refJ  && refJ->next  &&  faces[j]->indices) {
int	cntI = GetIndicesCnt(faces[i]),
	cntJ = GetIndicesCnt(faces[j]);

					int64_t	jInd_I = refJ->index, jInd_II = refJ->next->index;
					INDEX_LL	* refJ__jInd_I = refJ, * refJ__jInd_II = refJ->next;

					INDEX_LL	* refI = faces[i]->indices;
					while (refI  && refI->next) {
						int64_t		iInd_I = refI->index, iInd_II = refI->next->index;
						INDEX_LL	* refI__iInd_I = refI, *refI__iInd_II = refI->next;

						if (iInd_I == jInd_II  &&  iInd_II == jInd_I) {
							//
							//	Merge FACES!!!
							//
							INDEX_LL	**ppMyRef = &faces[i]->indices;

							int64_t* list = new int64_t[(int_t) indexArraySize], k = 0;
							refI = refI->next;
							while (refI  &&  refI->next) {
								list[k++] = refI->index;
								refI = refI->next;
							}
							assert(refI->index == faces[i]->indices->index);
							refI = faces[i]->indices;
//							while (refI  &&  refI->index != iInd_I) {
							while (refI  &&  refI != refI__iInd_I) {
								list[k++] = refI->index;
								refI = refI->next;
							}
							assert(refI->index == iInd_I);

							refJ = refJ->next;
							while (refJ  &&  refJ->next) {
								list[k++] = refJ->index;
								refJ = refJ->next;
							}
							assert(refJ->index == faces[j]->indices->index);
							refJ = faces[j]->indices;
//							while (refJ  &&  refJ->index != jInd_I) {
							while (refJ  &&  refJ != refJ__jInd_I) {
								list[k++] = refJ->index;
								refJ = refJ->next;
							}
							assert(refJ->index == jInd_I);

							int64_t	n = 0;
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

int cntResult = GetIndicesCnt(faces[i]);
assert((cntResult - 1) == (cntI - 1) + (cntJ - 1) - 2);

							delete[]  list;

							//		...
//		(*ppMyRef) = new REF;
//		(*ppMyRef)->index = ..
//		(*ppMyRef)->next = nullptr;
//		ppMyRef = &(*ppMyRef)->next;

faces[j]->indices = nullptr;	//	* /
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
		FA__CE	* face = faces[i];
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
									* polygonII = indII->next;
						indII->next = indI->next->next;
						indI->next = 0;

						double	polygonI_size = GetMaxSize(polygonI, vertexArray),
								polygonII_size = GetMaxSize(polygonII, vertexArray);

						FA__CE	* opening = new FA__CE;
						opening->plane.a = face->plane.a;
						opening->plane.b = face->plane.b;
						opening->plane.c = face->plane.c;
						opening->plane.d = face->plane.d;
						opening->openings = face->openings;

						if (polygonI_size > polygonII_size) {
							assert(face->indices == polygonI);
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
					} else {
						indII = indII->next;
					}
				}

				if (indI) {
					indI = indI->next;
				}
			}
		}

		if (found == false) {
			//
			//	Now the same for openings
			//
			FA__CE	* face = faces[i]->openings;
			while (face) {
				if (face && face->indices) {
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
											* polygonII = indII->next;
								indII->next = indI->next->next;
								indI->next = 0;

								FA__CE	* opening = new FA__CE;
								opening->plane.a = face->plane.a;
								opening->plane.b = face->plane.b;
								opening->plane.c = face->plane.c;
								opening->plane.d = face->plane.d;
								opening->openings = face->openings;

								face->indices = polygonII;
								opening->indices = polygonI;

								face->openings = opening;
								indI = 0;
								indII = 0;
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

				face = face->openings;
			}
		}

		i++;
	}

	//
	//	Second algorithm to find inner sections
	//
	i = 0;
	while (i < facesCnt) {
		FA__CE	* face = faces[i];
		bool	found = false;
		if (face && face->indices) {
			INDEX_LL	* indI = face->indices;
			while (indI) {
				INDEX_LL	* indII = indI->next;
				while (indII && indII->next) {
					if (indI->index == indII->index) {
						//
						//	Found inner opening
						//		split up faces
						//		decide what is the outer face
						//
						INDEX_LL	* polygonI = face->indices,
									* polygonII = new INDEX_LL;
						polygonII->index = indI->index;
						polygonII->next = indI->next;

						indI->next = indII->next;
						indII->next = nullptr;

						if (polygonI->next == nullptr || polygonI->next->next == nullptr) {
							face->indices = polygonII;
						}
						else if (polygonII->next == nullptr || polygonII->next->next == nullptr) {
							face->indices = polygonI;
						}
						else {
							double	polygonI_size = GetMaxSize(polygonI, vertexArray),
									polygonII_size = GetMaxSize(polygonII, vertexArray);

							FA__CE	* opening = new FA__CE;
							opening->plane.a = face->plane.a;
							opening->plane.b = face->plane.b;
							opening->plane.c = face->plane.c;
							opening->plane.d = face->plane.d;
							opening->openings = face->openings;

							if (polygonI_size > polygonII_size) {
								assert(face->indices == polygonI);
								opening->indices = polygonII;
							}
							else {
								face->indices = polygonII;
								opening->indices = polygonI;
							}

							face->openings = opening;
						}

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
			//
			//	Now the same for openings
			//
			FA__CE	* face = faces[i]->openings;
			bool	found = false;
			while (face) {
				if (face  &&  face->indices) {
					INDEX_LL	* indI = face->indices;
					while (indI) {
						INDEX_LL	* indII = indI->next;
						while (indII  &&  indII->next) {
							if (indI->index == indII->index) {
								//
								//	Found inner opening
								//		split up faces
								//		decide what is the outer face
								//
								INDEX_LL	* polygonI = face->indices,
											* polygonII = new INDEX_LL;
								polygonII->index = indI->index;
								polygonII->next = indI->next;

								indI->next = indII->next;
								indII->next = nullptr;

								if (polygonI->next == nullptr || polygonI->next->next == nullptr) {
									face->indices = polygonII;
								}
								else if (polygonII->next == nullptr || polygonII->next->next == nullptr) {
									face->indices = polygonI;
								}
								else {
									FA__CE	* opening = new FA__CE;
									opening->plane.a = face->plane.a;
									opening->plane.b = face->plane.b;
									opening->plane.c = face->plane.c;
									opening->plane.d = face->plane.d;
									opening->openings = face->openings;

									face->indices = polygonII;
									opening->indices = polygonI;

									face->openings = opening;
								}

								indI = 0;
								indII = 0;

								found = true;
							} else {
								indII = indII->next;
							}
						}

						if (indI) {
							indI = indI->next;
						}
					}
				}

				face = face->openings;
			}
		}

		i++;
	}

	/*
	//
	//	Third algorithm, check for 3 points on 1 line
	//
	i = 0;
	while (i < facesCnt) {
		double	epsilon = 0.0000001;
		VECTOR	vecI, vecII, vecIII,
				directionI, directionII;
		FACE	* face = faces[i];
		bool	found = false;
		while (face) {
			bool	updated = false;
			if (face  &&  face->indices  &&  face->indices->next) {
				INDEX_LL	* indPrev = face->indices,
							* indCurrent = face->indices,
							* indNext = indCurrent->next;
				while (indPrev  &&  indPrev->next  &&  indPrev->next->next) {
					indPrev = indPrev->next;
				}
				assert(indPrev->next->index == indCurrent->index && indPrev->next->next == nullptr);
				vecI.x = vertexArray[3 * indPrev->index + 0];
				vecI.y = vertexArray[3 * indPrev->index + 1];
				vecI.z = vertexArray[3 * indPrev->index + 2];
				vecII.x = vertexArray[3 * indCurrent->index + 0];
				vecII.y = vertexArray[3 * indCurrent->index + 1];
				vecII.z = vertexArray[3 * indCurrent->index + 2];
				vecIII.x = vertexArray[3 * indNext->index + 0];
				vecIII.y = vertexArray[3 * indNext->index + 1];
				vecIII.z = vertexArray[3 * indNext->index + 2];
				directionI.x = vecII.x - vecI.x;
				directionI.y = vecII.y - vecI.y;
				directionI.z = vecII.z - vecI.z;
				Vec3Normalize(&directionI);
				directionII.x = vecIII.x - vecII.x;
				directionII.y = vecIII.y - vecII.y;
				directionII.z = vecIII.z - vecII.z;
				Vec3Normalize(&directionII);
				if ( ( (std::fabs(directionI.x - directionII.x) < epsilon) &&
					   (std::fabs(directionI.y - directionII.y) < epsilon) &&
					   (std::fabs(directionI.z - directionII.z) < epsilon) ) ||
					   ( (std::fabs(directionI.x + directionII.x) < epsilon) &&
						 (std::fabs(directionI.y + directionII.y) < epsilon) &&
						 (std::fabs(directionI.z + directionII.z) < epsilon) ) ) {
					assert(indPrev  &&  indPrev->next  &&  indPrev->next->next == nullptr);
					assert(indPrev->next->index == indCurrent->index);
					indPrev->next = nullptr;
					assert(indCurrent == face->indices);
					indCurrent->index = indPrev->index;

					updated = true;
				} else {
					while (indNext->next) {
						indPrev = indCurrent;
						indCurrent = indNext;
						indNext = indNext->next;

						vecI.x = vertexArray[3 * indPrev->index + 0];
						vecI.y = vertexArray[3 * indPrev->index + 1];
						vecI.z = vertexArray[3 * indPrev->index + 2];
						vecII.x = vertexArray[3 * indCurrent->index + 0];
						vecII.y = vertexArray[3 * indCurrent->index + 1];
						vecII.z = vertexArray[3 * indCurrent->index + 2];
						vecIII.x = vertexArray[3 * indNext->index + 0];
						vecIII.y = vertexArray[3 * indNext->index + 1];
						vecIII.z = vertexArray[3 * indNext->index + 2];
						directionI.x = vecII.x - vecI.x;
						directionI.y = vecII.y - vecI.y;
						directionI.z = vecII.z - vecI.z;
						Vec3Normalize(&directionI);
						directionII.x = vecIII.x - vecII.x;
						directionII.y = vecIII.y - vecII.y;
						directionII.z = vecIII.z - vecII.z;
						Vec3Normalize(&directionII);

						if ( ( (std::fabs(directionI.x - directionII.x) < epsilon) &&
							   (std::fabs(directionI.y - directionII.y) < epsilon) &&
							   (std::fabs(directionI.z - directionII.z) < epsilon) ) ||
							   ( (std::fabs(directionI.x + directionII.x) < epsilon) &&
								 (std::fabs(directionI.y + directionII.y) < epsilon) &&
								 (std::fabs(directionI.z + directionII.z) < epsilon) ) ) {
							assert(indPrev->next == indCurrent);
							assert(indCurrent->next == indNext);
							indPrev->next = indNext;
							indCurrent = indPrev;
						}
					}
					//...
				}
			}

			if (updated == false) {
				face = face->openings;
			}
		}

		i++;
	}
	//	*/

	//
	//	Put them back
	//

	int64_t	m = 0;
	i = 0;
	while (i < facesCnt) {
		FA__CE	* face = faces[i];
		if (face  &&  face->indices) {
			if ((face->indices) &&
				(face->indices->next) &&
				(face->indices->next->next)) {
				INDEX_LL	* ref = face->indices;
				while (ref  &&  ref->next) {
					indexArray[m++] = ref->index;
					ref = ref->next;
				}
				assert(ref->index == face->indices->index);
				indexArray[m++] = -1;

				FA__CE	* opening = face->openings;
				while (opening) {
					if ((opening->indices) &&
						(opening->indices->next) &&
						(opening->indices->next->next)) {
						INDEX_LL	* ref = opening->indices;
						while (ref  &&  ref->next) {
							indexArray[m++] = ref->index;
							ref = ref->next;
						}
						assert(ref->index == opening->indices->index);
						indexArray[m++] = -2;
					}
					opening = opening->openings;
				}
			}
		}

		i++;
	}	//	*/

	assert(m <= indexArraySize);

	//
	//	...
	//

	int k = 0;
	while (k < noFaces) {
		if (faces[k]) {
			//	STILL TO ADD = REMOVE LINKED LIST OF INDICES
			delete faces[k];
		}
		k++;
	}
	delete[] faces;

	return	m;
}

void		__memcpy(
					int64_t		* indexArray,
					int64_t		offsetI,
					int64_t		offsetII,
					int64_t		noElements
				)
{
	int64_t	i = 0;
	while (i < noElements) {
		indexArray[offsetI + i] = indexArray[offsetII + i];
		i++;
	}
	assert(indexArray[offsetI + i] < 0);
}

void		MergeFaces__new(
					int64_t		rdfModel,
					int64_t		owlInstanceBoundaryRepresentationNormal,
					int64_t		owlInstanceBoundaryRepresentationInverted,
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					int64_t		vertexArraySize,			//	measured in number of elements
					int64_t		* indexArray,				//	we expect polygons ending with -1
					int64_t		indexArraySize				//	measured in number of elements
				)
{
	bool		inverted = true;// true;// false;

	if (vertexTextureArray == 0) {
//		return;	//!!!!!!!!!!!!!!!!
	}

//	20180531
//	vertexArraySize = MinimizeVertices(
//							vertexXYZArray,
//							vertexTextureArray,
//							vertexArraySize,
//							indexArray,
//							indexArraySize
//						);

	if (MERGE_FACES) {
		indexArraySize = MergeFaces(
								vertexXYZArray,
								vertexArraySize,
								indexArray,
								indexArraySize
							);
	}		

	int64_t	//owlClassBoundary Representation = GetClassByName(rdfModel,  Boundary Representation"),
			rdfPropertyIndices = GetPropertyByName(rdfModel, "indices"),
			rdfPropertyTextureCoordinates = GetPropertyByName(rdfModel, "textureCoordinates"),
			rdfPropertyVertices = GetPropertyByName(rdfModel, "vertices"),
			rdfPropertyConsistancyCheck = GetPropertyByName(rdfModel, "consistancyCheck"),
			rdfPropertyFraction = GetPropertyByName(rdfModel, "fraction"),
			rdfPropertyEpsilon = GetPropertyByName(rdfModel, "epsilon");

	if (indexArraySize > 30000) {
/////200180711///		assert(false);
/////200180711///		return;
	}

	double	epsilon = 0.002;
//	SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyEpsilon, &epsilon, 1);

	double	fraction = 0.7;
//	SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyFraction, &fraction, 1);

	int64_t	consistancyCheck = 103;
//	20180531
	/////200180711///	SetDataTypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyConsistancyCheck, &consistancyCheck, 1);

consistancyCheck = 256;		//	all faces as 1 conceptual face
SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyConsistancyCheck, &consistancyCheck, 1);

	SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyVertices, vertexXYZArray, 3 * vertexArraySize);
//	SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyVertices, vertexXYZArray, 3 * vertexArraySize);
	if (vertexTextureArray) {
		SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyTextureCoordinates, vertexTextureArray, 2 * vertexArraySize);
//		SetDataTypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyTextureCoordinates, vertexTextureArray, 2 * vertexArraySize);

		if (vertexArraySize == 164) {
//			assert(false);
		}
	}

	//
	//	Remove single points and single lines
	//
	int64_t	elements = 0, first = 0;
	for (int64_t mm = 0; mm < indexArraySize; mm++) {
		if (indexArray[mm] >= 0) {
			assert(indexArray[mm] < vertexArraySize);
			elements++;
		}
		else {
			assert(indexArray[mm] == -1 || indexArray[mm] == -2);
			if (elements < 3) {
				//
				//	we do not check if this is the parent and maybe children are ignored
				//
				mm -= elements;
				elements++;
				indexArraySize -= elements;
				
				int64_t n = mm;
				while (n < indexArraySize) {
					indexArray[n] = indexArray[n + elements];
					n++;
				}
			}

			elements = 0;
			first = mm + 1;
		}
	}

	//
	// Check for consistency
	//		Remove repeating points directly after each other
	//		Point accurs twice
	//		3 points on 1 line
	//
	elements = 0;
	for (int64_t mm = 0; mm < indexArraySize; mm++) {
		if (indexArray[mm] >= 0) {
			assert(indexArray[mm] < vertexArraySize);
			if (elements) {
				if (indexArray[mm] == indexArray[mm - 1]) {
					__memcpy(indexArray, mm - 1, mm, (indexArraySize - mm));
					indexArraySize--;
					elements--;
					mm--;
				}
			}
			elements++;
		}
		else {
			assert(indexArray[mm] == -1 || indexArray[mm] == -2);
			elements = 0;
		}
	}

	elements = 0;
	for (int64_t mm = 0; mm < indexArraySize; mm++) {
		if (indexArray[mm] >= 0) {
			assert(indexArray[mm] < vertexArraySize);
			elements++;
		}
		else {
			assert(indexArray[mm] == -1 || indexArray[mm] == -2);
			int64_t	first = mm - elements,
					last = mm - 1;
			if (indexArray[first] == indexArray[last]) {
				__memcpy(indexArray, mm - 1, mm, indexArraySize - mm);
				indexArraySize--;
				mm--;
			}
			elements = 0;
		}
	}

	for (int64_t mm = 0; mm < indexArraySize; mm++) {
		if (indexArray[mm] >= 0) {
			assert(indexArray[mm] < vertexArraySize);
			elements++;
		}
		else {
			assert(indexArray[mm] == -1 || indexArray[mm] == -2);
			int64_t first = mm - elements,
					last = mm - 1;
//			assert(indexArray[first] == indexArray[last]);
			int64_t i = first;
			while (i <= last) {
				int64_t j = i + 1;
				while (j <= last) {
//					assert(indexArray[i] != indexArray[j]);
					j++;
				}
				i++;
			}

			elements = 0;
			//...
		}
	}

	if (DUPLICATE_FACES == false && inverted == false) {
		SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyIndices, indexArray, indexArraySize);
	}
	else {
		int64_t	* extendedIndexArray = new int64_t[2 * (int_t) indexArraySize];
		elements = 0;
		first = 0;
		for (int64_t mm = 0; mm < indexArraySize; mm++) {
			extendedIndexArray[mm + 0] = indexArray[mm];
			extendedIndexArray[mm + indexArraySize] = indexArray[mm];
			if (indexArray[mm] >= 0) {
				assert(indexArray[mm] < vertexArraySize);
				elements++;
			}
			else {
				assert(indexArray[mm] == -1 || indexArray[mm] == -2);
//				assert(elements >= 3);

				int64_t	last = mm - 1;
				while (first < last) {
					int64_t	tmp = extendedIndexArray[first];
					extendedIndexArray[first] = extendedIndexArray[last];
					extendedIndexArray[last] = tmp;
					assert(extendedIndexArray[first] >= 0 && extendedIndexArray[first] < vertexArraySize);
					assert(extendedIndexArray[last] >= 0 && extendedIndexArray[last] < vertexArraySize);
					first++;
					last--;
				}

				elements = 0;
				first = mm + 1;
			}
		}
		assert(elements == 0);


		if (inverted) {
//			SetDatatypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyIndices, &extendedIndexArray[0],				indexArraySize);
			SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal,   rdfPropertyIndices, &extendedIndexArray[indexArraySize], indexArraySize);
		}
		else {
//			SetDatatypeProperty(owlInstanceBoundaryRepresentationInverted, rdfPropertyIndices, &extendedIndexArray[0], indexArraySize * 2);
			SetDatatypeProperty(owlInstanceBoundaryRepresentationNormal, rdfPropertyIndices, &extendedIndexArray[0], indexArraySize * 2);
		}
	}
}
