
#include "stdafx.h"
#include "pg_dae_material.h"
#include "pg_dae_image.h"

#include	"../../include/engine.h"

#include <cstdio>
#include <cstring>
#include <assert.h>



CO__LOR				* colors = nullptr;
MA__TERIAL			* materials = nullptr;



CO__LOR	* new_COLOR(
				colorType	type
			)
{
	CO__LOR	* color = new CO__LOR;

	color->type = type;
	switch (type) {
	case blinn:
	case constant:
	case lambert:
	case phong:
		color->id = nullptr;
		color->emission = nullptr;
		color->ambient = nullptr;
		color->diffuse = nullptr;
		color->specular = nullptr;
		color->rdfInstance = 0;
		color->newparam = nullptr;
		color->next = nullptr;
		color->hasTexture = false;
		color->textureName = nullptr;
		color->transparent = nullptr;
		color->transparency = 1;
		break;
	default:
		assert(false);
		break;
	}

	return	color;
}

CO__LOR	* GetColor(
				char	* id
			)
{
	CO__LOR	* color = colors;
	while (color) {
		if (equal(color->id, id)) {
			return	color;
		}
		color = color->next;
	}

	/////200180711///assert(false);
	return	nullptr;
}

void	Rescale(
				double	* colorComponent,
				double	scale
			)
{
	if (colorComponent) {
		if (colorComponent[0] < 0.99) {
			colorComponent[0] *= scale;
		}
		if (colorComponent[1] < 0.99) {
			colorComponent[1] *= scale;
		}
		if (colorComponent[2] < 0.99) {
			colorComponent[2] *= scale;
		}
		if (colorComponent[3] < 0.99) {
			colorComponent[3] *= scale;
		}
	}
}

