
#include "stdafx.h"
#include "pg_dae_core_sub.h"
#include "pg_dae_xmlParsing.h"
#include "pg_dae_material.h"
#include "pg_dae_image.h"
#include "pg_dae_optimize3D.h"

#include <cmath>
#include <stdio.h>
#include <string.h>

#include <assert.h>

//#include "dae2bin.h"

#include	"../../include/engine.h"


#define	TYPE__UNKNOWN		1001
#define	TYPE__XYZ			1101
#define	TYPE__ST			1102
#define	TYPE__RGB			1103
#define	TYPE__UV			1104

#define	TYPE__VERTEX		1201
#define	TYPE__NORMAL		1202
#define	TYPE__TEXCOORD		1203



double	glOffset__X = 0,
		glOffset__Y = 0,
		glOffset__Z = 0;

bool	isProcessingFIRST = true;

bool	DUPLICATE_FACES = false,
		MERGE_FACES = false;

extern	CO__LOR		* colors;
extern	MA__TERIAL	* materials;
extern	IMAGE		* images;
extern	int			listIndex;
extern	STRUCT_KEY	* firstKey;



bool	GET_INVERTED = false;

MA__TRIX	myMatrix;

int64_t	noElements = 0;
int64_t	//rdfModel = OpenModel(0),
		* globalObjectsNormal = nullptr, * globalObjectsInverted = nullptr;

//struct	MESH_INSTANCE
//{
//	int64_t				rdfInstance;
//	bool				texture;
//
//	char				* material;
//
//	MESH_INSTANCE		* next;
//};

struct  STRUCT_UNIT
{
	double				factor;
};

struct	MESH_INSTANCES
{
	int64_t				rdfInstanceNormal;
	int64_t				rdfInstanceInverted;
	bool				texture;

	char				* material;

	MESH_INSTANCES		* next;
};

struct	SOURCE;

struct	SOURCES
{
	SOURCE				* source;

	SOURCES				* next;
};

struct	MESH
{
	char				* id;

	MESH_INSTANCES		* instances;

	SOURCES				* sources;

	MESH				* next;
};

struct	NODE_REF
{
	char				* id;
	STRUCT_ITEM			* items;

	NODE_REF			* next;
};

NODE_REF	* nodeRefs = nullptr;

void	SetNodeRef(NODE_REF * nodeRef)
{
	nodeRef->next = nodeRefs;
	nodeRefs = nodeRef;
}

MESH	* meshes = nullptr;


void	DEBUG__checkMaterialPresent(
				int64_t	model,
				int64_t	* objects,
				int64_t	nodeCount
			)
{
	//
	//	Check if every node has attached a material
	//
	int64_t	owlClassTransformation = GetClassByName(model, "Transformation"),
	rdfPropertyMaterial = GetPropertyByName(model, "material");
	for (int i = 0; i < nodeCount; i++) {
		if (GetInstanceClass(objects[i]) == owlClassTransformation) {
			//...
		} else {
			assert(false);
		}

		int64_t	* values = nullptr, card = 0;
		GetObjectProperty(objects[i], rdfPropertyMaterial, &values, &card);
		assert(card == 1);
	}
}

void	SetMesh(MESH * newMesh)
{
	MESH	* mesh = meshes;
	while  (mesh) {
		if	(equal(mesh->id, newMesh->id)) {
			assert(false);
		}
		mesh = mesh->next;
	}

	assert(newMesh->next == nullptr);
	newMesh->next = meshes;
	meshes = newMesh;
}

MESH	* GetMesh(char * id)
{
	MESH	* mesh = meshes;
	while  (mesh) {
		if	(equal(mesh->id, id)) {
			return	mesh;
		}
		mesh = mesh->next;
	}

	assert(false);
	return	nullptr;
}

struct	FLOAT_ARRAY
{
	double				* doubleArray;
	int					count;
};

struct	INT_ARRAY
{
	int					* intArray;
	int					count;
};

struct	ACCESSOR
{
	int					__type;
	int					__count;
	int					__stride;
};

struct	MATERIAL_INSTANCE
{
	MA__TERIAL			* material;
	char				* symbol;

	MATERIAL_INSTANCE	* next;
};

struct	TECHNIQUE_COMMON
{
	ACCESSOR			* accessor;
	MATERIAL_INSTANCE	* materialInstances;
};

struct	NODE
{
	int64_t				rdfInstance;
};

struct	INSTANCE_GEOMETRY
{
	MATERIAL_INSTANCE	* materialInstances;
	MESH				* mesh;
};

struct	SOURCE
{
	double				* __floatArray;
	int					__floatArrayCount;

	bool				adjusted;

	int					__type;
	char				* __id;

//	bool				scaled;
//	TECHNIQUE_COMMON	* techniqueCommon;
};

struct	__INPUT
{
	int					type;
	int					offset;
	int					set;
	char				* source;
	__INPUT				* next;
};

struct	__PH
{
	INT_ARRAY			* p;
	INT_ARRAY			* h;

	__PH				* next;
};

struct	POLYGON
{
	__INPUT				* inputs;
	int					count;
	char				* material;
	__PH				* ph;
};

struct	POLYLIST
{
	__INPUT				* inputs;
	int					count;
	char				* material;
	INT_ARRAY			* vcount;
	INT_ARRAY			* p;
};

bool	checkDouble(
				char	* text
			)
{
	if	(text) {
		int	i = 0;
		while  (text[i]) {
			if	( (text[i] >= '0'  &&  text[i] <= '9')  ||  text[i] == '.'  ||  text[i] == '-'  ||  text[i] == 'e') {
			} else {
				if (text[i] == 10) {
					i++;
					while (text[i] == ' ') { i++; }
					if (text[i] == 0) {
						return	true;
					}
				}
				else {
					while (text[i] == ' ') { i++; }
					if (text[i] == 0) {
						return	true;
					}
				}
				return	false;
			}
			i++;
		}
		return	true;
	}

	return	false;
}

bool	checkInteger(
				char	* text
			)
{
	if	(text) {
		int	i = 0;
		while (text[i]) {
			if ((text[i] >= '0' && text[i] <= '9')) {
			}
			else {
				if (text[i] == 10) {
					i++;
					while (text[i] == ' ') { i++; }
					if (text[i] == 0) {
						return	true;
					}
				}
				return	false;
			}
			i++;
		}
		return	true;
	}

	return	false;
}

int		findIndex(
				char	* text,
				char	str
			)
{
	int	i = 0;
	while (text[i]) {
		if (text[i] == str) {
			return	i;
		}
		i++;
	}

	return	-1;
}

int		findIndex(
				char	* text,
				char	strI,
				char	strII
			)
{
	int	i = 0;
	while (text[i]) {
		if (text[i] == strI) {
			return	i;
		}
		if (text[i] == strII) {
			return	i;
		}
		i++;
	}

	return	-1;
}

double	* GetDoubles(
				char	* value,
				int		count
			)
{
	double	* doubles = new double[count];

	int	i = 0;
	while (i < count - 1) {
		int index = findIndex(value, ' ', 10);
		while (index == 0) {
			value = &value[1];
			index = findIndex(value, ' ', 10);
		}
		if (index == 0) {
			assert(false);
			value = &value[1];
			index = findIndex(value, ' ', 10);
		}
		assert(index > 0);
		value[index] = 0;

		assert(checkDouble(value));
		doubles[i] = atof(value);

		value = &value[index + 1];
		i++;
	}
	assert(checkDouble(value));
	doubles[i] = atof(value);

	return	doubles;
}

int	GetCnt(STRUCT_ITEM * item)
{
	char	* value = item->value;

	int index = findIndex(value, ' ');
	if (index == 0) {
		assert(false);
		value = &value[1];
		index = findIndex(value, ' ');
	}

	int	cnt = 1;
	while (index > 0) {
		value = &value[index + 1];
		cnt++;

		index = findIndex(value, ' ');
		if (index == 0) {
			assert(false);
			value = &value[1];
			index = findIndex(value, ' ');
		}
	}

	return	cnt;
}

int	* GetIntegers(
				char	* value,
				int		count
			)
{
	int	* integers = new int[count];

	int	i = 0;
	while  (i < count - 1) {
		int index = findIndex(value, ' ');
		if	(index == 0) {
//			assert(false);
			value = &value[1];
			index = findIndex(value, ' ');
		}
//		assert(index > 0);
		value[index] = 0;

		assert(checkInteger(value));
		integers[i] = atoi(value);

		value = &value[index + 1];
		i++;
	}

	int index = findIndex(value, ' ');
	if	(index > 0) {
		if	(value[index + 1] == 0) {
			value[index] = 0;
		} else {
			assert(false);
		}
	}

/////20180711///	assert(checkInteger(value));
	integers[i] = atoi(value);

	return	integers;
}

FLOAT_ARRAY	* parseFloatArray(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "float_array")) {
		assert(item->child == 0);
		assert(item->attr->key == GetKey((char*) "id"));
		assert(item->attr->next->key == GetKey((char*) "count"));

		FLOAT_ARRAY	* floatArray = new FLOAT_ARRAY;

		floatArray->count = atoi(item->attr->next->value);
		if	(floatArray->count) {
			assert(item->value);
			floatArray->doubleArray = GetDoubles(item->value, floatArray->count);
		} else {
			floatArray->doubleArray = nullptr;
		}
		return	floatArray;
	}

	assert(false);
	return	nullptr;
}

ACCESSOR	* parseAccessor(STRUCT_ITEM * item)
{
	ACCESSOR	* accessor = new ACCESSOR;

	accessor->__type = TYPE__UNKNOWN;
	accessor->__count = 0;
	accessor->__stride = 0;

	STRUCT_ATTR	* attr = item->attr;

	while  (attr) {
		if	(attr->key == GetKey((char*) "count")) {
			assert(accessor->__count == 0);
			accessor->__count = atoi(attr->value);
		}
		if	(attr->key == GetKey((char*) "stride")) {
			assert(accessor->__stride == 0);
			accessor->__stride = atoi(attr->value);
		}
		attr = attr->next;
	}

//	assert(accessor->__count);
	assert(accessor->__stride);

	bool	hasX = false, hasA = false, hasB = false, hasY = false, hasZ = false, hasG = false, hasR = false, hasS = false, hasT = false, hasU = false, hasV = false;
	int		cnt = 0;
	if	(item->key == GetKey((char*) "accessor")) {
		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if	(items->key == GetKey((char*) "param")) {
				switch  (cnt) {
					case  0:
						assert(items->attr  &&  items->attr->next  &&  items->attr->next->next == nullptr);
						if	(items->attr->key == GetKey((char*) "name")) {
							if	(equal(items->attr->value, (char*) "X")) {
								hasX = true;
							}
							else if (equal(items->attr->value, (char*) "S")) {
								hasS = true;
							}
							else if (equal(items->attr->value, (char*) "R")) {
								hasR = true;
							}
							else if (equal(items->attr->value, (char*) "U")) {
								hasU = true;
							}
							else {
								assert(false);
							}
						} else {
							assert(false);
						}
						assert(items->attr->next->key == GetKey((char*) "type")  &&  equal(items->attr->next->value, (char*) "float"));
						break;
					case  1:
						assert(items->attr  &&  items->attr->next  &&  items->attr->next->next == nullptr);
						if	(items->attr->key == GetKey((char*) "name")) {
							if	(equal(items->attr->value, (char*) "Y")) {
								hasY = true;
							}
							else if (equal(items->attr->value, (char*) "T")) {
								hasT = true;
							}
							else if (equal(items->attr->value, (char*) "G")) {
								hasG = true;
							}
							else if (equal(items->attr->value, (char*) "V")) {
								hasV = true;
							}
							else {
								assert(false);
							}
						} else {
							assert(false);
						}
						assert(items->attr->next->key == GetKey((char*) "type")  &&  equal(items->attr->next->value, (char*) "float"));
						break;
					case  2:
						assert(items->attr  &&  items->attr->next  &&  items->attr->next->next == nullptr);
						if	(items->attr->key == GetKey((char*) "name")) {
							if (equal(items->attr->value, (char*) "Z")) {
								hasZ = true;
							}
							else if (equal(items->attr->value, (char*) "B")) {
								hasB = true;
							}
							else {
								assert(false);
							}
						} else {
							assert(false);
						}
						assert(items->attr->next->key == GetKey((char*) "type")  &&  equal(items->attr->next->value, (char*) "float"));
						break;
					case  3:
						assert(items->attr  &&  items->attr->next  &&  items->attr->next->next == nullptr);
						if	(items->attr->key == GetKey((char*) "name")) {
							if (equal(items->attr->value, (char*) "A")) {
								hasA = true;
							}
							else {
								assert(false);
							}
						} else {
							assert(false);
						}
						assert(items->attr->next->key == GetKey((char*) "type")  &&  equal(items->attr->next->value, (char*) "float"));
						break;
					default:
						assert(false);
						break;
				}
				cnt++;
			} else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}

	if (hasX  &&  hasY  &&  hasZ) {
		accessor->__type = TYPE__XYZ;
		hasX = false;
		hasY = false;
		hasZ = false;
	}
	else if (hasS  &&  hasT) {
		accessor->__type = TYPE__ST;
		hasS = false;
		hasT = false;
	}
	else if (hasU  &&  hasV) {
		accessor->__type = TYPE__UV;
		hasS = false;
		hasT = false;
	}
	else if (hasR  &&  hasG  &&  hasB) {
		accessor->__type = TYPE__RGB;
		hasR = false;
		hasG = false;
		hasB = false;
	}
	else {
		assert(false);
	}

	assert(!hasX  &&  !hasY  &&  !hasZ  &&  !hasS  &&  !hasT);

	return	accessor;
}

MA__TERIAL	* parseInstanceMaterial(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "instance_material")) {
		/////20180711///		assert(item->child == nullptr  &&  item->value == nullptr);
//		assert(item->attr  &&  item->attr->next  &&  item->attr->next->next == nullptr);
//		assert(item->attr->key == GetKey("symbol"));
//		assert(item->attr->next->key == GetKey("target")  &&  item->attr->next->value[0] == '#');
		STRUCT_ATTR	* attr = item->attr, * attrTarget = nullptr;
		while  (attr) {
			if	(attr->key == GetKey((char*) "target")) {
				attrTarget = attr;
			}
			attr = attr->next;
		}
		assert(attrTarget);

		if	(attrTarget  &&  attrTarget->value[0] == '#') {
			return	GetMaterial(&attrTarget->value[1]);
		}
	}

	assert(false);
	return	nullptr;
}

TECHNIQUE_COMMON	* parseTechniqueCommon(STRUCT_ITEM * item)
{
	TECHNIQUE_COMMON	* techniqueCommon = nullptr;
	if	(item->key == GetKey((char*) "technique_common")) {
		techniqueCommon = new TECHNIQUE_COMMON;

		techniqueCommon->accessor = nullptr;
		techniqueCommon->materialInstances = nullptr;

		MATERIAL_INSTANCE	** ppMaterialInstances = &techniqueCommon->materialInstances;

		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if	(items->key == GetKey((char*) "accessor")) {
				techniqueCommon->accessor = parseAccessor(items);
			} else if  (items->key == GetKey((char*) "instance_material")) {
				char		* symbol = nullptr;
				STRUCT_ATTR	* attr = items->attr;
				while (attr) {
					if (attr->key == GetKey((char*) "symbol")) {
						symbol = attr->value;
					} else if (attr->key == GetKey((char*) "target")) {
						//...
					} else {
						assert(false);
					}
					attr = attr->next;
				}

				assert(symbol);

				(* ppMaterialInstances) = new MATERIAL_INSTANCE;

				(* ppMaterialInstances)->material = parseInstanceMaterial(items);
				(* ppMaterialInstances)->symbol = symbol;
				(* ppMaterialInstances)->next = nullptr;

				ppMaterialInstances = &(* ppMaterialInstances)->next;
			} else {
				assert(false);
			}
			items = items->next;
		}
	} else {
		assert(false);
	}

	return	techniqueCommon;
}

