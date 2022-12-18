// dae2bin.h : main header file for the dae2bin DLL
//

#ifndef __RDF_LTD__DAE2BIN_H
#define __RDF_LTD__DAE2BIN_H


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
			);

//
//
//
void	convertDAE_char(
				char		* inputFileName,
				char		* outputFileName,
				double		* matrix
			);

//
//
//
void	convertDAE_wchar_t(
				wchar_t		* inputFileName,
				wchar_t		* outputFileName,
				double		* matrix
			);


#endif