void	SetColor(
				int64_t	model,
				CO__LOR	* color
			)
{
	CO__LOR	* colorIterator = colors;
	while  (colorIterator) {
		if	(equal(colorIterator->id, color->id)) {
			assert(false);
		}
		colorIterator = colorIterator->next;
	}

	assert(color->next == nullptr);
	color->next = colors;

	switch (color->type) {
	case constant:
		if (color->transparent  &&  color->textureName == nullptr  &&  color->diffuse == nullptr) {
			color->diffuse = color->transparent;
		}
		break;
	case blinn:
	case lambert:
	case phong:
		break;
	default:
		assert(false);
		break;
	}

	switch (color->type) {
	case blinn:
	case constant:
	case lambert:
	case phong:
		{
			Rescale(color->ambient, 0.2);
			Rescale(color->diffuse, 0.2);
			Rescale(color->emission, 0.2);
			Rescale(color->specular, 0.2);
		}
		break;
	default:
		assert(false);
		break;
	}

	switch (color->type) {
	case blinn:
	case constant:
	case lambert:
	case phong:
		{
			if (color->ambient == 0 && color->diffuse && color->emission == 0) {
				color->ambient = color->diffuse;
			}

			if (color->specular == 0) {
				color->specular = color->ambient;
			}

			int u = 0;
		}
		break;
	default:
		assert(false);
		break;
	}

	int64_t	owlClassColor = GetClassByName(model, "Color"),
			owlClassColorComponent = GetClassByName(model, "ColorComponent"),
			owlClassMaterial = GetClassByName(model, "Material"),
			rdfPropertyR = GetPropertyByName(model, "R"),
			rdfPropertyG = GetPropertyByName(model, "G"),
			rdfPropertyB = GetPropertyByName(model, "B"),
			rdfPropertyW = GetPropertyByName(model, "W"),
			rdfPropertyColor = GetPropertyByName(model, "color"),
			rdfPropertyAmbient = GetPropertyByName(model, "ambient"),
			rdfPropertyDiffuse = GetPropertyByName(model, "diffuse"),
			rdfPropertySpecular = GetPropertyByName(model, "specular"),
			rdfPropertyEmissive = GetPropertyByName(model, "emissive");

	int64_t	instanceMaterial = CreateInstance(owlClassMaterial, nullptr),
			instanceColor = CreateInstance(owlClassColor, nullptr),
			instanceColorComponentAmbient = CreateInstance(owlClassColorComponent, nullptr),
			instanceColorComponentDiffuse = CreateInstance(owlClassColorComponent, nullptr),
			instanceColorComponentSpecular = CreateInstance(owlClassColorComponent, nullptr),
			instanceColorComponentEmissive = CreateInstance(owlClassColorComponent, nullptr);

/*double	R = newColor->diffuse[0],
		G = newColor->diffuse[1],
		B = newColor->diffuse[2],
		cfac = 5;
newColor->diffuse[0] = 1 - ((1 - R) / cfac);
newColor->diffuse[1] = 1 - ((1 - G) / cfac);
newColor->diffuse[2] = 1 - ((1 - B) / cfac);	//	*/

	SetObjectProperty(instanceMaterial, rdfPropertyColor, &instanceColor, 1);
	SetObjectProperty(instanceColor, rdfPropertyAmbient, &instanceColorComponentAmbient, 1);
	SetObjectProperty(instanceColor, rdfPropertyDiffuse, &instanceColorComponentDiffuse, 1);
	SetObjectProperty(instanceColor, rdfPropertySpecular, &instanceColorComponentSpecular, 1);
	SetObjectProperty(instanceColor, rdfPropertyEmissive, &instanceColorComponentEmissive, 1);

/*	if	(color->ambient == nullptr) {
		color->ambient = new double[4];

		if (color->diffuse != nullptr) {
			// copy diffuse color
			newColor->ambient[0] = newColor->diffuse[0];
			newColor->ambient[1] = newColor->diffuse[1];
			newColor->ambient[2] = newColor->diffuse[2];
			newColor->ambient[3] = newColor->diffuse[3];
		}
		else {
			// white color
			newColor->ambient[0] = 1.0;
			newColor->ambient[1] = 1.0;
			newColor->ambient[2] = 1.0;
			newColor->ambient[3] = 1.0;
		}
	}	//	*/

	double	valueZero = 0.0, valueOne = 1.0;

	if (color->ambient) {
		//	Ambient
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyR, &color->ambient[0], 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyG, &color->ambient[1], 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyB, &color->ambient[2], 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyW, &color->ambient[3], 1);
	}
	else {
		//	Ambient
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyR, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyG, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyB, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyW, &valueOne, 1);
	}

	if (color->transparency != 1) {
		double	rescaled = color->transparency * 0.2;
		SetDatatypeProperty(instanceColorComponentAmbient, rdfPropertyW, &rescaled, 1);
	}

	if (color->diffuse) {
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyR, &color->diffuse[0], 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyG, &color->diffuse[1], 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyB, &color->diffuse[2], 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyW, &color->diffuse[3], 1);		
	}
	else {
		char		* myFile = nullptr;
		NEWPARAM	* newparam = color->newparam;
		if (newparam  &&  newparam->next  &&  !color->textureName) {
			IMAGE	* image = GetImage(newparam->next->value, nullptr);
			if (image  &&  image->fileName) {
				myFile = new char[strlen(image->fileName) + 1];
				memcpy(myFile, image->fileName, strlen(image->fileName) + 1);

				if (myFile[0] == '.' && myFile[1] == '/') {
					memcpy(myFile, &image->fileName[2], strlen(image->fileName) + 1 - 2);
				}
				int u = 0;
				while (myFile[u]) {
					if (myFile[u] == '/') {
						myFile[u] = '\\';
					}
					u++;
				}

				int uu = 0;
			}
			else {
				assert(false);
			}
		}
		else {
//			assert(color->hasTexture  &&  color->textureName);
			assert(color->textureName);
			if (color->textureName) {
				myFile = new char[strlen(color->textureName) + 1];
				memcpy(myFile, color->textureName, strlen(color->textureName) + 1);

				if (myFile[0] == '.' && myFile[1] == '/') {
					memcpy(myFile, &color->textureName[2], strlen(color->textureName) + 1 - 2);
				}
				int u = 0;
				while (myFile[u]) {
					if (myFile[u] == '/') {
						myFile[u] = '\\';
					}
					u++;
				}
			}
//color->hasTexture = false;
			//			assert(false);
		}

		int64_t	owlClassTexture = GetClassByName(model, "Texture"),
				rdfPropertyTextures = GetPropertyByName(model, "textures"),
				rdfPropertyType = GetPropertyByName(model, "type"),
				rdfPropertyOffsetX = GetPropertyByName(model, "offsetX"),
				rdfPropertyOffsetY = GetPropertyByName(model, "offsetY"),
				rdfPropertyScalingX = GetPropertyByName(model, "scalingX"),
				rdfPropertyScalingY = GetPropertyByName(model, "scalingY"),
				rdfPropertyRotation = GetPropertyByName(model, "rotation"),
				rdfPropertyOrigin = GetPropertyByName(model, "origin"),
				rdfPropertyName = GetPropertyByName(model, "name");

		int64_t	instanceTexture = CreateInstance(owlClassTexture, nullptr);

		SetObjectProperty(instanceMaterial, rdfPropertyTextures, &instanceTexture, 1);

		int64_t	__type = 0;
		double	__offset = 0, __scaling = 1, __rotation = 0, __origin[3];
		__origin[0] = 0;
		__origin[1] = 0;
		__origin[2] = 0;

		SetDatatypeProperty(instanceTexture, rdfPropertyType, &__type, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyOffsetX, &__offset, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyOffsetY, &__offset, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyScalingX, &__scaling, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyScalingY, &__scaling, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyRotation, &__rotation, 1);
		SetDatatypeProperty(instanceTexture, rdfPropertyOrigin, &__origin[0], 3);
		if (myFile) {
			SetDatatypeProperty(instanceTexture, rdfPropertyName, &myFile, 1);
		}

		double	value = 0;
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyR, &value, 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyG, &value, 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyB, &value, 1);
		SetDatatypeProperty(instanceColorComponentDiffuse, rdfPropertyW, &value, 1);

		assert(color->hasTexture == false);
		color->hasTexture = true;
	}

	if (color->emission) {
		//	Emissive
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyR, &color->emission[0], 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyG, &color->emission[1], 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyB, &color->emission[2], 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyW, &color->emission[3], 1);
	}
	else {
		//	Emissive
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyR, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyG, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyB, &valueZero, 1);
		SetDatatypeProperty(instanceColorComponentEmissive, rdfPropertyW, &valueOne, 1);
	}

	if (color->specular) {
		//	Specular
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyR, &color->specular[0], 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyG, &color->specular[1], 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyB, &color->specular[2], 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyW, &color->specular[3], 1);
	}
	else {
		double	value = 1;
		//	Specular
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyR, &value, 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyG, &value, 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyB, &value, 1);
		SetDatatypeProperty(instanceColorComponentSpecular, rdfPropertyW, &value, 1);