INT_ARRAY	* parseVcount(STRUCT_ITEM * item, int count)
{
	INT_ARRAY	* intArray = new INT_ARRAY;

	intArray->intArray = nullptr;
	intArray->count = count;

	if	(item->key == GetKey((char*) "vcount")) {
		assert(item->child == nullptr);

		intArray->intArray = GetIntegers(item->value, intArray->count);
	}

	return	intArray;
}

INT_ARRAY	* parseP(STRUCT_ITEM * item, int count)
{
	INT_ARRAY	* intArray = new INT_ARRAY;

	intArray->intArray = nullptr;
	intArray->count = count;

	if	(item->key == GetKey((char*) "p")) {
		assert(item->child == nullptr);

		intArray->intArray = GetIntegers(item->value, intArray->count);
	}

	return	intArray;
}

INT_ARRAY	* parseH(STRUCT_ITEM * item, int count)
{
	INT_ARRAY	* intArray = new INT_ARRAY;

	intArray->intArray = nullptr;
	intArray->count = count;

	if (item->key == GetKey((char*) "h")) {
		assert(item->child == nullptr);

		intArray->intArray = GetIntegers(item->value, intArray->count);
	}

	return	intArray;
}

POLYGON		* parsePolygons(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "polygons")) {
		int			count = 0;
		char		* material = nullptr;
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "count")) {
				count = atoi(attr->value);
			} else if (attr->key == GetKey((char*) "material")) {
				material = attr->value;
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		assert(count  &&  material);

		POLYGON	* polygon = new POLYGON;
		polygon->count = count;
		polygon->material = material;
		polygon->inputs = nullptr;
		polygon->ph = nullptr;

		int			maxOffset = 0, phItems = 0;
		__INPUT		** ppInput = &polygon->inputs;

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "input")) {
				STRUCT_ATTR	* attr = items->attr, *attrSemantic = nullptr, *attrOffset = nullptr, *attrSource = nullptr;
				while (attr) {
					if (attr->key == GetKey((char*) "semantic")) {
						attrSemantic = attr;
					} else if (attr->key == GetKey((char*) "offset")) {
						attrOffset = attr;
					} else if (attr->key == GetKey((char*) "source")) {
						attrSource = attr;
					} else {
						assert(false);
					}
					attr = attr->next;
				}
				assert(attrSemantic  &&  attrOffset  &&  attrSource);

				(*ppInput) = new __INPUT;
				(*ppInput)->next = nullptr;
				(*ppInput)->source = attrSource->value;
				(*ppInput)->offset = atoi(attrOffset->value);
				(*ppInput)->type = TYPE__UNKNOWN;
				(*ppInput)->set = -1;

				if (equal(attrSemantic->value, (char*) "VERTEX")) {
					(*ppInput)->type = TYPE__VERTEX;
				} else if (equal(attrSemantic->value, (char*) "NORMAL")) {
					(*ppInput)->type = TYPE__NORMAL;
				} else if (equal(attrSemantic->value, (char*) "TEXCOORD")) {
					(*ppInput)->type = TYPE__TEXCOORD;
				} else {
					assert(false);
				}

				if (maxOffset == (*ppInput)->offset) {
					maxOffset++;
				} else {
					assert(maxOffset == (*ppInput)->offset + 1);
					assert((*ppInput)->set >= 0);
				}

				ppInput = &(*ppInput)->next;
			} else if (items->key == GetKey((char*) "ph")) {
				phItems++;
				//...
			} else {
				assert(false);
			}
			items = items->next;
		}

		assert(phItems == count);

		__PH	** ppPH = &polygon->ph;

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "input")) {
				//...
			} else if (items->key == GetKey((char*) "ph")) {
				assert(items->value == nullptr && items->attr == nullptr);

				INT_ARRAY	* p = nullptr, * h = nullptr;

				STRUCT_ITEM	* phChildren = items->child;
				while (phChildren) {
					if (phChildren->key == GetKey((char*) "p")) {
						assert(p == nullptr);
						p = parseP(phChildren, GetCnt(phChildren));
					} else if (phChildren->key == GetKey((char*) "h")) {
						assert(h == nullptr);
						h = parseH(phChildren, GetCnt(phChildren));
					} else {
						assert(false);
					}
					phChildren = phChildren->next;
				}

				assert(p  &&  h);

				(*ppPH) = new __PH;
				(*ppPH)->p = p;
				(*ppPH)->h = h;
				(*ppPH)->next = nullptr;
				ppPH = &(*ppPH)->next;
			} else {
				assert(false);
			}
			items = items->next;
		}

		return	polygon;
	}

	assert(false);
	return	nullptr;
}

POLYLIST	* parsePolylist(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "polylist")) {
		int			count = 0;
		char		* material = nullptr;
		STRUCT_ATTR	* attr = item->attr;
		while  (attr) {
			if	(attr->key == GetKey((char*) "count")) {
				count = atoi(attr->value);
			} else if (attr->key == GetKey((char*) "material")) {
				material = attr->value;
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		assert(count  &&  material);

		POLYLIST	* polylist = new POLYLIST;
		polylist->inputs = nullptr;
		polylist->count = count;
		polylist->material = material;
		polylist->p = nullptr;
		polylist->vcount = nullptr;

		int			maxOffset = 0;
		__INPUT		** ppInput = &polylist->inputs;
		INT_ARRAY	* vcount = nullptr;
		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if (items->key == GetKey((char*) "input")) {
				STRUCT_ATTR	* attr = items->attr, *attrSemantic = nullptr, *attrOffset = nullptr, *attrSource = nullptr, *attrSet = nullptr;
				while (attr) {
					if (attr->key == GetKey((char*) "semantic")) {
						attrSemantic = attr;
					} else if (attr->key == GetKey((char*) "offset")) {
						attrOffset = attr;
					} else if (attr->key == GetKey((char*) "source")) {
						attrSource = attr;
					} else if (attr->key == GetKey((char*) "set")) {
						attrSet = attr;
					} else {
						assert(false);
					}
					attr = attr->next;
				}
				assert(attrSemantic  &&  attrOffset  &&  attrSource);

				(*ppInput) = new __INPUT;
				(*ppInput)->next = nullptr;
				(*ppInput)->source = attrSource->value;
				(*ppInput)->offset = atoi(attrOffset->value);
				(*ppInput)->type = TYPE__UNKNOWN;
				(*ppInput)->set = -1;
				if (attrSet) {
					(*ppInput)->set = atoi(attrSet->value);
				}

				if (equal(attrSemantic->value, (char*) "VERTEX")) {
					(*ppInput)->type = TYPE__VERTEX;
				} else if (equal(attrSemantic->value, (char*) "NORMAL")) {
					(*ppInput)->type = TYPE__NORMAL;
				} else if (equal(attrSemantic->value, (char*) "TEXCOORD")) {
					(*ppInput)->type = TYPE__TEXCOORD;
				} else {
					assert(false);
				}

				if (maxOffset == (*ppInput)->offset) {
					maxOffset++;
					//					assert((*ppInput)->set == -1);
				} else {
					assert(maxOffset == (*ppInput)->offset + 1);
					assert((*ppInput)->set >= 0);
				}

				ppInput = &(*ppInput)->next;
			} else if (items->key == GetKey((char*) "vcount")) {
				vcount = parseVcount(items, count);
			} else if (items->key == GetKey((char*) "p")) {
				//...
			} else {
				assert(false);
			}
			items = items->next;
		}

		assert(vcount);

		INT_ARRAY	* p = nullptr;

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "input")) {
				//...
			}
			else if (items->key == GetKey((char*) "p")) {
				count = 0;
				int i = 0;
				while  (i < vcount->count) {
					count += vcount->intArray[i++];
				}
				p = parseP(items, count * maxOffset);
			}
			else if (items->key == GetKey((char*) "vcount")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		if	(p  &&  vcount) {
			polylist->vcount = vcount;
			polylist->p = p;

			return	polylist;
		}
	}

	assert(false);
	return	nullptr;
}

SOURCE	* parseSource(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "source")) {
		FLOAT_ARRAY			* floatArray = nullptr;
		TECHNIQUE_COMMON	* techniqueCommon = nullptr;
		POLYLIST			* polylist = nullptr;

		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if	(items->key == GetKey((char*) "float_array")) {
				assert(floatArray == nullptr);
				floatArray = parseFloatArray(items);
			}
			if	(items->key == GetKey((char*) "technique_common")) {
				techniqueCommon = parseTechniqueCommon(items);
			}
			if	(items->key == GetKey((char*) "vertices")) {
//				parseVertices(items);
				int I = 0;
			}
			if	(items->key == GetKey((char*) "polylist")) {
				polylist = parsePolylist(items);
			}
			items = items->next;
		}

		if	(floatArray  &&  techniqueCommon) {
			assert((techniqueCommon->accessor->__type == TYPE__XYZ  &&  techniqueCommon->accessor->__stride == 3) || (techniqueCommon->accessor->__type == TYPE__ST  &&  techniqueCommon->accessor->__stride == 2) || (techniqueCommon->accessor->__type == TYPE__UV  &&  techniqueCommon->accessor->__stride == 2) || (techniqueCommon->accessor->__type == TYPE__RGB && (techniqueCommon->accessor->__stride == 3 || techniqueCommon->accessor->__stride == 4)));
			assert(techniqueCommon->accessor->__count * techniqueCommon->accessor->__stride == floatArray->count);
			//...

			assert(polylist == nullptr);

			SOURCE	* source = new SOURCE;

//			source->techniqueCommon = techniqueCommon;
//			source->scaled = false;

//			assert(floatArray  &&  floatArray->count);
			assert(floatArray);
			assert(techniqueCommon  &&  techniqueCommon->accessor);
			assert(item->attr->key == GetKey((char*) "id"));

/////////////////			....->__id ... ...

			source->__floatArray = floatArray->doubleArray;
			source->__floatArrayCount = floatArray->count;

			source->adjusted = false;

			source->__id = item->attr->value;
			source->__type = techniqueCommon->accessor->__type;

			assert(source->__floatArrayCount == techniqueCommon->accessor->__count * techniqueCommon->accessor->__stride);

			return	source;
		} else {
			assert(false);
		}
	} else {
		assert(false);
	}

	return	nullptr;
}


/*
void	MergeFaces(int64_t owlInstance, int64_t rdfProperty, int64_t * indexArray, int64_t indexArraySize, double * vert)
{
REF		** ppRef = nullptr;
FACE	** faces = (FACE **) new int[indexArraySize];
int64_t	facesCnt = 0;

int i = 0;
bool	started = false;
while  (i < indexArraySize) {
	if	(indexArray[i] == -1) {
		assert(started);
		started = false;

		VECTOR	vec1, vec2, vec3;
		REF		* ref1 = faces[facesCnt]->ref, * ref2 = ref1->next, * ref3 = ref2->next;
		vec1.x = vert[3*ref1->index + 0];
		vec1.y = vert[3*ref1->index + 1];
		vec1.z = vert[3*ref1->index + 2];
		vec2.x = vert[3*ref2->index + 0];
		vec2.y = vert[3*ref2->index + 1];
		vec2.z = vert[3*ref2->index + 2];
		vec3.x = vert[3*ref3->index + 0];
		vec3.y = vert[3*ref3->index + 1];
		vec3.z = vert[3*ref3->index + 2];

		PlaneFromPoints(&faces[facesCnt]->plane, &vec1, &vec2, &vec3);

		(*ppRef) = new REF;
		(*ppRef)->index = faces[facesCnt]->ref->index;
		(*ppRef)->next = nullptr;

		facesCnt++;
	} else {
///		assert(indexArray[i] >= 0  &&  indexArray[i] < vertCnt);

		if	(started == false) {
			faces[facesCnt] = new FACE;
			faces[facesCnt]->plane.a = 0;
			faces[facesCnt]->plane.b = 0;
			faces[facesCnt]->plane.c = 1;
			faces[facesCnt]->plane.d = 0;
			faces[facesCnt]->ref = nullptr;//new REF;
			ppRef = &faces[facesCnt]->ref;
			started = true;
		}

		(*ppRef) = new REF;
		(*ppRef)->index = indexArray[i];
		(*ppRef)->next = nullptr;
		ppRef = &(*ppRef)->next;
	}
	i++;
}

//
//	Merge planes
//
i = 0;
while  (i < facesCnt) {
	int j = 0;
	while  (j < i) {
//		if	( (faces[j]->plane.a == faces[i]->plane.a)  &&
//			  (faces[j]->plane.b == faces[i]->plane.b)  &&
//			  (faces[j]->plane.c == faces[i]->plane.c)  &&
//			  (faces[j]->plane.d == faces[i]->plane.d) ) {
		if	( (std::fabs(faces[j]->plane.a - faces[i]->plane.a) < 0.0001)  &&
			  (std::fabs(faces[j]->plane.b - faces[i]->plane.b) < 0.0001)  &&
			  (std::fabs(faces[j]->plane.c - faces[i]->plane.c) < 0.0001)  &&
			  (std::fabs(faces[j]->plane.d - faces[i]->plane.d) < 0.0001) ) {
//		if	( (std::fabs(faces[j]->plane.a - faces[i]->plane.a) < 0.01)  &&
//			  (std::fabs(faces[j]->plane.b - faces[i]->plane.b) < 0.01)  &&
//			  (std::fabs(faces[j]->plane.c - faces[i]->plane.c) < 0.01)  &&
//			  (std::fabs(faces[j]->plane.d - faces[i]->plane.d) < 0.01) ) {
			REF	* refJ = faces[j]->ref;
			while  (refJ  && refJ->next  &&  faces[j]->ref) {
				int64_t	jInd_I = refJ->index, jInd_II = refJ->next->index;

				REF	* refI = faces[i]->ref;
				while  (refI  && refI->next) {
					int64_t	iInd_I = refI->index, iInd_II = refI->next->index;

					if	(iInd_I == jInd_II  &&  iInd_II == jInd_I) {
						//
						//	Merge FACES!!!
						//
						REF	**ppMyRef = &faces[i]->ref;

						int64_t	* list = new int64_t[indexArraySize], k = 0;
						refI = refI->next;
						while  (refI  &&  refI->next) {
							list[k++] = refI->index;
							refI = refI->next;
						}
						assert(refI->index == faces[i]->ref->index);
						refI = faces[i]->ref;
						while  (refI  &&  refI->index != iInd_I) {
							list[k++] = refI->index;
							refI = refI->next;
						}
						assert(refI->index == iInd_I);

						refJ = refJ->next;
						while  (refJ  &&  refJ->next) {
							list[k++] = refJ->index;
							refJ = refJ->next;
						}
						assert(refJ->index == faces[j]->ref->index);
						refJ = faces[j]->ref;
						while  (refJ  &&  refJ->index != jInd_I) {
							list[k++] = refJ->index;
							refJ = refJ->next;
						}
						assert(refJ->index == jInd_I);


		int64_t	n = 0;
		ppRef = &faces[i]->ref;
		while  (n < k) {
			(*ppRef) = new REF;
			(*ppRef)->index = list[n];
			(*ppRef)->next = nullptr;
			ppRef = &(*ppRef)->next;
			n++;
		}
		(*ppRef) = new REF;
		(*ppRef)->index = list[0];
		(*ppRef)->next = nullptr;



						delete[]  list;

				//		...
				//		(*ppMyRef) = new REF;
				//		(*ppMyRef)->index = ..
				//		(*ppMyRef)->next = nullptr;
				//		ppMyRef = &(*ppMyRef)->next;

						faces[j]->ref = nullptr;
						refI = nullptr;
					} else {
						refI = refI->next;
					}
				}

				refJ = refJ->next;
			}
		}
		j++;
	}
	i++;
}
* /

//
//	Put them back
//

//int64_t	* indices = new int64_t[indexArraySize];
int64_t	* indices = new int64_t[2*indexArraySize];
int64_t	m = 0;
i = 0;
while  (i < facesCnt) {
	FACE	* face = faces[i];
	if	(face->ref) {
		REF		* ref = face->ref;
		while  (ref  &&  ref->next) {
			indices[m++] = ref->index;
			ref = ref->next;
		}
		assert(ref->index == face->ref->index);
		indices[m++] = -1;
	}

	i++;
}

//
//	Other side, need a check to see if we want this
//
i = 0;
while  (i < facesCnt) {
	FACE	* face = faces[i];
	if	(face->ref) {
		REF		* ref = face->ref;
		int start = m;
		while  (ref  &&  ref->next) {
			indices[m++] = ref->index;
			ref = ref->next;
		}
		int end = m - 1;
		while  (start < end) {
			int tmp = indices[start];
			indices[start] = indices[end];
			indices[end] = tmp;
			start++;
			end--;
		}
		assert(ref->index == face->ref->index);
		indices[m++] = -1;
	}

	i++;
}



	SetDataTypeProperty(owlInstance, rdfProperty, indices, m);
}	//	*/

