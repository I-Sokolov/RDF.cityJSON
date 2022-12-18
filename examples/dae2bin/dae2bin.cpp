// dae2bin.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "dae2bin.h"
#include "pg_dae_core_sub.h"

#include "engine/include/engine.h"

#include "assert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern	MA__TRIX	myMatrix;

extern	bool		DUPLICATE_FACES,
					MERGE_FACES;


int64_t		flagbit0 = 1;				// 2^^0							 0000.0000..0000.0001
int64_t		flagbit1 = 2;				// 2^^1							 0000.0000..0000.0010
int64_t		flagbit2 = 4;				// 2^^2							 0000.0000..0000.0100
int64_t		flagbit3 = 8;				// 2^^3							 0000.0000..0000.1000

int64_t		flagbit4 = 16;				// 2^^4							 0000.0000..0001.0000
int64_t		flagbit5 = 32;				// 2^^5							 0000.0000..0010.0000
int64_t		flagbit6 = 64;				// 2^^6							 0000.0000..0100.0000
int64_t		flagbit7 = 128;				// 2^^7							 0000.0000..1000.0000

int64_t		flagbit8 = 256;				// 2^^8							 0000.0001..0000.0000
int64_t		flagbit9 = 512;				// 2^^9							 0000.0010..0000.0000
int64_t		flagbit10 = 1024;			// 2^^10						 0000.0100..0000.0000
int64_t		flagbit11 = 2048;			// 2^^11						 0000.1000..0000.0000

int64_t		flagbit12 = 4096;			// 2^^12						 0001.0000..0000.0000
int64_t		flagbit13 = 8192;			// 2^^13						 0010.0000..0000.0000
int64_t		flagbit14 = 16384;			// 2^^14						 0100.0000..0000.0000
int64_t		flagbit15 = 32768;			// 2^^15						 1000.0000..0000.0000

int64_t		flagbit16 = 65536;			// 2^^16   0000.0000..0000.0001  0000.0000..0000.0000
int64_t		flagbit17 = 131072;			// 2^^17   0000.0000..0000.0010  0000.0000..0000.0000
int64_t		flagbit18 = 262144;			// 2^^18   0000.0000..0000.0100  0000.0000..0000.0000
int64_t		flagbit19 = 524288;			// 2^^19   0000.0000..0000.1000  0000.0000..0000.0000

int64_t		flagbit20 = 1048576;		// 2^^20   0000.0000..0001.0000  0000.0000..0000.0000
int64_t		flagbit21 = 2097152;		// 2^^21   0000.0000..0010.0000  0000.0000..0000.0000
int64_t		flagbit22 = 4194304;		// 2^^22   0000.0000..0100.0000  0000.0000..0000.0000
int64_t		flagbit23 = 8388608;		// 2^^23   0000.0000..1000.0000  0000.0000..0000.0000

int64_t		flagbit24 = 16777216;		// 2^^24   0000.0001..0000.0000  0000.0000..0000.0000
int64_t		flagbit25 = 33554432;		// 2^^25   0000.0010..0000.0000  0000.0000..0000.0000
int64_t		flagbit26 = 67108864;		// 2^^26   0000.0100..0000.0000  0000.0000..0000.0000
int64_t		flagbit27 = 134217728;		// 2^^27   0000.1000..0000.0000  0000.0000..0000.0000

int64_t		flagbit28 = 268435456;		// 2^^28   0001.0000..0000.0000  0000.0000..0000.0000
int64_t		flagbit29 = 536870912;		// 2^^29   0010.0000..0000.0000  0000.0000..0000.0000
//int64_t		flagbit30 = 1073741824;		// 2^^30   0100.0000..0000.0000  0000.0000..0000.0000
//int64_t		flagbit31 = 2147483648;		// 2^^31   1000.0000..0000.0000  0000.0000..0000.0000



//
//	bit 0	(2^0 = 1)
//		UNSET	define one face for each triangle (culling has to correct)
//		SET		define both faces of each triangle
//
//	bit 1	(2^1 = 2)
//		UNSET	leave all faces and traingles as defined
//		SET		merge all faces and traingles
//
void	setBehavior(
				int			setting,
				int			mask
			)
{
	if (mask & flagbit0) {
		if (setting & flagbit0) {
			DUPLICATE_FACES = true;
		} else {
			DUPLICATE_FACES = false;
		}
	}

	if (mask & flagbit1) {
		if (setting & flagbit1) {
			MERGE_FACES = true;
		} else {
			MERGE_FACES = false;
		}
	}
}

void	convertDAE_char(
				char		* inputFileName,
				char		* outputFileName,
				double		* matrix
			)
{
	if (matrix) {
		myMatrix._11 = matrix[0];
		myMatrix._12 = matrix[1];
		myMatrix._13 = matrix[2];
		myMatrix._21 = matrix[3];
		myMatrix._22 = matrix[4];
		myMatrix._23 = matrix[5];
		myMatrix._31 = matrix[6];
		myMatrix._32 = matrix[7];
		myMatrix._33 = matrix[8];
		myMatrix._41 = matrix[9];
		myMatrix._42 = matrix[10];
		myMatrix._43 = matrix[11];
	}

	int64_t	myModel = CreateModel();
	
	InitializeGeometry_DAE_sub(myModel, inputFileName);

	SaveModel(myModel, outputFileName);
}

void	convertDAE_wchar_t(
				wchar_t		* inputFileName,
				wchar_t		* outputFileName,
				double		* matrix
			)
{
	if (matrix) {
		myMatrix._11 = matrix[0];
		myMatrix._12 = matrix[1];
		myMatrix._13 = matrix[2];
		myMatrix._21 = matrix[3];
		myMatrix._22 = matrix[4];
		myMatrix._23 = matrix[5];
		myMatrix._31 = matrix[6];
		myMatrix._32 = matrix[7];
		myMatrix._33 = matrix[8];
		myMatrix._41 = matrix[9];
		myMatrix._42 = matrix[10];
		myMatrix._43 = matrix[11];
	}

	int64_t	myModel = CreateModel();
	
	InitializeGeometry_DAE_sub(myModel, inputFileName);

	SaveModelW(myModel, outputFileName);
}
