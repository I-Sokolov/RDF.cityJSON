
#include "stdafx.h"

#include "stdlib.h"

#include "pg_dae_generic.h"



bool	equal(
				char	* strI,
				char	* strII
			)
{
	if (strI && strII) {
		size_t	i = 0;
		while (strI[i] && strII[i] && strI[i] == strII[i]) {
			i++;
		}

		if (strI[i] == 0 && strII[i] == 0) {
			return	true;
		}
	}

	return	false;
}

void	UpdateString(
				char	* str
			)
{
	if (str) {
		size_t	i = 0;
		while (str[i]) {
			if ((str[i + 0] == '%') &&
				(str[i + 1] == '2') &&
				(str[i + 2] == '0')) {
				str[i] = ' ';
				i++;
				while (str[i + 2]) {
					str[i] = str[i + 2];
					i++;
				}
				str[i] = 0;
				return	UpdateString(str);
			}
			i++;
		}
	}
}