int ttl = 0;
void	parseMesh(
				int64_t		model,
				STRUCT_ITEM * item,
				MESH		* mesh
			)
{
	MESH_INSTANCES	** ppMeshInstance = &mesh->instances;
	assert(ppMeshInstance  &&  (*ppMeshInstance) == nullptr);

	bool	hasTriangulationWithTexture = false;
	if (item->key == GetKey((char*) "mesh")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "triangles")) {
				STRUCT_ITEM	* children = items->child;
				while (children) {
					assert(children->child == nullptr);
					if (children->key == GetKey((char*) "input")) {
						STRUCT_ATTR	* childAttr = children->attr;
						while (childAttr) {
							if (childAttr->key == GetKey((char*) "semantic")) {
								if (equal(childAttr->value, (char*) "TEXCOORD")) {
									hasTriangulationWithTexture = true;
								}
							}
							childAttr = childAttr->next;
						}
					}
					children = children->next;
				}
			}
			items = items->next;
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "source")) {
				SOURCES	* sources = new SOURCES;
				sources->source = parseSource(items);
				sources->next = mesh->sources;
				mesh->sources = sources;
			}
			else if (items->key == GetKey((char*) "vertices")) {
				STRUCT_ITEM	* subItem = items->child;
				assert(subItem->attr->key == GetKey((char*) "semantic"));
				assert(subItem->attr->next->key == GetKey((char*) "source"));

				SOURCE	* verticesSource = nullptr;
				char	* valueSource = subItem->attr->next->value;
				SOURCES	* sources = mesh->sources;
				while  (sources) {
					assert(valueSource[0] == '#');
					if	(equal(sources->source->__id, &valueSource[1])) {
						assert(verticesSource == nullptr);
						verticesSource = sources->source;
					}
					sources = sources->next;
				}
				assert(verticesSource);

				SOURCES	* newSources = new SOURCES;
				newSources->source = new SOURCE;
				newSources->source->__floatArray		= verticesSource->__floatArray;
				newSources->source->__floatArrayCount	= verticesSource->__floatArrayCount;
				newSources->source->__id				= items->attr->value;
				newSources->source->adjusted			= verticesSource->adjusted;
				newSources->source->__type				= verticesSource->__type;
				newSources->next = mesh->sources;
				mesh->sources = newSources;
			}
			else if (items->key == GetKey((char*) "polylist")) {
//				POLYLIST	* polylist = nullptr;

//				int			count = 0;
//				char		* matTXT = nullptr;
//				STRUCT_ATTR	* attr = items->attr;
//				while (attr) {
//					if (attr->key == GetKey("count")) {
//						assert(checkInteger(attr->value));
//						count = atoi(attr->value);
//					}
//					else if (attr->key == GetKey("material")) {
//						matTXT = attr->value;
//					}
//					else {
//						assert(false);
//					}
//					attr = attr->next;
//				}

//				assert(matTXT);

				POLYLIST	* polylist = parsePolylist(items);

				if (true) {//(u != 2  &&  u != 17  &&  u != 18  &&  u != 19  &&  u != 21  &&  u != 30  &&  u != 31  &&  u != 34  &&  u != 35  &&  u != 36  &&  u != 38  &&  u != 41  &&  u != 54  &&  u != 62  &&  u != 64) {
					int64_t	owlClassBoundaryRepresentation = GetClassByName(model, "BoundaryRepresentation");// ,
						//	rdfPropertyIndices = GetPropertyByName(rdfModel, "indices"),
//							rdfPropertyTextureCoordinates = GetPropertyByName(rdfModel, "textureCoordinates"),
						//	rdfPropertyVertices = GetPropertyByName(rdfModel, "vertices");//,

					globalObjectsNormal[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);
					globalObjectsInverted[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);

					(*ppMeshInstance) = new MESH_INSTANCES;

					(*ppMeshInstance)->rdfInstanceNormal = globalObjectsNormal[noElements];
					(*ppMeshInstance)->rdfInstanceInverted = globalObjectsInverted[noElements];
					(*ppMeshInstance)->texture = false;
					(*ppMeshInstance)->next = nullptr;
					(*ppMeshInstance)->material = polylist->material;

					ppMeshInstance = &(*ppMeshInstance)->next;

			//		int		maxOffset = 0;

					SOURCE	* verticesSource = nullptr, * texCoordSource = nullptr;
					int		verticesOffset = -1, texCoordOffset = -1,
							offsetCnt = 0;

					__INPUT	* input = polylist->inputs;
					while (input) {
						if (input->type == TYPE__VERTEX) {
							assert(verticesSource == 0);

							SOURCES	* sources = mesh->sources;
							while  (sources) {
								assert(input->source[0] == '#');
								if	(equal(sources->source->__id, &input->source[1])) {
									assert(verticesSource == nullptr);
									verticesSource = sources->source;
									verticesOffset = input->offset;
								}
								sources = sources->next;
							}

							assert(verticesSource  &&  verticesOffset >= 0);
						}
						else if (input->type == TYPE__TEXCOORD) {
							assert(texCoordSource == 0);

							SOURCES	* sources = mesh->sources;
							while  (sources) {
								assert(input->source[0] == '#');
								if	(equal(sources->source->__id, &input->source[1])) {
									assert(texCoordSource == nullptr);
									texCoordSource = sources->source;
									texCoordOffset = input->offset;
								}
								sources = sources->next;
							}

							assert(texCoordSource  &&  texCoordOffset >= 0);
						}
						
						if (input->offset == offsetCnt) {
							offsetCnt++;
						}
						else {
							assert(false);
						}

						input = input->next;
					}
					assert(verticesSource);

					if	(texCoordSource) {
						int		length = (polylist->p->count / offsetCnt) + polylist->vcount->count;
						int64_t	* indexArray = new int64_t[length * 2];
						double	* verticesArray = new double[3 * polylist->p->count];
						double	* texCoordArray = new double[2 * polylist->p->count];

						int k = 0, myOffset = 0, cntPP = 0;
						while  (k < polylist->vcount->count) {
							int size = polylist->vcount->intArray[k], n = 0;
							int cnt = myOffset + n;
							while  (n < size) {
								assert(myOffset + n >= 0  &&  2 * (myOffset + n) + 0 < polylist->p->count);
								int	vertexIndex = polylist->p->intArray[offsetCnt * (myOffset + n) + verticesOffset],
									texCoordIndex = polylist->p->intArray[offsetCnt * (myOffset + n) + texCoordOffset];

								verticesArray[3 * cnt + 0] = verticesSource->__floatArray[3 * vertexIndex + 0];
								verticesArray[3 * cnt + 1] = verticesSource->__floatArray[3 * vertexIndex + 1];
								verticesArray[3 * cnt + 2] = verticesSource->__floatArray[3 * vertexIndex + 2];
								assert(vertexIndex >= 0  &&  3 * vertexIndex + 2 < verticesSource->__floatArrayCount);
								texCoordArray[2 * cnt + 0] = 0 + texCoordSource->__floatArray[2 * texCoordIndex + 0];
								texCoordArray[2 * cnt + 1] = 1 - texCoordSource->__floatArray[2 * texCoordIndex + 1];
//								texCoordArray[2 * cnt + 1] = 0 + texCoordSource->__floatArray[2 * texCoordIndex + 1];
								assert(texCoordIndex >= 0 && 2 * texCoordIndex + 1 < texCoordSource->__floatArrayCount);

								indexArray[cntPP] = cnt;
//								indexArray[cntPP + length + (size - 1) - n - n] = cnt;
								assert(cnt + (size - 1) - n == myOffset + size - 1);
								cnt++;
								cntPP++;
								n++;
							}

							assert(cnt == size + myOffset  &&  n == size);

							indexArray[cntPP] = -1;
//indexArray[cntPP + length] = -1;
							cntPP++;
							myOffset += size;
							k++;
						}


						assert(offsetCnt * myOffset == polylist->p->count);

						assert(cntPP == (polylist->p->count / offsetCnt) + polylist->vcount->count  &&  cntPP == (myOffset) + polylist->vcount->count);

						MergeFaces__new(
								model,
								globalObjectsNormal[noElements],
								globalObjectsInverted[noElements],
								verticesArray,
								texCoordArray,
								cntPP - k,
								indexArray,
								cntPP
							);

						assert(myOffset * offsetCnt == polylist->p->count);
						assert(length == cntPP);

						delete[] indexArray;
						delete[] verticesArray;
						delete[] texCoordArray;
					}
					else {
						int64_t vertexCnt = verticesSource->__floatArrayCount / 3;
						double	* verticesArray = new double[3 * (int_t) vertexCnt];
						memcpy(verticesArray, verticesSource->__floatArray, (size_t) vertexCnt * 3 * sizeof(double));

						int noVertices = verticesSource->__floatArrayCount / 3;
						assert(3 * noVertices == verticesSource->__floatArrayCount);

						int64_t	* indexArray = new int64_t[polylist->p->count + polylist->vcount->count];
			
						int64_t	indexArraySize = 0;
						int64_t k = 0, myOffset = 0;
						while  (k < polylist->vcount->count) {
							int64_t size = polylist->vcount->intArray[k], n = 0, first = indexArraySize;
							while  (n < size) {
//								assert(myOffset + n >= 0 && 2 * (myOffset + n) + 0 < polylist->p->count);
								assert(myOffset + n >= 0 && offsetCnt * (myOffset + n) + 0 < polylist->p->count);
								indexArray[indexArraySize] = polylist->p->intArray[offsetCnt * (myOffset + n) + verticesOffset];
								assert(indexArray[indexArraySize] >= 0  &&  indexArray[indexArraySize] < noVertices);
								indexArraySize++;
								n++;
							}

							indexArray[indexArraySize++] = -1;
							myOffset += size;
							k++;
						}
//						assert(2 * myOffset == polylist->p->count);
						assert(offsetCnt * myOffset == polylist->p->count);
//						assert(indexArraySize == (polylist->p->count / 2) + polylist->vcount->count);
						assert(indexArraySize == (polylist->p->count / offsetCnt) + polylist->vcount->count);

						MergeFaces__new(
								model,
								globalObjectsNormal[noElements],
								globalObjectsInverted[noElements],
								verticesArray,
								nullptr,
								vertexCnt,
								indexArray,
								indexArraySize
							);
					}
				} else {
					globalObjectsNormal[noElements] = 0;
					globalObjectsInverted[noElements] = 0;
				}

				noElements++;

				assert(noElements < 49999);
			}
			else if (items->key == GetKey((char*) "triangles")) {
				//
				//	Example:
				//
				//	        <triangles material="Material7" count="178">
				//				<input semantic = "VERTEX" source = "#Geometry29-vertices" offset = "0" / >
				//				<input semantic = "NORMAL" source = "#Geometry29-normal" offset = "1" / >
				//				<input semantic = "TEXCOORD" source = "#Geometry29-texcoord" offset = "2" / >
				//				<p>21 0 22 2 0 3 3 0 0 .... < / p>
				//			</triangles>
				//
				//
				int			ATTR__count__asInteger = 0;
				char		* ATTR__material__asString = nullptr;
				{
					//
					//	        <triangles material="Material7" count="178">
					//
					STRUCT_ATTR	* attr = items->attr;
					while (attr) {
						if (attr->key == GetKey((char*) "count")) {
							assert(checkInteger(attr->value));
							ATTR__count__asInteger = atoi(attr->value);
						}
						else if (attr->key == GetKey((char*) "material")) {
							ATTR__material__asString = attr->value;
						}
						else {
							assert(false);
						}
						attr = attr->next;
					}
				}

				assert(ATTR__count__asInteger && ATTR__material__asString);

				int64_t	owlClassBoundaryRepresentation = GetClassByName(model, "BoundaryRepresentation");
//						rdfPropertyIndices = GetPropertyByName(rdfModel, "ind ices"),
//						rdfPropertyTextureCoordinates = GetPropertyByName(rdfModel, "textureCoordinates"),
//						rdfPropertyVertices = GetPropertyByName(rdfModel, "vertices");

				globalObjectsNormal[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);
				globalObjectsInverted[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);

				(*ppMeshInstance) = new MESH_INSTANCES;

				(*ppMeshInstance)->rdfInstanceNormal = globalObjectsNormal[noElements];
				(*ppMeshInstance)->rdfInstanceInverted = globalObjectsInverted[noElements];
				(*ppMeshInstance)->texture = false;
				(*ppMeshInstance)->next = nullptr;
				(*ppMeshInstance)->material = ATTR__material__asString;

				MESH_INSTANCES	* mI = (*ppMeshInstance);

				ppMeshInstance = &(*ppMeshInstance)->next;

		//		assert(items->next == nullptr);
				assert(items->child  &&  items->child->child == nullptr);
				assert(items->child->next  &&  items->child->next->child == nullptr);
				////		assert(items->child->next->next == nullptr);


				bool	hasVERTEX = false, hasNORMAL = false, hasTEXCOORD = false;
				SOURCE	* VERTEX__source = nullptr,
						* NORMAL__source = nullptr,
						* TEXCOORD__source = nullptr;
				int		VERTEX__offset = 0, NORMAL__offset = 0, TEXCOORD__offset = 0;

				double	* verticesArray = nullptr;
				int		verticesArrayCard = 0;

				double	* texturesArray = nullptr;
				int		texturesArrayCard = 0;

				int		* pArray = nullptr;
				int		pArrayCard = 0, pBlockSize = 0;

				{
					//
					//				<input semantic = "VERTEX" source = "#Geometry29-vertices" offset = "0" / >
					//				<input semantic = "NORMAL" source = "#Geometry29-normal" offset = "1" / >
					//				<input semantic = "TEXCOORD" source = "#Geometry29-texcoord" offset = "2" / >
					//				<p>21 0 22 2 0 3 3 0 0 .... < / p>
					//

					int	maxOffset = 0;

					STRUCT_ITEM	* children = items->child;
					while (children) {
						assert(children->child == nullptr);
						if (children->key == GetKey((char*) "input")) {
							SOURCE	* source = nullptr;
							bool	isVERTEX = false, isNORMAL = false, isTEXCOORD = false, isCOLOR = false;
							int		offset = -1;
							STRUCT_ATTR	* childAttr = children->attr;
							while (childAttr) {
								if (childAttr->key == GetKey((char*) "offset")) {
									assert(checkInteger(childAttr->value));
									assert(offset == -1);
									offset = atoi(childAttr->value);

									if (maxOffset < offset) {
										maxOffset = offset;
									}
								}
								else if (childAttr->key == GetKey((char*) "semantic")) {
									assert(isVERTEX == false && isNORMAL == false && isTEXCOORD == false);
									if (equal(childAttr->value, (char*) "VERTEX")) {
										isVERTEX = true;
									}
									else if (equal(childAttr->value, (char*) "NORMAL")) {
										isNORMAL = true;
									}
									else if (equal(childAttr->value, (char*) "TEXCOORD")) {
										isTEXCOORD = true;
									}
									else if (equal(childAttr->value, (char*) "COLOR")) {
										isCOLOR = true;
									}
									else {
										assert(false);
									}
								}
								else if (childAttr->key == GetKey((char*) "source")) {
									if (childAttr->value[0] == '#') {
										SOURCES	* sources = mesh->sources;
										while (sources) {
											if (equal(sources->source->__id, &childAttr->value[1])) {
												assert(source == nullptr);
												source = sources->source;
											}
											sources = sources->next;
										}
										assert(source);
									}
									else {
										assert(false);
									}
								}
								else if (childAttr->key == GetKey((char*) "set")) {
									//...
									//...	ASSERT(FALSE)
									//...
								}
								else {
									assert(false);
								}
								childAttr = childAttr->next;
							}

							assert(offset >= 0 && source);
							if (isVERTEX) {
								assert(hasVERTEX == false);
								hasVERTEX = true;
								VERTEX__offset = offset;
								VERTEX__source = source;

								verticesArray = new double[VERTEX__source->__floatArrayCount];
								memcpy(verticesArray, VERTEX__source->__floatArray, VERTEX__source->__floatArrayCount * sizeof(double));
								verticesArrayCard = VERTEX__source->__floatArrayCount;

								//
								//				!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! te remove
								//

								if (isProcessingFIRST) {
									if (verticesArrayCard >= 3) {
										double	__x = verticesArray[0],
												__y = verticesArray[1],
												__z = verticesArray[2];
										if (std::fabs(__x) > 10000 || std::fabs(__y) > 10000 || std::fabs(__z) > 10000) {
											glOffset__X = -__x;
											glOffset__Y = -__y;
											glOffset__Z = -__z;
										}
										else {
											glOffset__X = 0;
											glOffset__Y = 0;
											glOffset__Z = 0;
										}
										isProcessingFIRST = false;
									}
								}

								int o = 0;
								if ((glOffset__X || glOffset__Y || glOffset__Z) && (o < verticesArrayCard)) {
									assert(VERTEX__source->adjusted == false);
									while (o < verticesArrayCard) {
										if (VERTEX__source->adjusted == false) {
											verticesArray[o + 0] += glOffset__X;
											verticesArray[o + 1] += glOffset__Y;
											verticesArray[o + 2] += glOffset__Z;
											VERTEX__source->__floatArray[o + 0] += glOffset__X;
											VERTEX__source->__floatArray[o + 1] += glOffset__Y;
											VERTEX__source->__floatArray[o + 2] += glOffset__Z;
										}
										o += 3;
									}
									VERTEX__source->adjusted = true;
									assert(o == verticesArrayCard);
								}
							}
							else if (isNORMAL) {
								assert(hasNORMAL == false);
								hasNORMAL = true;
								NORMAL__offset = offset;
								NORMAL__source = source;
							}
							else if (isTEXCOORD) {
								assert(hasTEXCOORD == false);
								hasTEXCOORD = true;
								TEXCOORD__offset = offset;
								TEXCOORD__source = source;

								texturesArray = new double[TEXCOORD__source->__floatArrayCount];
								memcpy(texturesArray, TEXCOORD__source->__floatArray, TEXCOORD__source->__floatArrayCount * sizeof(double));
								texturesArrayCard = TEXCOORD__source->__floatArrayCount;
							}
							else if (isCOLOR) {
								//...
								//...
								//...
							}
							else {
								assert(false);
							}
						}
						else if (children->key == GetKey((char*) "p")) {
							//	...
						}
						else {
							assert(false);
						}
						children = children->next;
					}

					pBlockSize = maxOffset + 1;

					children = items->child;
					while (children) {
						assert(children->child == nullptr);
						if (children->key == GetKey((char*) "input")) {
							//	...
						}
						else if (children->key == GetKey((char*) "p")) {
							assert(pArray == nullptr);

							assert(children->attr == nullptr);
							pArray = GetIntegers(children->value, pBlockSize * ATTR__count__asInteger * 3 /* 3 elements per triangle */);
						}
						else {
							assert(false);
						}
						children = children->next;
					}
				}



/*				double	* verticesArray = nullptr;

SOURCE		* verticesSource = nullptr, * texturesSource = nullptr;
				if	(items->child->key == GetKey("input")) {
//					SOURCE		* verticesSource = nullptr;

					STRUCT_ATTR	* attr = items->child->attr;
					while  (attr) {
						if	(attr->key == GetKey("offset")) {
							assert(checkInteger(attr->value));
							int offset = atoi(attr->value);
							assert(offset == 0);
						} else if  (attr->key == GetKey("semantic")) {
							assert(equal(attr->value, "VERTEX"));
						} else if  (attr->key == GetKey("source")) {
							if	(attr->value[0] == '#') {
								SOURCES	* sources = mesh->sources;
								while  (sources) {
									if	(equal(sources->source->__id, &attr->value[1])) {
										assert(verticesSource == nullptr);
										verticesSource = sources->source;
									}
									sources = sources->next;
								}
							} else {
								assert(false);
							}
						} else {
							assert(false);
						}
						attr = attr->next;
					}

					assert(verticesSource);

					verticesArray = new double[verticesSource->__floatArrayCount];
					memcpy(verticesArray, verticesSource->__floatArray, verticesSource->__floatArrayCount * sizeof(double));
				} else {
					assert(false);
				}	//	*/

/*				int lists = 0;
				STRUCT_ITEM	* items_source = items->child;
				while  (items_source) {
					if	(items_source->key == GetKey("input")) {
						lists++;
						if	(lists == 2) {
							assert(texturesSource == nullptr);
							STRUCT_ATTR	* attr = items_source->attr;
							while  (attr) {
								if	(attr->key == GetKey("source")) {
									if	(attr->value[0] == '#') {
										SOURCES	* sources = mesh->sources;
										while  (sources) {
											if	(equal(sources->source->__id, &attr->value[1])) {
												assert(texturesSource == nullptr);
												texturesSource = sources->source;
											}
											sources = sources->next;
										}
									} else {
										assert(false);
									}
								}
								attr = attr->next;
							}
							assert(texturesSource);
						} else if (lists == 3) {
/////20180711///							assert(texturesSource == nullptr);
							STRUCT_ATTR	* attr = items_source->attr;
							while (attr) {
								if (attr->key == GetKey("source")) {
									if (attr->value[0] == '#') {
										SOURCES	* sources = mesh->sources;
										while (sources) {
											if (equal(sources->source->__id, &attr->value[1])) {
/////20180711///												assert(texturesSource == nullptr);
												texturesSource = sources->source;
											}
											sources = sources->next;
										}
									} else {
										assert(false);
									}
								}
								attr = attr->next;
							}
							assert(texturesSource);
							int u = 0;
/////20180711///COULD ALSO BE 3 (file 5)							assert(lists == 1);
						} else {
							assert(lists == 1);
						}
					}
					items_source = items_source->next;
				}
				assert(lists == 1  ||  lists == 2  ||  lists == 3);
	//	*/

/*				bool	items_p_found = false;
				STRUCT_ITEM	* items_p = items->child;
				while (items_p) {
					if (items_p->key == GetKey("p")) {
						assert(items_p_found == false);
						items_p_found = true;

						int64_t count = 0;
						if (items->attr->key == GetKey("count")) {
							assert(checkInteger(items->attr->value));
							count = atoi(items->attr->value);



							//
							//
							//



							//
							//
							//
						}
						else if (items->attr->next && items->attr->next->key == GetKey("count")) {
							assert(checkInteger(items->attr->next->value));
							count = atoi(items->attr->next->value);
						}
						else {
							assert(false);
						}
						int	* pArray = GetIntegers(items_p->value, lists * 3 * count);
//	*/

				if (pArray) {
					double	* vArray = nullptr,
							* tArray = nullptr;
					if (hasVERTEX) {
						vArray = new double[3 * ATTR__count__asInteger * 3 /* 3 elements per triangle */];
					}

					assert(mI->texture == false);
					if (hasTEXCOORD) {
						tArray = new double[2 * ATTR__count__asInteger * 3 /* 3 elements per triangle */];

						mI->texture = true;
					}

					int64_t	* indexArray = new int64_t[8 * ATTR__count__asInteger];

					int i = 0;
					while (i < ATTR__count__asInteger) {

						indexArray[4 * i + 0] = 3 * i + 0;
						{
							int index = pArray[pBlockSize * (3 * i + 0) + VERTEX__offset];
							vArray[3 * (3 * i + 0) + 0] = VERTEX__source->__floatArray[3 * index + 0];
							vArray[3 * (3 * i + 0) + 1] = VERTEX__source->__floatArray[3 * index + 1];
							vArray[3 * (3 * i + 0) + 2] = VERTEX__source->__floatArray[3 * index + 2];

							if (TEXCOORD__source) {
/////200180711///								assert(pBlockSize >= 2);
								index = pArray[pBlockSize * (3 * i + 0) + TEXCOORD__offset];

								tArray[2 * (3 * i + 0) + 0] = TEXCOORD__source->__floatArray[2 * index + 0];// -toffsetx;
								tArray[2 * (3 * i + 0) + 1] = -TEXCOORD__source->__floatArray[2 * index + 1];// -toffsety;
							}
						}

						indexArray[4 * i + 1] = 3 * i + 2;
						{
							int index = pArray[pBlockSize * (3 * i + 1) + VERTEX__offset];
							vArray[3 * (3 * i + 1) + 0] = VERTEX__source->__floatArray[3 * index + 0];
							vArray[3 * (3 * i + 1) + 1] = VERTEX__source->__floatArray[3 * index + 1];
							vArray[3 * (3 * i + 1) + 2] = VERTEX__source->__floatArray[3 * index + 2];
							if (TEXCOORD__source) {
								index = pArray[pBlockSize * (3 * i + 1) + TEXCOORD__offset];
								tArray[2 * (3 * i + 1) + 0] = TEXCOORD__source->__floatArray[2 * index + 0];// -toffsetx;
								tArray[2 * (3 * i + 1) + 1] = -TEXCOORD__source->__floatArray[2 * index + 1];// -toffsety;
							}
						}

						indexArray[4 * i + 2] = 3 * i + 1;
						{
							int index = pArray[pBlockSize * (3 * i + 2) + VERTEX__offset];
							vArray[3 * (3 * i + 2) + 0] = VERTEX__source->__floatArray[3 * index + 0];
							vArray[3 * (3 * i + 2) + 1] = VERTEX__source->__floatArray[3 * index + 1];
							vArray[3 * (3 * i + 2) + 2] = VERTEX__source->__floatArray[3 * index + 2];
							if (TEXCOORD__source) {
								index = pArray[pBlockSize * (3 * i + 2) + TEXCOORD__offset];
								tArray[2 * (3 * i + 2) + 0] = TEXCOORD__source->__floatArray[2 * index + 0];// -toffsetx;
								tArray[2 * (3 * i + 2) + 1] = -TEXCOORD__source->__floatArray[2 * index + 1];// -toffsety;
							}
						}

						indexArray[4 * i + 3] = -1;
						i++;
					}

					i = 0;
					while (i < ATTR__count__asInteger) {
						indexArray[4 * (ATTR__count__asInteger + i) + 0] = 3 * i + 0;
						indexArray[4 * (ATTR__count__asInteger + i) + 1] = 3 * i + 2;
						indexArray[4 * (ATTR__count__asInteger + i) + 2] = 3 * i + 1;
						indexArray[4 * (ATTR__count__asInteger + i) + 3] = -1;

						i++;
					}

//					if (hasTriangulationWithTexture && hasTEXCOORD == false) {
						//	skip this side
//					} else {
						MergeFaces__new(
								model,
								globalObjectsNormal[noElements],
								globalObjectsInverted[noElements],
								vArray,
								tArray,
								3 * ATTR__count__asInteger,
								indexArray,
								4 * ATTR__count__asInteger
							);
//					}
				}








/*
double minZ = 0, maxZ = 0;
int	toffsetx = 0, toffsety = 0;
	
if	(texturesSource) {
	//toffsetx = texturesSource->__floatArray[0] - 2;
	//toffsety = texturesSource->__floatArray[1] - 2;

	mI->texture = true;
}

//						int64_t	* indexArray = new int64_t[4 * count];
int64_t	* indexArray = new int64_t[8 * count];
int i = 0;
while  (i < count) {

	indexArray[4 * i + 0] = 3 * i + 0;
	{
		int index = pArray[lists * (3 * i + 0)];
		vArray[3 * (3 * i + 0) + 0] = verticesSource->__floatArray[3 * index + 0];
		vArray[3 * (3 * i + 0) + 1] = verticesSource->__floatArray[3 * index + 1];
		vArray[3 * (3 * i + 0) + 2] = verticesSource->__floatArray[3 * index + 2];

		if	(texturesSource) {
		//	assert(lists == 2);
			assert(lists >= 2);
			index = pArray[lists * (3 * i + 0) + 1];

/*			toffsetx = 0;
			if (texturesSource->__floatArray[2 * index + 0] - toffsetx < 0) {
				while (texturesSource->__floatArray[2 * index + 0] - toffsetx < 0) {
					toffsetx -= 1;
				}
			} else {
				while (texturesSource->__floatArray[2 * index + 0] - toffsetx > 1) {
					toffsetx += 1;
				}
			}

			toffsety = 0;
			if (texturesSource->__floatArray[2 * index + 1] - toffsety < 0) {
				while (texturesSource->__floatArray[2 * index + 1] - toffsety < 0) {
					toffsety -= 1;
				}
			}
			else {
				while (texturesSource->__floatArray[2 * index + 1] - toffsety > 1) {
					toffsety += 1;
				}
			}
* /
			tArray[2 * (3 * i + 0) + 0] = texturesSource->__floatArray[2 * index + 0] - toffsetx;
	//		tArray[2 * (3 * i + 0) + 1] = texturesSource->__floatArray[2 * index + 1] - toffsety;
			tArray[2 * (3 * i + 0) + 1] = -texturesSource->__floatArray[2 * index + 1] - toffsety;
		}
	}

	indexArray[4 * i + 1] = 3 * i + 2;//1;
	{
		int index = pArray[lists * (3 * i + 1)];
		vArray[3 * (3 * i + 1) + 0] = verticesSource->__floatArray[3 * index + 0];
		vArray[3 * (3 * i + 1) + 1] = verticesSource->__floatArray[3 * index + 1];
		vArray[3 * (3 * i + 1) + 2] = verticesSource->__floatArray[3 * index + 2];
		if	(texturesSource) {
			index = pArray[lists * (3 * i + 1) + 1];
			tArray[2 * (3 * i + 1) + 0] = texturesSource->__floatArray[2 * index + 0] - toffsetx;
	//		tArray[2 * (3 * i + 1) + 1] = texturesSource->__floatArray[2 * index + 1] - toffsety;
			tArray[2 * (3 * i + 1) + 1] = -texturesSource->__floatArray[2 * index + 1] - toffsety;
		}
	}

	indexArray[4 * i + 2] = 3 * i + 1;//2;
	{
		int index = pArray[lists * (3 * i + 2)];
		vArray[3 * (3 * i + 2) + 0] = verticesSource->__floatArray[3 * index + 0];
		vArray[3 * (3 * i + 2) + 1] = verticesSource->__floatArray[3 * index + 1];
		vArray[3 * (3 * i + 2) + 2] = verticesSource->__floatArray[3 * index + 2];
		if	(texturesSource) {
			index = pArray[lists * (3 * i + 2) + 1];
			tArray[2 * (3 * i + 2) + 0] = texturesSource->__floatArray[2 * index + 0] - toffsetx;
	//		tArray[2 * (3 * i + 2) + 1] = texturesSource->__floatArray[2 * index + 1] - toffsety;
			tArray[2 * (3 * i + 2) + 1] = -texturesSource->__floatArray[2 * index + 1] - toffsety;
		}
	}

if (i == 0) {
	minZ = vArray[3 * (3 * i + 0) + 2];
	maxZ = minZ;
} else {
	if (minZ > vArray[3 * (3 * i + 0) + 2]) { minZ = vArray[3 * (3 * i + 0) + 2]; }
	if (maxZ < vArray[3 * (3 * i + 0) + 2]) { maxZ = vArray[3 * (3 * i + 0) + 2]; }
}

							indexArray[4 * i + 3] = -1;
							i++;
						}

						i = 0;
						while  (i < count) {
							indexArray[4 * (count + i) + 0] = 3 * i + 0;
							indexArray[4 * (count + i) + 1] = 3 * i + 2;
							indexArray[4 * (count + i) + 2] = 3 * i + 1;
							indexArray[4 * (count + i) + 3] = -1;


							i++;
						}

	if	(texturesSource) {
		i = 0;
		while  (i < 3 * count * 2) {
	//		while  (tArray[i] < 0) {
	//			tArray[i]++;
	//		}
	//		while  (tArray[i] > 1) {
	//			tArray[i]--;
	//		}
			i++;
		}
	}

if (texturesSource == 0) {
	tArray = 0;
}

						MergeFaces__new(
								rdfModel,
								globalObjectsNormal[noElements],
								globalObjectsInverted[noElements],
								vArray,
								tArray,
								3 * count,
								indexArray,
								4 * count
							);
					}
					items_p = items_p->next;
				}
				assert(items_p_found);
//	*/
				///...
				///...
				///...

				noElements++;

				assert(noElements < 49999);
			} else if (items->key == GetKey((char*) "lines")) {
				 //...
				 //...
				 //...
				 //...
				 //...
 			} else if  (items->key == GetKey((char*) "polygons")) {
//				POLYGON		* polygon = nullptr;

//				int			count = 0;
//				char		* matTXT = nullptr;
//				STRUCT_ATTR	* attr = items->attr;
//				while (attr) {
//					if (attr->key == GetKey("count")) {
//						assert(checkInteger(attr->value));
//						count = atoi(attr->value);
//					} else if (attr->key == GetKey("material")) {
//						matTXT = attr->value;
//					} else {
//						assert(false);
//					}
//					attr = attr->next;
//				}
//
//				assert(matTXT);

				POLYGON	* polygon = parsePolygons(items);

				int64_t	owlClassBoundaryRepresentation = GetClassByName(model, "BoundaryRepresentation");

				globalObjectsNormal[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);
				globalObjectsInverted[noElements] = CreateInstance(owlClassBoundaryRepresentation, 0);

				(*ppMeshInstance) = new MESH_INSTANCES;

				(*ppMeshInstance)->rdfInstanceNormal = globalObjectsNormal[noElements];
				(*ppMeshInstance)->rdfInstanceInverted = globalObjectsInverted[noElements];
				(*ppMeshInstance)->texture = false;
				(*ppMeshInstance)->next = nullptr;
				(*ppMeshInstance)->material = polygon->material;

				ppMeshInstance = &(*ppMeshInstance)->next;

				SOURCE	* verticesSource = nullptr, *texCoordSource = nullptr;
				int		verticesOffset = -1, texCoordOffset = -1,
						offsetCnt = 0;

				__INPUT	* input = polygon->inputs;
				while (input) {
					if (input->type == TYPE__VERTEX) {
						assert(verticesSource == 0);

						SOURCES	* sources = mesh->sources;
						while (sources) {
							assert(input->source[0] == '#');
							if (equal(sources->source->__id, &input->source[1])) {
								assert(verticesSource == nullptr);
								verticesSource = sources->source;
								verticesOffset = input->offset;
							}
							sources = sources->next;
						}

						assert(verticesSource  &&  verticesOffset >= 0);
					} else if (input->type == TYPE__TEXCOORD) {
						assert(texCoordSource == 0);

						SOURCES	* sources = mesh->sources;
						while (sources) {
							assert(input->source[0] == '#');
							if (equal(sources->source->__id, &input->source[1])) {
								assert(texCoordSource == nullptr);
								texCoordSource = sources->source;
								texCoordOffset = input->offset;
							}
							sources = sources->next;
						}

						assert(texCoordSource  &&  texCoordOffset >= 0);
					} else {
						assert(false);
					}

					if (input->offset == offsetCnt) {
						offsetCnt++;
					} else {
						assert(false);
					}

					input = input->next;
				}
				assert(verticesSource);

	//			if (texCoordSource) {
//					int64_t vertexCnt = verticesSource->__floatArrayCount / 3;
//					double	* verticesArray = new double[3 * vertexCnt];
//					memcpy(verticesArray, verticesSource->__floatArray, vertexCnt * 3 * sizeof(double));

					int noVertices = verticesSource->__floatArrayCount / 3;
					assert(3 * noVertices == verticesSource->__floatArrayCount);

					int		sizeIndexA = 0, sizeVertexA = 0;
					__PH	* ph = polygon->ph;
					while (ph) {
						sizeIndexA += ph->p->count / offsetCnt + 1;
						sizeVertexA += ph->p->count / offsetCnt;
						sizeIndexA += ph->h->count / offsetCnt + 1;
						sizeVertexA += ph->h->count / offsetCnt;
						ph = ph->next;
					}

					assert(sizeIndexA == sizeVertexA + 2 * polygon->count);

					int64_t vertexCnt = sizeVertexA;
					double	* verticesArray = new double[3 * sizeVertexA],
							* texCoordArray = nullptr;
					if (texCoordSource) {
						texCoordArray = new double[2 * sizeVertexA];
					}

					int64_t	* indexArray = new int64_t[sizeIndexA];
					int64_t	indexArraySize = 0, vertexArraySize = 0;
					ph = polygon->ph;
					while (ph) {
						int pc = 0;
						while (pc < ph->p->count) {
							int	vertexIndex = ph->p->intArray[pc + verticesOffset];
							verticesArray[3 * vertexArraySize + 0] = verticesSource->__floatArray[3 * vertexIndex + 0];
							verticesArray[3 * vertexArraySize + 1] = verticesSource->__floatArray[3 * vertexIndex + 1];
							verticesArray[3 * vertexArraySize + 2] = verticesSource->__floatArray[3 * vertexIndex + 2];
							assert(vertexIndex >= 0 && 3 * vertexIndex + 2 < verticesSource->__floatArrayCount);

							if (texCoordSource) {
								int	texCoordIndex = ph->p->intArray[pc + texCoordOffset];
								texCoordArray[2 * vertexArraySize + 0] = 0 + texCoordSource->__floatArray[2 * texCoordIndex + 0];
								texCoordArray[2 * vertexArraySize + 1] = 1 - texCoordSource->__floatArray[2 * texCoordIndex + 1];
								assert(texCoordIndex >= 0 && 2 * texCoordIndex + 1 < verticesSource->__floatArrayCount);
							}
							indexArray[indexArraySize] = vertexArraySize;

							vertexArraySize++;
							indexArraySize++;
							pc += offsetCnt;
						}
						indexArray[indexArraySize++] = -1;

						int hc = 0;
						while (hc < ph->h->count) {
							int	vertexIndex = ph->h->intArray[hc + verticesOffset];
							verticesArray[3 * vertexArraySize + 0] = verticesSource->__floatArray[3 * vertexIndex + 0];
							verticesArray[3 * vertexArraySize + 1] = verticesSource->__floatArray[3 * vertexIndex + 1];
							verticesArray[3 * vertexArraySize + 2] = verticesSource->__floatArray[3 * vertexIndex + 2];
							assert(vertexIndex >= 0 && 3 * vertexIndex + 2 < verticesSource->__floatArrayCount);

							if (texCoordSource) {
								int	texCoordIndex = ph->h->intArray[hc + texCoordOffset];
								texCoordArray[2 * vertexArraySize + 0] = 0 + texCoordSource->__floatArray[2 * texCoordIndex + 0];
								texCoordArray[2 * vertexArraySize + 1] = 1 - texCoordSource->__floatArray[2 * texCoordIndex + 1];
								assert(texCoordIndex >= 0 && 2 * texCoordIndex + 1 < verticesSource->__floatArrayCount);
							}

							indexArray[indexArraySize] = vertexArraySize;

							vertexArraySize++;
							indexArraySize++;
							hc += offsetCnt;
						}
						indexArray[indexArraySize++] = -2;

						ph = ph->next;
					}

					assert(indexArraySize == sizeIndexA);

					MergeFaces__new(
							model,
							globalObjectsNormal[noElements],
							globalObjectsInverted[noElements],
							verticesArray,
							texCoordArray,
							vertexCnt,
							indexArray,
							indexArraySize
						);
				

				noElements++;

				assert(noElements < 49999);
			} else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

