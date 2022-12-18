#pragma once



#include "library3D.h"



void		MergeFaces__new(
					__int64		owlInstanceBoundaryRepresentation,
					__int64		owlInstanceBoundaryRepresentationInverted,
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					__int64		vertexArraySize,			//	measured in number of elements
					__int64		* indexArray,				//	we expect polygons ending with -1
					__int64		indexArraySize				//	measured in number of elements
				);

//void		MergeFaces(
//					__int64		owlInstance,
//					__int64		rdfProperty,
//					__int64		* indexArray,
//					__int64		indexArraySize,
//					double		* vert
//				);
