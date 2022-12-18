#ifndef __RDF_LTD__PG_DAE_OPTIMIZE_3D_H
#define __RDF_LTD__PG_DAE_OPTIMIZE_3D_H


#include "pg_dae_library3D.h"


void		MergeFaces__new(
					int64_t		rdfModel,
					int64_t		owlInstanceBoundaryRepresentation,
					int64_t		owlInstanceBoundaryRepresentationInverted,
					double		* vertexXYZArray,			//	we expect now (X, Y, Z) coordinates
					double		* vertexTextureArray,		//	we expect now (S, T) coordinates
					int64_t		vertexArraySize,			//	measured in number of elements
					int64_t		* indexArray,				//	we expect polygons ending with -1
					int64_t		indexArraySize				//	measured in number of elements
				);

//void		MergeFaces(
//					int64_t		owlInstance,
//					int64_t		rdfProperty,
//					int64_t		* indexArray,
//					int64_t		indexArraySize,
//					double		* vert
//				);


#endif