void	parseGeometry(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
	if (item->key == GetKey((char*) "geometry")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "mesh")) {
				if (item->attr  &&  item->attr->next) {
					assert(item->attr  &&  item->attr->next  &&  item->attr->next->next == nullptr);
					assert(item->attr->key == GetKey((char*) "id"));
					assert(item->attr->next->key == GetKey((char*) "name"));
				}
				else {
					assert(item->attr  &&  item->attr->next == nullptr);
					assert(item->attr->key == GetKey((char*) "id"));
				}
				MESH	* mesh = new MESH;

				mesh->sources = nullptr;
				mesh->instances = nullptr;
				if	(item->attr->key == GetKey((char*) "id")) {
					mesh->id = item->attr->value;
				} else {
					mesh->id = nullptr;
				}
				mesh->next = nullptr;

				SetMesh(mesh);
				parseMesh(model, items, mesh);

				int u = 0;
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

void	parseLibraryGeometries(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
	if (item->key == GetKey((char*) "library_geometries")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "geometry")) {
				parseGeometry(model, items);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

double	* parseRotate(STRUCT_ITEM * item, int * offset)
{
	if (item->key == GetKey((char*) "rotate")) {
		assert(item->child == nullptr);
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "sid")) {
				if (equal(attr->value, (char*) "rotateX")) {
					*offset = 0;
				} else if (equal(attr->value, (char*) "rotateY")) {
					*offset = 1;
				} else if (equal(attr->value, (char*) "rotateZ")) {
					*offset = 2;
				} else {
					assert(false);
					*offset = -1;
				}
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		if (item->value) {
			char	* buff = new char[512];
			memcpy(buff, item->value, (strlen(item->value) + 1) * sizeof(char));
			return	GetDoubles(buff, 4);
		}
	}

	assert(false);
	return	nullptr;
}

double	* parseTranslate(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "translate")) {
		assert(item->child == nullptr);
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "sid")) {
				assert(equal(attr->value, (char*) "translate"));
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		if (item->value) {
			char	* buff = new char[512];
			memcpy(buff, item->value, (strlen(item->value) + 1) * sizeof(char));
			return	GetDoubles(buff, 3);
		}
	}

	assert(false);
	return	nullptr;
}

