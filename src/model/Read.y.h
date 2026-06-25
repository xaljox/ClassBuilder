#define LINE 257
#define START_COMMENT 258
#define END_COMMENT 259
#define START_USER1 260
#define END_USER1 261
#define START_USER2 262
#define END_USER2 263
#define START_USER3 264
#define END_USER3 265
#define START_DECLARATION 266
#define END_DECLARATION 267
#define START_NOTE 268
#define END_NOTE 269
#define START_INIT 270
#define START_CODE 271
#define END_CODE 272
typedef union {
    int ival;
	double dval;
	char* sval;
} READSTYPE;
extern READSTYPE readlval;
