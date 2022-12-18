
#include "stdafx.h"
#include "pg_dae_parse.h"

#include "assert.h"



#define	LIST_SIZE	4096//65536
char	list[1024], _list[LIST_SIZE + 10];
int		listIndex = 0,
		current_index,
#ifdef _DEBUG
		TOTAL_LINES_READ,
		TOTAL_CHARS_READ,
#endif // _DEBUG
		readPos;
size_t	numread;
bool	eof;
static	char				returned_when_eof = ';';

void	InitGetByte(
				FILE	* fp
			)
{
	numread = fread( _list, sizeof( char ), LIST_SIZE, fp );
	if (numread) {
		listIndex = 0;
		list[0] = _list[listIndex++];
		while (list[0] != '<' && listIndex < LIST_SIZE) {
			list[0] = _list[listIndex++];
		}
		numread--;
	}
	current_index = 0;
	readPos = 0;
	eof = false;
}

char	* GetByte(
				FILE	* fp
			)
{
	if (current_index >= 1) {
		if (numread == 0 || (listIndex < 0  ||  listIndex >= LIST_SIZE)) {
			numread = fread( _list, sizeof( char ), LIST_SIZE, fp );
			listIndex = 0;
			if (numread == 0) {
				eof = true;
				current_index = 1;
				return  &returned_when_eof;
			}
			assert(numread <= LIST_SIZE);
		}
		list[0] = _list[listIndex++];
		numread--;
		current_index = 0;
	}

#ifdef _DEBUG
	if (list[current_index] == 10) {
		TOTAL_LINES_READ++;
		TOTAL_CHARS_READ = 0;
	}
	else {
		TOTAL_CHARS_READ++;
	}
#endif // _DEBUG

	return	&list[current_index++];
}

void	UndoGetByte(
			)
{
	assert(current_index > 0);
	current_index--;
}