double	* parseScale(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "scale")) {
		assert(item->child == nullptr);
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "sid")) {
				assert(equal(attr->value, (char*) "scale"));
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		if (item->value) {
			char	* buff = new char[512];
			memcpy(buff, item->value, (strlen(item->value) + 1) * sizeof(char));
			return	GetDoubles(buff, 3);
		}
	}

	assert(false);
	return	nullptr;
}

double	* parseMatrix(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "matrix")) {
		assert(item->child == nullptr);

		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "sid")) {
				assert(equal(attr->value, (char*) "transform") || equal(attr->value, (char*) "matrix"));
			} else {
				assert(false);
			}
			attr = attr->next;
		}

		if	(item->value) {
			char	* buff = new char[512];
			memcpy(buff, item->value, (strlen(item->value) + 1) * sizeof(char));
			return	GetDoubles(buff, 16);
		}
	}
	
	assert(false);
	return	nullptr;
}

MATERIAL_INSTANCE	* parseBindMaterial(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "bind_material")) {
		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if	(items->key == GetKey((char*) "technique_common")) {
				TECHNIQUE_COMMON	* TS = parseTechniqueCommon(items);
				assert(TS->materialInstances);
				return	TS->materialInstances;
			} else {
				assert(false);
			}
			items = items->next;
		}
	}

	assert(false);
	return	nullptr;
}

INSTANCE_GEOMETRY	* parseInstanceGeometry(
							int64_t		model,
							STRUCT_ITEM * item
						)
{
	if	(item->key == GetKey((char*) "instance_geometry")) {
		if	(item->attr  &&  item->attr->next) {
			assert(item->attr  &&  item->attr->next  &&  item->attr->next->next == nullptr);
			assert(item->attr->key == GetKey((char*) "url")  &&  item->attr->value[0] == '#');
			assert(item->attr->next->key == GetKey((char*) "name"));
		} else {
			assert(item->attr  &&  item->attr->next == nullptr);
			assert(item->attr->key == GetKey((char*) "url")  &&  item->attr->value[0] == '#');
		}

		INSTANCE_GEOMETRY	* instance_geometry = new INSTANCE_GEOMETRY;

		instance_geometry->materialInstances = nullptr;
		if	(item->attr->key == GetKey((char*) "url")  &&  item->attr->value[0] == '#') {
			char	* value = &item->attr->value[1];
			instance_geometry->mesh = GetMesh(value);
		} else {
			instance_geometry->mesh = nullptr;
			assert(false);
		}
		
		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if	(items->key == GetKey((char*) "bind_material")) {
				instance_geometry->materialInstances = parseBindMaterial(items);
			} else {
				assert(false);
			}
			items = items->next;
		}

		if	(instance_geometry->mesh) {
			bool	texture = false, over_ride = false;
			MESH_INSTANCES	* meshInstances = instance_geometry->mesh->instances;
		} else {
			assert(false);
		}

		return	instance_geometry;
	}

	assert(false);
	return	nullptr;
}

