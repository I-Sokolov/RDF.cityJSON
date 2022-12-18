
#include "stdafx.h"
#include "pg_dae_xmlParsing.h"
#include "pg_dae_parse.h"

#include "assert.h"


extern	bool	eof;
STRUCT_KEY		* firstKey = nullptr;

#ifdef _DEBUG
extern	int		TOTAL_LINES_READ,
				TOTAL_CHARS_READ;
#endif // _DEBUG



char	* copy(char * name)
{
	if	(name) {
		int i = 0;
		while  (name[i]) { i++; }
		char	* rValue = new char[i + 1];
		memcpy(rValue, name, (i + 1) * sizeof(char));
		return	rValue;
	}

	assert(false);
	return	nullptr;
}

STRUCT_KEY	* GetKey(
					char	* name
				)
{
	STRUCT_KEY	* myKey = firstKey;
	while  (myKey) {
		if	(equal(name, myKey->name)) {
			return	myKey;
		}
		myKey = myKey->next;
	}

	myKey = new STRUCT_KEY;

	myKey->name = copy(name);
	myKey->next = firstKey;
	firstKey = myKey;

	return	myKey;
}

char	largeArray__[599999];
char	* parseValue(
				FILE	* fp,
				char	endChar
			)
{
	int		i = 0;
	largeArray__[i] = *GetByte(fp);
	while (largeArray__[i] != endChar && !eof) {
		i++;
		largeArray__[i] = *GetByte(fp);
assert(i < 599999);
	}
	largeArray__[i] = 0;

assert(i < 599999);

	return	copy(largeArray__);
}

STRUCT_KEY	* parseKey(
					FILE	* fp
				)
{
	char	name[512];
	switch  (name[0] = *GetByte(fp)) {
		CA_SE_ENTITY_CHAR
		CA_SE_LOWER_CASE
		CA_SE_UPPER_CASE
			{
				int i = 1;
				while  (!eof) {
					switch  (name[i] = *GetByte(fp)) {
						CA_SE_ENTITY_CHAR
						CA_SE_LOWER_CASE
						CA_SE_UPPER_CASE
						CA_SE_NUMBER
							i++;
							break;
						default:
							switch  (name[i]) {
								CA_SE_SPACE
								case  '>':
								case  '=':
								case  '/':
									break;
								default:
									assert(false);
									break;
							}
							name[i] = 0;

							UndoGetByte();
							return	GetKey(name);
					}
				}
				assert(false);
			}
			break;
		default:
			assert(false);
			break;
	}

	return	nullptr;
}

void	parseBracket(FILE * fp, STRUCT_ITEM ** ppItem);

void	parseContent(
				FILE		* fp,
				STRUCT_ITEM * item
			)
{
	assert(item  &&  item->key  &&  item->value == 0  &&  item->child == 0  &&  item->next == 0);
	STRUCT_ITEM	** ppChild = &item->child;

	char	myChar;
	while  (true) {
		switch  (*GetByte(fp)) {
			CA_SE_SPACE;
				break;
			case  '<':
				assert(item->value == 0);
				switch (myChar = *GetByte(fp)) {
					case  '/':
						return;
					CA_SE_ENTITY_CHAR
					CA_SE_LOWER_CASE
					CA_SE_UPPER_CASE
//					case  '!':
						UndoGetByte();
						parseBracket(fp, ppChild);
						if	((*ppChild)) {
							ppChild = &(*ppChild)->next;
						}
						break;
					case  '!':
						{
							assert(item->value == 0 && item->child == 0);
							item->value = parseValue(fp, '<');
							size_t	i = strlen(item->value);
							assert(item->value[i - 1] == '>' && item->value[i] == 0);
							item->value[i - 1] = 0;
							if (*GetByte(fp) != '/') {
								assert(false);
							}
							assert(item->value);
							return;
						}
					default:
						assert(false);
						break;
				}
				break;
			default:
				{
					UndoGetByte();
					assert(item->value == 0  &&  item->child == 0);
					item->value = parseValue(fp, '<');
					if (*GetByte(fp) != '/') {
						assert(false);
					}
					assert(item->value);
					return;
				}
		}
	}
}

