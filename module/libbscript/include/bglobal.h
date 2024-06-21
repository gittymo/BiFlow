#ifndef COM_PLUS_MEVANSPN_BSCRIPT_GLOBAL
#define COM_PLUS_MEVANSPN_BSCRIPT_GLOBAL

#define BSCRIPT_DATA_STRUCT_ID (('B' << 24) + ('S' << 16) + ('c' << 8) + 'r')

struct bscript_value;

int BScriptGetHash(char * string);

#endif