int_t	parseNode(
				int64_t		model,
				STRUCT_ITEM * item,
				int64_t		* objects,
				char		** objectsID,
				int_t		nodeCount
			)
{
	if (item->key == GetKey((char*) "node")) {
		INSTANCE_GEOMETRY	* instance_geometry = nullptr;
		bool	instance_camera = false,
				instance_light = false;
		double	* matrix = nullptr,
				* translate = nullptr,
				* rotateX = nullptr, * rotateY = nullptr, * rotateZ = nullptr,
				* scale = nullptr;

		char	* name = nullptr, * id = nullptr, * sid = nullptr, * layer = nullptr, * type = nullptr;
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "name")) {
				name = attr->value;
			}
			else if (attr->key == GetKey((char*) "id")) {
				id = attr->value;
			}
			else if (attr->key == GetKey((char*) "sid")) {
				sid = attr->value;
			}
			else if (attr->key == GetKey((char*) "layer")) {
				layer = attr->value;
			}
			else if (attr->key == GetKey((char*) "type")) {
				type = attr->value;
			}
			else {
				assert(false);
			}
			attr = attr->next;
		}

		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if (items->key == GetKey((char*) "matrix")) {
				//...
			}
			else if  (items->key == GetKey((char*) "instance_camera")) {
				instance_camera = true;
			}
			else if  (items->key == GetKey((char*) "instance_light")) {
				instance_light = true;
			}
			else if  (items->key == GetKey((char*) "instance_geometry")) {
				//...
			}
			else if  (items->key == GetKey((char*) "node")) {
				//...
			}
			else if  (items->key == GetKey((char*) "instance_node")) {
				//...
			}
			else if (items->key == GetKey((char*) "translate")) {
				translate = parseTranslate(items);
			}
			else if  (items->key == GetKey((char*) "rotate")) {
				int		offset = -1;
				double	* rotate = parseRotate(items, &offset);
				switch (offset) {
					case  0:
						rotateX = rotate;
						break;
					case  1:
						rotateY = rotate;
						break;
					case  2:
						rotateZ = rotate;
						break;
					default:
/////20180711///						assert(false);
						break;
				}
			}
			else if  (items->key == GetKey((char*) "lookat")) {
				//...
			}
			else if  (items->key == GetKey((char*) "extra")) {
				//...
			}
			else if  (items->key == GetKey((char*) "scale")) {
				scale = parseScale(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		if (translate || rotateX || rotateY || rotateZ || scale) {
			matrix = new double[16];

			if (rotateX) {
				double	scaleFactor = 1;
				if (scale) {
					scaleFactor = scale[0];
				}
				else {
					assert(false);
				}
				matrix[0] = rotateX[0] * scaleFactor;
				matrix[1] = rotateX[1] * scaleFactor;
				matrix[2] = rotateX[2] * scaleFactor;
				matrix[3] = rotateX[3] * scaleFactor;
				assert(matrix[3] == 0);
			}
			else {
//////////////				assert(false);
				matrix[0] = 1;
				matrix[1] = 0;
				matrix[2] = 0;
				matrix[3] = 0;
			}

			if (rotateY) {
				double	scaleFactor = 1;
				if (scale) {
					scaleFactor = scale[1];
				}
				else {
					assert(false);
				}
				matrix[4] = rotateY[0] * scaleFactor;
				matrix[5] = rotateY[1] * scaleFactor;
				matrix[6] = rotateY[2] * scaleFactor;
				matrix[7] = rotateY[3] * scaleFactor;
				assert(matrix[7] == 0);
			}
			else {
/////////////				assert(false);
				matrix[4] = 0;
				matrix[5] = 1;
				matrix[6] = 0;
				matrix[7] = 0;
			}

			if (rotateZ) {
				double	scaleFactor = 1;
				if (scale) {
					scaleFactor = scale[2];
				}
				else {
					assert(false);
				}
				matrix[8] = rotateZ[0] * scaleFactor;
				matrix[9] = rotateZ[1] * scaleFactor;
				matrix[10] = rotateZ[2] * scaleFactor;
				matrix[11] = rotateZ[3] * scaleFactor;
				assert(matrix[11] == 0);
			}
			else {
////////////				assert(false);
				matrix[8] = 0;
				matrix[9] = 0;
				matrix[10] = 1;
				matrix[11] = 0;
			}

			if (translate) {
				matrix[12] = translate[0];
				matrix[13] = translate[1];
				matrix[14] = translate[2];
				matrix[15] = 1;
			}
			else {
				assert(false);
				matrix[12] = 0;
				matrix[13] = 0;
				matrix[14] = 0;
				matrix[15] = 1;
			}
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "matrix")) {
				assert(matrix == nullptr);
				matrix = parseMatrix(items);
			}
			else if  (items->key == GetKey((char*) "instance_camera")) {
				//...
			}
			else if  (items->key == GetKey((char*) "instance_light")) {
				//...
			}
			else if  (items->key == GetKey((char*) "instance_geometry")) {
				//...
			}
			else if  (items->key == GetKey((char*) "node")) {
				//...
			}
			else if  (items->key == GetKey((char*) "instance_node")) {
				//...
			}
			else if (items->key == GetKey((char*) "translate")) {
				//...
			}
			else if  (items->key == GetKey((char*) "rotate")) {
				//...
			}
			else if  (items->key == GetKey((char*) "lookat")) {
				//...
			}
			else if  (items->key == GetKey((char*) "extra")) {
				//...
			}
			else if  (items->key == GetKey((char*) "scale")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		if (!matrix) {
			matrix = new double[16];
			matrix[0] = 1;
			matrix[1] = 0;
			matrix[2] = 0;
			matrix[3] = 0;
			matrix[4] = 0;
			matrix[5] = 1;
			matrix[6] = 0;
			matrix[7] = 0;
			matrix[8] = 0;
			matrix[9] = 0;
			matrix[10] = 1;
			matrix[11] = 0;
			matrix[12] = 0;
			matrix[13] = 0;
			matrix[14] = 0;
			matrix[15] = 1;
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "matrix")) {
				//...
			}
			else if (items->key == GetKey((char*) "instance_camera")) {
				//...
			}
			else if (items->key == GetKey((char*) "instance_light")) {
				//...
			}
			else if (items->key == GetKey((char*) "instance_geometry")) {
				instance_geometry = parseInstanceGeometry(model, items);

				char	* myObjectID = item->attr->value;

				if (instance_geometry->materialInstances  &&  instance_geometry->mesh->instances) {
					bool				foundTexture = false;
					MESH_INSTANCES		* meshInstances = instance_geometry->mesh->instances;
					while (meshInstances) {
						MATERIAL_INSTANCE	* materialInstances = instance_geometry->materialInstances;
						while (materialInstances) {
							if (equal(materialInstances->symbol, meshInstances->material)) {
								if (materialInstances->material  &&  materialInstances->material->color->hasTexture) {
									foundTexture = true;
								}
							}
							materialInstances = materialInstances->next;
						}
						meshInstances = meshInstances->next;
					}

					meshInstances = instance_geometry->mesh->instances;
					myObjectID = instance_geometry->mesh->id;

					int instancesCnt = 0, instancesTextureCnt = 0;

					while (meshInstances) {
						MATERIAL_INSTANCE	* materialInstances = instance_geometry->materialInstances;

//						bool	skipMe = false;

						MA__TERIAL	* selectedMaterial = nullptr;
						bool	forceIgnoreThisGeometry = false;
						int		fndMaterials = 0;
						while (materialInstances) {
							if (equal(materialInstances->symbol, meshInstances->material)) {
								if (materialInstances->material) {
									selectedMaterial = materialInstances->material;
//									SetObjectTypeProperty(instanceTransformation, rdfPropertyMaterial, &materialInstances->material->color->rdfInstance, 1);
								} else {
									forceIgnoreThisGeometry = true;
								}
								//								SetObjectTypeProperty(meshInstances->rdfInstance, rdfPropertyMaterial, &materialInstances->material->color->rdfInstance, 1);
								fndMaterials++;
							}
							materialInstances = materialInstances->next;
						}
	///////					assert(fndMaterials == 1);

//						if (materialInstances->material  &&  materialInstances->material->color  &&  materialInstances->material->color->rdfInstance) {
//							SetObjectTypeProperty(instanceTransformation, rdfPropertyMaterial, &materialInstances->material->color->rdfInstance, 1);
//							SetObjectTypeProperty(meshInstances->rdfInstance, rdfPropertyMaterial, &materialInstances->material->color->rdfInstance, 1);
//
//							colorCnt++;
//						} else {
//							assert(false);
//							skipMe = true;
//						}

						if (meshInstances->rdfInstanceNormal) {
//							SetObjectTypeProperty(instanceTransformation, rdfPropertyObject, &meshInstances->rdfInstance, 1);
						} else {
							assert(false);
							forceIgnoreThisGeometry = true;
//							skipMe = true;
						}

					
//						...

						if (foundTexture) {
							if (selectedMaterial) {
								if (selectedMaterial->color->hasTexture == false) {
									forceIgnoreThisGeometry = true;
								}
							} else {
								assert(forceIgnoreThisGeometry == true);
							}
						}

						if (forceIgnoreThisGeometry == false) {
							int64_t	instanceMatrix = CreateInstanceMatrix(model, matrix),
									instanceTransformation = CreateInstance(GetClassByName(model, "Transformation"), nullptr);

							SetObjectProperty(instanceTransformation, GetPropertyByName(model, "material"), &selectedMaterial->color->rdfInstance, 1);
							if (GET_INVERTED) {
						//	if (GET_INVERTED == false) {
								SetObjectProperty(instanceTransformation, GetPropertyByName(model, "object"), &meshInstances->rdfInstanceInverted, 1);
							}
							else {
								SetObjectProperty(instanceTransformation, GetPropertyByName(model, "object"), &meshInstances->rdfInstanceNormal, 1);

								SetObjectProperty(meshInstances->rdfInstanceNormal, GetPropertyByName(model, "material"), &selectedMaterial->color->rdfInstance, 1);
							}
							SetObjectProperty(instanceTransformation, GetPropertyByName(model, "matrix"), &instanceMatrix, 1);

							objectsID[nodeCount] = myObjectID;
							objects[nodeCount] = instanceTransformation;
							nodeCount++;

							instancesCnt++;

							assert(selectedMaterial);
							if (selectedMaterial->color->hasTexture) {
								instancesTextureCnt++;
							}
						}

						meshInstances = meshInstances->next;
					}

					if (instancesTextureCnt) {
	//					assert(instancesCnt == 1);
					}
					else {
				//		assert(instancesCnt <= 1);
					}
				}

				DEBUG__checkMaterialPresent(model, objects, nodeCount);
			}
			else if (items->key == GetKey((char*) "node")) {

				bool	setInverted = false;
				if (equal(name, (char*) "_120_buildings")) {
					assert(GET_INVERTED == false);
					GET_INVERTED = true;
					setInverted = true;
				}

				int_t oldNodeCount = nodeCount;
				nodeCount = parseNode(model, items, objects, objectsID, nodeCount);

				if (matrix) {
					int64_t	instanceMatrix = CreateInstanceMatrix(model, matrix);

					int64_t	owlClassTransformation = GetClassByName(model, "Transformation"),
							rdfPropertyMaterial = GetPropertyByName(model, "material"),
							rdfPropertyMatrix = GetPropertyByName(model, "matrix"),
							rdfPropertyObject = GetPropertyByName(model, "object");

					while (oldNodeCount < nodeCount) {
						int64_t	instanceTransformation = CreateInstance(owlClassTransformation, nullptr);

						SetObjectProperty(instanceTransformation, rdfPropertyObject, &objects[oldNodeCount], 1);
						SetObjectProperty(instanceTransformation, rdfPropertyMatrix, &instanceMatrix, 1);

						int64_t	card = 0, *values = nullptr;
						GetObjectProperty(objects[oldNodeCount], rdfPropertyMaterial, &values, &card);
						if (card == 1) {
							SetObjectProperty(instanceTransformation, rdfPropertyMaterial, values, 1);
						}
						else {
							assert(false);
						}

						objects[oldNodeCount] = instanceTransformation;

						oldNodeCount++;
					}
				}

				if (setInverted) {
					assert(GET_INVERTED == true);
					GET_INVERTED = false;
				}

				DEBUG__checkMaterialPresent(model, objects, nodeCount);
			}
			else if (items->key == GetKey((char*) "instance_node")) {
				assert(items->attr->key == GetKey((char*) "url") && items->attr->value[0] == '#');
				NODE_REF	* nodeRef = nodeRefs;
				int_t	oldNodeCount = nodeCount;
				bool	done = false;
				while (nodeRef) {
					if (equal(nodeRef->id, &items->attr->value[1])) {
						assert(done == false);
						nodeCount = parseNode(model, nodeRef->items, objects, objectsID, nodeCount);
						done = true;
					}
					nodeRef = nodeRef->next;
				}
//				assert(done);

				while (oldNodeCount < nodeCount) {
					int64_t	owlClassTransformation = GetClassByName(model, "Transformation"),
							rdfPropertyMaterial = GetPropertyByName(model, "material"),
							rdfPropertyMatrix = GetPropertyByName(model, "matrix"),
							rdfPropertyObject = GetPropertyByName(model, "object");

					int64_t	instanceMatrix = CreateInstanceMatrix(model, matrix),
							instanceTransformation = CreateInstance(owlClassTransformation, nullptr);

					SetObjectProperty(instanceTransformation, rdfPropertyObject, &objects[oldNodeCount], 1);
					SetObjectProperty(instanceTransformation, rdfPropertyMatrix, &instanceMatrix, 1);

					int64_t	card = 0, *values = nullptr;
					GetObjectProperty(objects[oldNodeCount], rdfPropertyMaterial, &values, &card);
					if (card == 1) {
						SetObjectProperty(instanceTransformation, rdfPropertyMaterial, values, 1);
					}
					else {
						assert(false);
					}

					objects[oldNodeCount] = instanceTransformation;

					oldNodeCount++;
				}

				DEBUG__checkMaterialPresent(model, objects, nodeCount);
			}
			else if (items->key == GetKey((char*) "translate")) {
				//...
			}
			else if (items->key == GetKey((char*) "rotate")) {
				//...
			}
			else if (items->key == GetKey((char*) "lookat")) {
				//...
			}
			else if (items->key == GetKey((char*) "extra")) {
				//...
			}
			else if (items->key == GetKey((char*) "scale")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		if (instance_camera) {
			assert(instance_geometry == nullptr);
			return	nodeCount;
		}

		if (instance_light) {
			assert(instance_geometry == nullptr);
			return	nodeCount;
		}
	}

	DEBUG__checkMaterialPresent(model, objects, nodeCount);

	return	nodeCount;
}

int64_t	parseVisualScene(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
	int64_t	* objects = new int64_t[49999], nodeCount = 0;
	char	** objectsID = (char**) new int64_t[49999];

	if (item->key == GetKey((char*) "visual_scene")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "node")) {
//				NODE	* node = parseNode(items);
				nodeCount = parseNode(model, items, objects, objectsID, (int_t) nodeCount);

	//			if	(node  &&  node->rdfInstance) {
	//				objects[nodeCount] = node->rdfInstance;
	//				nodeCount++;
	//			}
			}
			else if (items->key == GetKey((char*) "extra")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}

	int64_t	objectsCard = nodeCount,
			owlClassCollection = GetClassByName(model, "Collection"),
			rdfPropertyObjects = GetPropertyByName(model, "objects"),
			rdfPropertyName = GetPropertyByName(model, "name"),
			rdfPropertyMaterial = GetPropertyByName(model, "material"),
			owlInstanceCollection = CreateInstance(owlClassCollection, 0),
			rdfPropertyExpressID = GetPropertyByName(model, "expressID");

	int64_t	expressID = 0;
	SetDatatypeProperty(owlInstanceCollection, rdfPropertyExpressID, &expressID, 1);

	int	k = 0;
	while (k < noElements) {
//		objects[k] = globalObjects[k];
		k++;
	}

	for (int i = 0; i < objectsCard; i++) {
		int64_t	myCollection = CreateInstance(owlClassCollection, 0);
		SetObjectProperty(myCollection, rdfPropertyObjects, &objects[i], 1);
		SetDatatypeProperty(myCollection, rdfPropertyName, &objectsID[i], 1);

		int64_t	card = 0, *values = nullptr;
		GetObjectProperty(objects[i], rdfPropertyMaterial, &values, &card);
		if (card == 1) {
			SetObjectProperty(myCollection, rdfPropertyMaterial, values, 1);
		}
		else {
			assert(false);
		}

		objects[i] = myCollection;
	}

//	SetObjectTypeProperty(owlInstanceCollection, rdfPropertyObjects, objects, objectsCard);
	delete[]  objects;

	return	owlInstanceCollection;
}

int64_t	parseLibraryVisualScenes(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
	int64_t	owlInstance = 0;

	if (item->key == GetKey((char*) "library_visual_scenes")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "visual_scene")) {
				assert(owlInstance == 0);
				owlInstance = parseVisualScene(model, items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}

	return	owlInstance;
}

char	* parseTexture(STRUCT_ITEM * item, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "texture")) {
		char		* texture = nullptr;
		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "texture")) {
				assert(texture == nullptr);

				IMAGE	* image = GetImage(attr->value, newParam);

				if (image) {
					texture = image->fileName;
				}
			}
			else if (attr->key == GetKey((char*) "texcoord")) {
				//...
			}
			else {
				assert(false);
			}
			attr = attr->next;
		}

		assert(texture);
		return	texture;
	}
	assert(false);

	return	0;
}

double	* parseColor(STRUCT_ITEM * item)
{
	if	(item->key == GetKey((char*) "color")) {
///		assert(item->attr  &&  item->attr->next == nullptr  &&  item->child == nullptr  &&  item->value);
///		assert(item->attr->key == GetKey("sid"));

		return	GetDoubles(item->value, 4);
	}
		
	assert(false);
	return	nullptr;
}

