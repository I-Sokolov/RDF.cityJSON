
#include "stdafx.h"
#include "pg_dae_image.h"
#include "pg_dae_generic.h"

#include "assert.h"


IMAGE		* images = nullptr;


void	SetImage(
				IMAGE	* newImage
			)
{
	IMAGE	* image = images;
	while (image) {
		if (equal(image->id, newImage->id)) {
			assert(false);
		}
		image = image->next;
	}

	assert(newImage->next == nullptr);
	newImage->next = images;

	images = newImage;

	//
	//	Edit char
	//
	UpdateString(newImage->fileName);
}

IMAGE	* GetImage(
				char		* id,
				NEWPARAM	* newParam
			)
{
	if (newParam) {
		bool	found = true;
		while (found) {
			found = false;
			NEWPARAM	* param = newParam;
			while (param) {
				if (equal(param->sid, id)) {
					id = param->value;
				}
				param = param->next;
			}
		}
	}

	IMAGE	* image = images;
	while (image) {
		if (equal(image->id, id)) {
			return	image;
		}
		image = image->next;
	}

	assert(false);
	return	nullptr;
}