STRUCT_ITEM	* newItem(STRUCT_KEY * key)
{
	STRUCT_ITEM	* item = new STRUCT_ITEM;

	item->key = key;
	item->value = nullptr;
	item->attr = nullptr;
	item->child = nullptr;
	item->next = nullptr;

	return	item;
}

STRUCT_ATTR	* newAttr(STRUCT_KEY * key)
{
	STRUCT_ATTR	* attribute = new STRUCT_ATTR;

	attribute->key = key;
	attribute->value = nullptr;
	attribute->next = nullptr;

	return	attribute;
}

void	parseBracket(
				FILE		* fp,
				STRUCT_ITEM	** ppItem
			)
{
	assert(ppItem  &&  (*ppItem) == nullptr);

	bool	closeDirectly = false;
	switch  (*GetByte(fp)) {
		case  '!':
			closeDirectly = true;
			break;
		CA_SE_ENTITY_CHAR
		CA_SE_LOWER_CASE
		CA_SE_UPPER_CASE
			{
				UndoGetByte();
				(*ppItem) = newItem(parseKey(fp));
				STRUCT_ATTR	** ppAttr = &(*ppItem)->attr;

				bool	withinBrackets = true;
				while  (withinBrackets) {
					switch  (*GetByte(fp)) {
						CA_SE_SPACE
							break;
						CA_SE_ENTITY_CHAR
						CA_SE_LOWER_CASE
						CA_SE_UPPER_CASE
							{
								UndoGetByte();
								assert((*ppAttr) == nullptr);
								(*ppAttr) = newAttr(parseKey(fp));
								if ((*GetByte(fp) == '=')  &&
									(*GetByte(fp) == '"') ) {
									(*ppAttr)->value = parseValue(fp, '"');
								}
								else {
									assert(false);
								}
								ppAttr = &(*ppAttr)->next;
							}
							break;
						case  '/':
							closeDirectly = true;
							if	(*GetByte(fp) == '>') {
								withinBrackets = false;
							} else {
								assert(false);
							}
							break;
						case  '>':
							withinBrackets = false;
							break;
						default:
							assert(false);
							break;
					}
				}
				
				if (closeDirectly) {
					return;
				}
				else {
					parseContent(fp, (*ppItem));
					STRUCT_KEY	* closingKey = parseKey(fp);
					if (closingKey != (*ppItem)->key) {
						assert(false);
					}

					if (*GetByte(fp) != '>') {
						assert(false);
					}
					else {
						return;
					}
				}
			}
			break;
		case  '?':
			while (*GetByte(fp) != '?' && !eof) { }

			if (*GetByte(fp) != '>') {
				assert(false);
			}
			break;
		default:
			assert(false);
			break;
	}

	return;
}

void	parse(
				FILE		* fp,
				STRUCT_ITEM ** ppItem
			)
{
#ifdef _DEBUG
	TOTAL_LINES_READ = 0;
	TOTAL_CHARS_READ = 0;
#endif // _DEBUG

	assert(ppItem && (*ppItem) == nullptr);

	while (!eof) {
		char	myChar = 0;
		switch (myChar = *GetByte(fp)) {
			CA_SE_SPACE
				break;
			case  '<':
				assert((*ppItem) == nullptr);
				parseBracket(fp, ppItem);
				break;
			default:
				if (!eof) {
					assert(false);
				}
				break;
		}
	}
}

STRUCT_ITEM	* parseXML(
				const char	* fileName
			)
{
	FILE	* fp = nullptr;
printf("AA 1  \n");
	fp = fopen(fileName, "r");
printf("AA 2  \n");
	if (fp) {
printf("AA 3  \n");
		InitGetByte(fp);

		STRUCT_ITEM	* item = nullptr;
		parse(fp, &item);

		fclose(fp);

		return	item;
	}

	return	nullptr;
}

STRUCT_ITEM	* parseXML(
				const wchar_t	* fileName
			)
{
	FILE	* fp = nullptr;
//	_wfopen_s(&fp, fileName, L"r");
	if (fp) {
		InitGetByte(fp);

		STRUCT_ITEM	* item = nullptr;
		parse(fp, &item);

		fclose(fp);

		return	item;
	}

	return	nullptr;
}