double	parseTransparency(STRUCT_ITEM * item)
{
	double	* values = nullptr;

	if (item->key == GetKey((char*) "transparency")) {
		///		assert(item->attr  &&  item->attr->next == nullptr  &&  item->child == nullptr  &&  item->value);
		///		assert(item->attr->key == GetKey("sid"));

		double	* values = GetDoubles(item->child->value, 1);

		return	values[0];
	}

	assert(false);
	return	0;
}

double	* parseEmission(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "emission")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
				return	parseColor(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
		
	assert(false);
	return	nullptr;
}

double	* parseAmbient(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "ambient")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
				return	parseColor(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
		
	assert(false);
	return	nullptr;
}

double	* parseTransparent(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "transparent")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
			}
			else if (items->key == GetKey((char*) "texture")) {
				assert(*pTextureName == nullptr);
				*pTextureName = parseTexture(items, newParam);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
				return	parseColor(items);
			}
			else if (items->key == GetKey((char*) "texture")) {
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
		
	assert(false);
	return	nullptr;
}

double	* parseDiffuse(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "diffuse")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
			}
			else if (items->key == GetKey((char*) "texture")) {
				assert(*pTextureName == nullptr);
				*pTextureName = parseTexture(items, newParam);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
				return	parseColor(items);
			}
			else if (items->key == GetKey((char*) "texture")) {
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
		
	assert(pTextureName);
	return	nullptr;
}

double	* parseSpecular(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "specular")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "color")) {
				return	parseColor(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
		
	assert(false);
	return	nullptr;
}

CO__LOR	* parsePhong(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "phong")) {
		CO__LOR	* color = new_COLOR(phong);

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "emission")) {
				color->emission = parseEmission(items);
			}
			else if (items->key == GetKey((char*) "ambient")) {
				color->ambient = parseAmbient(items);
			}
			else if (items->key == GetKey((char*) "diffuse")) {
				color->diffuse = parseDiffuse(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "specular")) {
				color->specular = parseSpecular(items);
			}
			else if (items->key == GetKey((char*) "shininess")) {
				//...
			}
			else if (items->key == GetKey((char*) "reflective")) {
				//...
			}
			else if (items->key == GetKey((char*) "reflectivity")) {
				//...
			}
			else if (items->key == GetKey((char*) "transparent")) {
				//...
			}
			else if (items->key == GetKey((char*) "transparency")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

CO__LOR	* parseConstant(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "constant")) {
		CO__LOR	* color = new_COLOR(constant);

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "emission")) {
				color->emission = parseEmission(items);
			}
			else if (items->key == GetKey((char*) "ambient")) {
				color->ambient = parseAmbient(items);
			}
			else if (items->key == GetKey((char*) "diffuse")) {
				color->diffuse = parseDiffuse(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "specular")) {
				color->specular = parseSpecular(items);
			}
			else if (items->key == GetKey((char*) "transparent")) {
				color->transparent = parseTransparent(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "transparency")) {
				color->transparency = parseTransparency(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

CO__LOR	* parseLambert(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "lambert")) {
		CO__LOR	* color = new_COLOR(lambert);

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "emission")) {
				color->emission = parseEmission(items);
			}
			else if (items->key == GetKey((char*) "ambient")) {
				color->ambient = parseAmbient(items);
			}
			else if (items->key == GetKey((char*) "diffuse")) {
				color->diffuse = parseDiffuse(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "specular")) {
				color->specular = parseSpecular(items);
			}
			else if (items->key == GetKey((char*) "transparent")) {
				color->transparent = parseTransparent(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "transparency")) {
				color->transparency = parseTransparency(items);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

CO__LOR	* parseBlinn(STRUCT_ITEM * item, char ** pTextureName, NEWPARAM * newParam)
{
	if (item->key == GetKey((char*) "blinn")) {
		CO__LOR	* color = new_COLOR(blinn);

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "emission")) {
				color->emission = parseEmission(items);
			}
			else if (items->key == GetKey((char*) "ambient")) {
				color->ambient = parseAmbient(items);
			}
			else if (items->key == GetKey((char*) "diffuse")) {
				color->diffuse = parseDiffuse(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "specular")) {
				color->specular = parseSpecular(items);
			}
			else if (items->key == GetKey((char*) "transparent")) {
				color->transparent = parseTransparent(items, pTextureName, newParam);
			}
			else if (items->key == GetKey((char*) "transparency")) {
				color->transparency = parseTransparency(items);
			}
			else if (items->key == GetKey((char*) "reflective")) {
				//...
			}
			else if (items->key == GetKey((char*) "reflectivity")) {
				//...
			}
			else if (items->key == GetKey((char*) "shininess")) {
				//...
			}
			else if (items->key == GetKey((char*) "index_of_refraction")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

CO__LOR	* parseTechnique(STRUCT_ITEM * item, NEWPARAM * newParam)
{
	if	(item->key == GetKey((char*) "technique")) {
		CO__LOR	* color = nullptr;
//		bool	hasTexture = false;
		char	* textureName = nullptr;

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "phong")) {
				color = parsePhong(items, &textureName, newParam);
			}
			else if (items->key == GetKey((char*) "blinn")) {
				//...
				color = parseBlinn(items, &textureName, newParam);
			}
			else if (items->key == GetKey((char*) "extra")) {
				//...
//				assert(false);
			}
			else if (items->key == GetKey((char*) "lambert")) {
				color = parseLambert(items, &textureName, newParam);
			}
			else if (items->key == GetKey((char*) "constant")) {
				color = parseConstant(items, &textureName, newParam);
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

//		if (color  &&  hasTexture) {
		if (color  &&  textureName) {
			assert(color->hasTexture == false);
			assert(color->textureName == nullptr);
	//		color->hasTexture = true;
			color->textureName = textureName;
			assert(textureName);
		}
		else {
//			assert(hasTexture == false);
			assert(textureName == nullptr);
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

NEWPARAM	* parseNewparam(STRUCT_ITEM * item)
{
	NEWPARAM	* newparam = new NEWPARAM;
	newparam->sid = nullptr;
	newparam->value = nullptr;
	newparam->next = nullptr;

	if (item->key == GetKey((char*) "newparam")) {
		assert(item->attr->key == GetKey((char*) "sid")  &&  item->attr->next == nullptr);
		assert(item->child  &&  item->child->next == nullptr);

		newparam->sid = item->attr->value;

		STRUCT_ITEM	* subItem = item->child;
		if (subItem->key == GetKey((char*) "surface")) {
			assert(subItem->attr->key == GetKey((char*) "type")  &&  subItem->attr->next == nullptr);
//			assert(subItem->child  &&  subItem->child->next == nullptr);
			assert(subItem->child);
			STRUCT_ITEM	* subSubItem = subItem->child;
			
			assert(subSubItem->key == GetKey((char*) "init_from"));
			newparam->value = subSubItem->value;
			if (subSubItem->next) {
				assert(subSubItem->next->key == GetKey((char*) "format"));
			}
		}
		else if (subItem->key == GetKey((char*) "sampler2D")) {
			assert(subItem->attr == nullptr);
//			assert(subItem->child  &&  subItem->child->next == nullptr);
			assert(subItem->child);
			STRUCT_ITEM	* subSubItem = subItem->child;
			
			assert(subSubItem->key == GetKey((char*) "source"));
			newparam->value = subSubItem->value;
		}
		else {
			assert(false);
		}
	} else {
		assert(false);
	}

	return	newparam;
}

CO__LOR	* parseProfileCOMMON(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "profile_COMMON")) {
		NEWPARAM	* newParam = nullptr;
		CO__LOR		* color = nullptr;

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "technique")) {
//				color = parseTechnique(items, newParam);
//				if	(color) {
//					color->newparam = newParam;
//				} else {
//					assert(false);
//				}
			}
			else if (items->key == GetKey((char*) "extra")) {
				//...
			}
			else if (items->key == GetKey((char*) "newparam")) {
				NEWPARAM	* newparam = parseNewparam(items);
				assert(newparam->sid  &&  newparam->value);

				newparam->next = newParam;
				newParam = newparam;
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "technique")) {
				color = parseTechnique(items, newParam);
				if (color) {
					color->newparam = newParam;
				}
				else {
					assert(false);
				}
			}
			else if (items->key == GetKey((char*) "extra")) {
				//...
			}
			else if (items->key == GetKey((char*) "newparam")) {
//				NEWPARAM	* newparam = parseNewparam(items);
//				assert(newparam->sid  &&  newparam->value);
//
//				newparam->next = newParam;
//				newParam = newparam;
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

CO__LOR	* parseEffect(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "effect")) {
		CO__LOR	* color = nullptr;

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "profile_COMMON")) {
				color = parseProfileCOMMON(items);
				assert(color);
			}
			else if (items->key == GetKey((char*) "extra")) {
				//....
//				...
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	color;
	}

	assert(false);
	return	nullptr;
}

void	parseLibraryEffects(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
	if (item->key == GetKey((char*) "library_effects")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "effect")) {
				CO__LOR	* color = parseEffect(items);

				if (color) {
					if (items->attr->key == GetKey((char*) "id")) {
						color->id = items->attr->value;
					}
					else {
						assert(false);
					}

//color->ambient = nullptr;

					SetColor(model, color);
				}

				int u = 0;
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

char	* UpdateImagePath(char * inputPath)
{
	size_t	i = strlen(inputPath);
	char	* correctPath = new char[i + 1];
	memcpy(correctPath, inputPath, i + 1 * sizeof(char));

	size_t	j = 0, firstOpen = -1, lastClose = -1;
	while (j < i) {
		if (correctPath[j] == '[' && firstOpen == -1) {
			firstOpen = j;
		}
		if (correctPath[j] == ']') {
			lastClose = j;
		}
		j++;
	}

	if (firstOpen >= 0 && lastClose >= 0 && firstOpen < lastClose) {
		correctPath[lastClose] = 0;
		correctPath = UpdateImagePath(&correctPath[firstOpen + 1]);
	}

	return	correctPath;
}

IMAGE	* parseImage(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "image")) {
		IMAGE	* image = new IMAGE;
		image->id = nullptr;
		image->fileName = nullptr;
		image->next = nullptr;

		if (item->attr->key == GetKey((char*) "id")) {
			image->id = item->attr->value;
		}
		else {
			assert(false);
		}

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "init_from")) {
				image->fileName = UpdateImagePath(items->value);
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	image;
	}

	assert(false);
	return	nullptr;
}

void	parseLibraryImages(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "library_images")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "image")) {
				IMAGE	* image = parseImage(items);

				if (image) {
					SetImage(image);
				}
				else {
					assert(false);
				}
/*				if	(color) {
					if	(items->attr->key == GetKey("id")) {
						color->id = items->attr->value;
					} else {
						assert(false);
					}
					SetColor(model, color);
				}	//	*/
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

MA__TERIAL	* parseMaterial(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "material")) {
		MA__TERIAL	* material = new MA__TERIAL;

		material->id = nullptr;
		material->color = nullptr;
		material->next = nullptr;

		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "instance_effect")) {
				assert(items->child == nullptr);
				assert(items->attr  &&  items->attr->next == nullptr);
				if (items->attr->key == GetKey((char*) "url")  &&  items->attr->value[0] == '#') {
					material->color = GetColor(&items->attr->value[1]);
					assert(material->color);
				}
				else {
					assert(false);
				}
			}
			else {
				assert(false);
			}
			items = items->next;
		}

		return	material;	
	}

	assert(false);
	return	nullptr;
}