/*		if (newColor->ambient) {
			double R = newColor->ambient[0],
				G = newColor->ambient[1],
				B = newColor->ambient[2],
				W = newColor->ambient[3];
			SetDataTypeProperty(instanceColorComponentSpecular, rdfPropertyR, &R, 1);
			SetDataTypeProperty(instanceColorComponentSpecular, rdfPropertyG, &G, 1);
			SetDataTypeProperty(instanceColorComponentSpecular, rdfPropertyB, &B, 1);
			SetDataTypeProperty(instanceColorComponentSpecular, rdfPropertyW, &W, 1);
		}	//	*/
	}

	color->rdfInstance = instanceMaterial;
	colors = color;
}

void	SetMaterial(
				MA__TERIAL	* newMaterial
			)
{
	MA__TERIAL	* material = materials;
	while  (material) {
		if	(equal(material->id, newMaterial->id)) {
			assert(false);
		}
		material = material->next;
	}

	assert(newMaterial->next == nullptr);
	newMaterial->next = materials;

	materials = newMaterial;
}

MA__TERIAL	* GetMaterial(
					char	* id
				)
{
	MA__TERIAL	* material = materials;
	while  (material) {
		if	(equal(material->id, id)) {
			return	material;
		}
		material = material->next;
	}

//	assert(false);
	return	nullptr;
}