void	parseLibraryMaterials(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "library_materials")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "material")) {
				MA__TERIAL	* material = parseMaterial(items);

				if (items->attr->key == GetKey((char*) "id")) {
					material->id = items->attr->value;
				}
				else {
					assert(false);
				}

				SetMaterial(material);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

void	parseLibraryNodes(STRUCT_ITEM * item)
{
	if (item->key == GetKey((char*) "library_nodes")) {
		STRUCT_ITEM	* items = item->child;
		while (items) {
			if (items->key == GetKey((char*) "node")) {
				NODE_REF * nodeRef = new NODE_REF;

				nodeRef->items = items;

				if (items->attr->key == GetKey((char*) "id")) {
					nodeRef->id = items->attr->value;
				}
				else {
					assert(false);
				}

				SetNodeRef(nodeRef);
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}
}

STRUCT_UNIT	* parseUnit(STRUCT_ITEM * item)
{
	STRUCT_UNIT	* unit = new STRUCT_UNIT;
	unit->factor = 0;

	if (item->key == GetKey((char*) "unit")) {
		assert(item->child == nullptr);
		
		char	* name = nullptr;

		STRUCT_ATTR	* attr = item->attr;
		while (attr) {
			if (attr->key == GetKey((char*) "meter")) {
				unit->factor = atof(attr->value);
			}
			else if (attr->key == GetKey((char*) "name")) {
				name = attr->value;		
			}
			else {
				assert(false);
			}
			attr = attr->next;
		}

		assert(equal(name, (char*) "meter") || equal(name, (char*) "millimeter") || equal(name, (char*) "inch") || equal(name, (char*) "feet"));
	}

	return	unit;
}

STRUCT_UNIT	* parseAsset(STRUCT_ITEM * item)
{
	STRUCT_UNIT	* unit = nullptr;

	if	(item->key == GetKey((char*) "asset")) {
		assert(item->attr == nullptr);

		STRUCT_ITEM	* items = item->child;
		while  (items) {
			if (items->key == GetKey((char*) "contributor")) {
				//...
			}
			else if (items->key == GetKey((char*) "created")) {
				//...
			}
			else if (items->key == GetKey((char*) "modified")) {
				//...
			}
			else if (items->key == GetKey((char*) "unit")) {
				unit = parseUnit(items);
			}
			else if (items->key == GetKey((char*) "up_axis")) {
				//...
			}
			else if (items->key == GetKey((char*) "revision")) {
				//...
			}
			else {
				assert(false);
			}
			items = items->next;
		}
	}
	else {
		assert(false);
	}

	return	unit;
}

int64_t	parseCOLLADA(
				int64_t		model,
				STRUCT_ITEM * item,
				STRUCT_UNIT ** pUnit
			)
{
	isProcessingFIRST = true;

	int64_t	owlInstance = 0;
	if (item->key == GetKey((char*) "COLLADA")) {
		assert(item->next == nullptr);
		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
				if (items->key == GetKey((char*) "asset")) {
					//...
				}
				else if (items->key == GetKey((char*) "library_effects")) {
					//...
				}
				else if (items->key == GetKey((char*) "library_images")) {
					//...
				}
				else if (items->key == GetKey((char*) "library_geometries")) {
					//...
				}
				else if (items->key == GetKey((char*) "library_materials")) {
					//...
				}
				else if (items->key == GetKey((char*) "library_visual_scenes")) {
					//...
				}

				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
				if (items->key == GetKey((char*) "asset")) {
					(*pUnit) = parseAsset(items);
				}

				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while  (items) {
				if	(items->key == GetKey((char*) "library_nodes")) {
					parseLibraryNodes(items);
				}
			
				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
//				if (items->key == GetKey((char*) "library_effects")) {
//					parseLibraryEffects(items);
//				}
				if (items->key == GetKey((char*) "library_images")) {
					parseLibraryImages(items);
				}
//				if (items->key == GetKey((char*) "library_geometries")) {
//					parseLibraryGeometries(items);
//				}
//				if (items->key == GetKey((char*) "library_materials")) {
//					parseLibraryMaterials(items);
//				}
//				if (items->key == GetKey((char*) "library_visual_scenes")) {
//					assert(owlInstance == 0);
//					owlInstance = parseLibraryVisualScenes(model, items);
//				}
			
				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
				if (items->key == GetKey((char*) "library_effects")) {
					parseLibraryEffects(model, items);
				}
//				if (items->key == GetKey((char*) "library_images")) {
//					parseLibraryImages(items);
//				}
//				if (items->key == GetKey((char*) "library_geometries")) {
//					parseLibraryGeometries(items);
//				}
//				if (items->key == GetKey((char*) "library_materials")) {
//					parseLibraryMaterials(items);
//				}
//				if (items->key == GetKey((char*) "library_visual_scenes")) {
//					assert(owlInstance == 0);
//					owlInstance = parseLibraryVisualScenes(model, items);
//				}
			
				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
//				if (items->key == GetKey((char*) "library_effects")) {
//					parseLibraryEffects(items);
//				}
//				if (items->key == GetKey((char*) "library_images")) {
//					parseLibraryImages(items);
//				}
				if (items->key == GetKey((char*) "library_geometries")) {
					parseLibraryGeometries(model, items);
				}
				if (items->key == GetKey((char*) "library_materials")) {
					parseLibraryMaterials(items);
				}
//				if (items->key == GetKey((char*) "library_visual_scenes")) {
//					assert(owlInstance == 0);
//					owlInstance = parseLibraryVisualScenes(model, items);
//				}
			
				items = items->next;
			}
		}

		{
			STRUCT_ITEM	* items = item->child;
			while (items) {
//				if (items->key == GetKey((char*) "library_effects")) {
//					parseLibraryEffects(items);
//				}
//				if (items->key == GetKey((char*) "library_images")) {
//					parseLibraryImages(items);
//				}
//				if (items->key == GetKey((char*) "library_geometries")) {
//					parseLibraryGeometries(items);
//				}
//				if (items->key == GetKey((char*) "library_materials")) {
//					parseLibraryMaterials(items);
//				}
				if (items->key == GetKey((char*) "library_visual_scenes")) {
					assert(owlInstance == 0);
					owlInstance = parseLibraryVisualScenes(model, items);
				}
			
				items = items->next;
			}
		}
	} else {
		assert(false);
	}

	return	owlInstance;
}

int		globalcnt = 0;
int64_t	globalBuff[49999];

void	InitializeGeometry_DAE(
				int64_t		model,
				STRUCT_ITEM * item
			)
{
//	rdfModel = model;

//	if	(true) {
//		rdfModel = OpenModel(0);

		globalObjectsNormal = new int64_t[49999];
		globalObjectsInverted = new int64_t[49999];

		STRUCT_UNIT	* unit = nullptr;
		int64_t	owlInstance = parseCOLLADA(model, item, &unit);

printf("bb 1: %d \n", (int) owlInstance);

		SetFormat(model, 64, 64);

int64_t	classCnt = 0, propertyCnt = 0, instanceCnt = 0;
OrderedHandles(model, &classCnt, &propertyCnt, &instanceCnt, 0, 0);

printf("2 SCAL 4\n");
printf("AAB %d - %d \n", (int) classCnt, (int) instanceCnt);


		int64_t	vertexBufferSize = 0, indexBufferSize = 0;
		CalculateInstance(owlInstance, &vertexBufferSize, &indexBufferSize, 0);

		if (vertexBufferSize) {
		//	float	* buffer = new float[(3 + 3 + 2) * vertexBufferSize];

double	minX = 999., minY = 999., minZ = 999.;
double	maxX = -999., maxY = -999., maxZ = -999.;
		//	UpdateInstanceVertexBuffer(owlInstance, buffer);

//			if	(vertexBufferSize  &&  indexBufferSize) {
				float	* vertexBuffer = new float[8 * (int_t) vertexBufferSize];
				UpdateInstanceVertexBuffer(owlInstance, vertexBuffer);

	//			if	(started == false) {
					minX = vertexBuffer[0 * 8 + 0];
					maxX = minX;
					minY = vertexBuffer[0 * 8 + 1];
					maxY = minY;
					minZ = vertexBuffer[0 * 8 + 2];
					maxZ = minZ;
	//				started = true;
	//			}

				for (int i = 0; i < vertexBufferSize; i++) {
					if (minX > vertexBuffer[i * 8 + 0]) { minX = vertexBuffer[i * 8 + 0]; }
					if (minY > vertexBuffer[i * 8 + 1]) { minY = vertexBuffer[i * 8 + 1]; }
					if (minZ > vertexBuffer[i * 8 + 2]) { minZ = vertexBuffer[i * 8 + 2]; }

					if (maxX < vertexBuffer[i * 8 + 0]) { maxX = vertexBuffer[i * 8 + 0]; }
					if (maxY < vertexBuffer[i * 8 + 1]) { maxY = vertexBuffer[i * 8 + 1]; }
					if (maxZ < vertexBuffer[i * 8 + 2]) { maxZ = vertexBuffer[i * 8 + 2]; }
				}

				delete[]  vertexBuffer;
//			}


			int64_t	owlClassCollection = GetClassByName(model, "Collection"),
					rdfPropertyObjects = GetPropertyByName(model, "objects");

			if (GetInstanceClass(owlInstance) == owlClassCollection) {
				int64_t	* values = nullptr, card = 0;
				GetObjectProperty(owlInstance, rdfPropertyObjects, &values, &card);

				int64_t	* buff = new int64_t[(int_t) card];
				int i = 0;
				while (i < card) {
					int64_t	owlClassTransformation = GetClassByName(model, "Transformation"),
							owlClassMatrix = GetClassByName(model, "Matrix"),
							rdfProperty_11 = GetPropertyByName(model, "_11"),
							rdfProperty_12 = GetPropertyByName(model, "_12"),
							rdfProperty_13 = GetPropertyByName(model, "_13"),
							rdfProperty_21 = GetPropertyByName(model, "_21"),
							rdfProperty_22 = GetPropertyByName(model, "_22"),
							rdfProperty_23 = GetPropertyByName(model, "_23"),
							rdfProperty_31 = GetPropertyByName(model, "_31"),
							rdfProperty_32 = GetPropertyByName(model, "_32"),
							rdfProperty_33 = GetPropertyByName(model, "_33"),
							rdfProperty_41 = GetPropertyByName(model, "_41"),
							rdfProperty_42 = GetPropertyByName(model, "_42"),
							rdfProperty_43 = GetPropertyByName(model, "_43"),
							rdfPropertyMaterial = GetPropertyByName(model, "material"),
							rdfPropertyMatrix = GetPropertyByName(model, "matrix"),
							rdfPropertyObject = GetPropertyByName(model, "object");

					int64_t	owlInstanceTransformation = CreateInstance(owlClassTransformation, 0),
							owlInstanceMatrix = CreateInstance(owlClassMatrix, 0);

					if (GetInstanceClass(values[i]) == owlClassCollection) {
						int64_t	* valuesSub = nullptr, cardSub = 0;
						GetObjectProperty(values[i], rdfPropertyObjects, &valuesSub, &cardSub);
						if (cardSub == 1) {
							buff[i] = values[i];
							values[i] = valuesSub[0];
							SetObjectProperty(buff[i], rdfPropertyObjects, &owlInstanceTransformation, 1);
							//		SetObjectTypeProperty(owlInstanceTransformation, rdfPropertyObject, &values[i], 1);

							bool done = false;
							if (GetInstanceClass(values[i]) == owlClassTransformation) {
								int64_t	* valuesSub2 = nullptr, cardSub2 = 0;
								GetObjectProperty(values[i], rdfPropertyObject, &valuesSub2, &cardSub2);
								if (cardSub2 == 1) {
									//	if	(GetInstanceClass(valuesSub2[0]) == owlClassTransformation) {
									int64_t	* valuesSub3 = nullptr, cardSub3 = 0;
									GetObjectProperty(valuesSub2[0], rdfPropertyMaterial, &valuesSub3, &cardSub3);
									if (cardSub3 == 1) {
										SetObjectProperty(buff[i], rdfPropertyMaterial, &valuesSub3[0], 1);
										SetObjectProperty(owlInstanceTransformation, rdfPropertyMaterial, &valuesSub3[0], 1);
										done = true;
									}
									//	}
								}
							}
							///////////////assert(done);



						}
						else {
							assert(false);
							buff[i] = owlInstanceTransformation;
							//		SetObjectTypeProperty(owlInstanceTransformation, rdfPropertyObject, &values[i], 1);
						}
					}
					else {
						buff[i] = owlInstanceTransformation;
						//	SetObjectTypeProperty(owlInstanceTransformation, rdfPropertyObject, &values[i], 1);
					}

					SetObjectProperty(owlInstanceTransformation, rdfPropertyObject, &values[i], 1);
					SetObjectProperty(owlInstanceTransformation, rdfPropertyMatrix, &owlInstanceMatrix, 1);

					int64_t	card__ = 0, *values__ = nullptr;
					GetObjectProperty(values[i], rdfPropertyMaterial, &values__, &card__);
					if (card__ == 1) {
						SetObjectProperty(owlInstanceTransformation, rdfPropertyMaterial, values__, 1);
					}
					else {
						assert(false);
					}


					double	valueZero = 0, valueOne = 1,
						//							X = -minX, Y = -minY, Z = -minZ;
						X = -(minX + maxX) / 2, Y = -(minY + maxY) / 2, Z = 0;// -(minZ + maxZ) / 2;





					int64_t	owlInstanceColl = CreateInstance(GetClassByName(model, "Collection"), 0);
					SetObjectProperty(owlInstanceColl, GetPropertyByName(model, "objects"), &buff[i], 1);

					char	* _name = (char*) "object";
					SetDatatypeProperty(owlInstanceColl, GetPropertyByName(model, "name"), &_name, 1);

					globalBuff[globalcnt] = owlInstanceColl;	//  owlInstanceMatrix
					globalcnt++;
					//					double	Pi = 3.14159,
					//							angle = 141.2 * (2 * Pi / 360),
					//							valueCOS = cos(angle),
					//							valueSIN = sin(angle),
					//							valueMINSIN = -valueSIN;

					//					X = 41.1;
					//					Y = 3.7;
					//					Z = 0;
					assert(unit->factor);
					double	Pi = 3.14159,
						valueLocalScale = unit->factor,
						angle = 0 * (2 * Pi / 360);

					valueLocalScale *= 300. / 254.;

	///				if (myMatrix._33 != 1) {
//						valueLocalScale *= myMatrix._33;
	///				}

					int	kavel = 5;// 5;// 0;

					switch (kavel) {
						case 1:
							//
							//	KAVEL 1
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 3170;
							Y = 1050;
							Z = 0;
							break;
						case 2:
							//
							//	KAVEL 2
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 4030;
							Y = 1020;
							Z = 0;
							break;
						case 3:
							//
							//	KAVEL 3
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 3180;
							Y = 450;
							Z = 0;
							break;
						case 4:
							//
							//	KAVEL 4
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 4110;
							Y = 370;
							Z = 0;
							break;
						case 5:
							//
							//	KAVEL 5
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 3175;
							Y = -60;
							Z = 0;
							break;
						case 6:
							//
							//	KAVEL 6
							//
							valueLocalScale = 0.03;// 0.0254 * 3;
							angle = 141.2 * (2 * Pi / 360);
							X = 4110;
							Y = -240;
							Z = 0;
							break;
						default:
							break;
					}


					double	valueCOS = cos(angle) * valueLocalScale,
							valueSIN = sin(angle) * valueLocalScale,
							valueMINSIN = -valueSIN;

					X *= valueLocalScale;
					Y *= valueLocalScale;
					Z *= valueLocalScale;

					double	myMatrix_11 = myMatrix._11 * valueLocalScale,
							myMatrix_12 = myMatrix._12 * valueLocalScale,
							myMatrix_13 = myMatrix._13 * valueLocalScale,
							myMatrix_21 = myMatrix._21 * valueLocalScale,
							myMatrix_22 = myMatrix._22 * valueLocalScale,
							myMatrix_23 = myMatrix._23 * valueLocalScale,
							myMatrix_31 = myMatrix._31 * valueLocalScale,
							myMatrix_32 = myMatrix._32 * valueLocalScale,
							myMatrix_33 = myMatrix._33 * valueLocalScale;

					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_11, &valueCOS, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_12, &valueSIN, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_13, &valueZero, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_21, &valueMINSIN, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_22, &valueCOS, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_23, &valueZero, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_31, &valueZero, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_32, &valueZero, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_33, &valueLocalScale, 1);

/*
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_11, &myMatrix_11, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_12, &myMatrix_12, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_13, &myMatrix_13, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_21, &myMatrix_21, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_22, &myMatrix_22, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_23, &myMatrix_23, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_31, &myMatrix_31, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_32, &myMatrix_32, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_33, &myMatrix_33, 1);
//	*/

					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_41, &X, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_42, &Y, 1);
					SetDatatypeProperty(owlInstanceMatrix, rdfProperty_43, &Z, 1);


if (false) {
//if (true) {
double valueLocalScaleMinus = -valueLocalScale;
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_11, &valueZero, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_12, &valueLocalScale, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_13, &valueZero, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_21, &valueZero, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_22, &valueZero, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_23, &valueLocalScale, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_31, &valueLocalScale, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_32, &valueZero, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_33, &valueZero, 1);

X = -(minX + maxX) / 2;
Y = -(minY + maxY) / 2;
Z = -(minZ + maxZ) / 2;
X *= valueLocalScale;
Y *= valueLocalScale;
Z *= valueLocalScale;

//Z = 0;
//Y = 0;
Y = -minY * valueLocalScale;

SetDatatypeProperty(owlInstanceMatrix, rdfProperty_41, &Z, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_42, &X, 1);
SetDatatypeProperty(owlInstanceMatrix, rdfProperty_43, &Y, 1);
}

//					buff[i] = owlInstanceTransformation;

					i++;
				}

				SetObjectProperty(owlInstance, rdfPropertyObjects, buff, card);

				int u = 0;
			}
			else {
				assert(false);
			}

			int u = 0;
		}
		else {
/////200180711///			assert(false);
		}





		int64_t	objectsCard = globalcnt,
				owlClassCollection = GetClassByName(model, "Collection"),
				rdfPropertyObjects = GetPropertyByName(model, "objects"),
				owlInstanceCollection = CreateInstance(owlClassCollection, 0),
				rdfPropertyExpressID = GetPropertyByName(model, "expressID");


		int64_t	expressID = 1;
		SetDatatypeProperty(owlInstanceCollection, rdfPropertyExpressID, &expressID, 1);

		if (objectsCard > 100) {
//			objectsCard = 100;
		}

		SetObjectProperty(owlInstanceCollection, rdfPropertyObjects, globalBuff, objectsCard);


	///	SaveModelW(rdfModel, outputFileName);

		int u = 0;
//	} else {
//		rdfModel = OpenModel("c:\\1\\input1.bin");
//		SaveModel(rdfModel, "c:\\1\\input2.bin");
//	}


		assert(GET_INVERTED == false);
		GET_INVERTED = false;

//		extern	MATRIX	myMatrix;
//		int64_t	noElements = 0;
//		int64_t	rdfModel = OpenModel(0), *globalObjectsNormal = nullptr, *globalObjectsInverted = nullptr;
		noElements = 0;
//		rdfModel = 0;
		globalObjectsNormal = nullptr;
		globalObjectsInverted = nullptr;

//		NODE_REF	* nodeRefs = nullptr;
		nodeRefs = nullptr;

//		MESH	* meshes = nullptr;
		meshes = nullptr;

//		IMAGE		* images = nullptr;
		images = nullptr;

//		COLOR				* colors = nullptr;
		colors = nullptr;

//		MATERIAL			* materials = nullptr;
		materials = nullptr;

//		bool	DUPLICATE_FACES = false;
		DUPLICATE_FACES = false;
	
//		bool	MERGE_FACES = false;
		MERGE_FACES = false;

//		int		listIndex = 0,
		listIndex = 0;

//		STRUCT_KEY		* firstKey = nullptr;
		firstKey = nullptr;

//	return	model;
}

void	InitializeGeometry_DAE_sub(
				int64_t		model,
				const char	* inputFileName
			)
{
	STRUCT_ITEM	* item = parseXML(inputFileName);

	InitializeGeometry_DAE(
			model,
			item
		);
}

void	InitializeGeometry_DAE_sub(
				int64_t			model,
				const wchar_t	* inputFileName
			)
{
	STRUCT_ITEM	* item = parseXML(inputFileName);

	InitializeGeometry_DAE(
			model,
			item
		);
}
