/**
 * @file
 *
 * @brief Source for mathcheck plugin
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */


#ifndef HAVE_KDBCONFIG
# include "kdbconfig.h"
#endif

#include <string.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <kdberrors.h>
#include <ctype.h>
#include "mathcheck.h"
#include "floathelper.h"

#define MIN_VALID_STACK 3
#define EPSILON 0.00001
#define str(s) #s
#define xstr(s) str(s)

typedef enum{ERROR, ADD, SUB, MUL, DIV, NOT, EQU, LT, GT, LE, GE, RES, VAL, END, SET, EMPTY}Operation;
typedef struct{
	double value;
	Operation op;
}PNElem;


int elektraMathcheckGet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned ELEKTRA_UNUSED, Key *parentKey)
{
	if (!strcmp(keyName(parentKey), "system/elektra/modules/mathcheck"))
	{
		KeySet *contract = ksNew (30,
				keyNew ("system/elektra/modules/mathcheck",
					KEY_VALUE, "mathcheck plugin waits for your orders", KEY_END),
				keyNew ("system/elektra/modules/mathcheck/exports", KEY_END),
				keyNew ("system/elektra/modules/mathcheck/exports/get",
					KEY_FUNC, elektraMathcheckGet, KEY_END),
				keyNew ("system/elektra/modules/mathcheck/exports/set",
					KEY_FUNC, elektraMathcheckSet, KEY_END),
#include ELEKTRA_README(mathcheck)
				keyNew ("system/elektra/modules/mathcheck/infos/version",
					KEY_VALUE, PLUGINVERSION, KEY_END),
				keyNew ("system/elektra/modules/mathcheck/export/constants", KEY_END),
				keyNew ("system/elektra/modules/mathcheck/export/constants/EPSILON", KEY_VALUE, xstr(EPSILON), KEY_END),
				KS_END);
		ksAppend (returned, contract);
		ksDel (contract);

		return 1; /* success */
	}

	return 1; /* success */
}
static PNElem nextVal(const PNElem *stackPtr)
{
	PNElem *ptr = (PNElem*)stackPtr;
	PNElem result;
	result.op = ERROR;
	while(ptr->op != END)
	{
		if(ptr->op == VAL)
		{
			ptr->op = EMPTY;
			result.op = VAL;
			result.value = ptr->value;
			break;
		}
		++ptr;
	}
	return result;
}
static PNElem doPrefixCalculation(PNElem *stack, PNElem *stackPtr)
{
	--stackPtr;
	PNElem result;
	if(stackPtr < stack)
		result.op = ERROR;
	while(stackPtr >= stack)
	{	
		if(stackPtr->op == VAL && stackPtr != stack)
		{
			--stackPtr;
			continue;
		}
		else if(stackPtr->op == VAL && stackPtr == stack)
		{
			break;
		}
		PNElem e1 = nextVal(stackPtr);
		PNElem e2 = nextVal(stackPtr);
		if(e1.op == VAL && e2.op == VAL)
		{
			switch(stackPtr->op)
			{
				case ADD:
					stackPtr->value = e1.value + e2.value; 
					stackPtr->op = VAL;
					break;
				case SUB:
					stackPtr->value = e1.value - e2.value; 
					stackPtr->op = VAL;
					break;
				case DIV:
					if(e2.value < EPSILON)
					{
						result.op = ERROR;
						return result;
					}
					stackPtr->value = e1.value / e2.value; 
					stackPtr->op = VAL;
					break;
				case MUL:
					stackPtr->value = e1.value * e2.value; 
					stackPtr->op = VAL;
					break;
				default:
					break;
			}
		}
		else
		{
			return result;
		}
	}
	if(stackPtr->op != VAL)
		result.op = ERROR;
	else
		result.op = RES;
	result.value = stackPtr->value;
	return result;
}
static PNElem parsePrefixString(const char *prefixString, KeySet *ks, Key *parentKey)
{
	const char *regexString = "((([[:alnum:]]*/)*[[:alnum:]]+))|('[0-9]*[.,]{0,1}[0-9]*')|([-+:/<>=!{*])";
	char *ptr = (char *)prefixString;
	regex_t regex;
	Key *key;

	PNElem *stack = malloc(MIN_VALID_STACK*sizeof(PNElem));

	PNElem *stackPtr = stack;
	PNElem result;
	Operation resultOp = ERROR;
	result.op = ERROR;
	int ret;
	if((ret = regcomp(&regex, regexString, REG_EXTENDED|REG_NEWLINE)))
	{
		ksDel(ks);
		return result;
	}
	regmatch_t match;
	int nomatch;
	int start;
	int len;
	char *searchKey = NULL;
	while(1)
	{
		nomatch = regexec(&regex, ptr, 1, &match, 0);
		if(nomatch)
		{
			break;
		}
		len = match.rm_eo - match.rm_so;
		start = match.rm_so + (ptr - prefixString);
		if(len == 1 && !isalpha(prefixString[start]))
		{
			switch(prefixString[start])
			{

				case '+':
					stackPtr->op = ADD;
					++stackPtr;
					break;
				case '-':
					stackPtr->op = SUB;
					++stackPtr;
					break;
				case '/':
					stackPtr->op = DIV;
					++stackPtr;
					break;
				case '*':
					stackPtr->op = MUL;
					++stackPtr;
					break;
				case ':':
					resultOp = SET;
					break;
				case '=':
					if(resultOp == LT)
						resultOp = LE;
					else if(resultOp == GT)
						resultOp = GE;
					else if(resultOp == ERROR)
						resultOp = EQU;
					else if(resultOp == EQU)
						resultOp = EQU;
					else if(resultOp == NOT)
						resultOp = NOT;
					else if(resultOp == SET)
						resultOp = SET;
					break;
				case '<':
					resultOp = LT;
					break;
				case '>':
					resultOp = GT;
					break;
				case '!':
					resultOp = NOT;
					break;
				default:
					ELEKTRA_SET_ERRORF(122, parentKey, "%c isn't a valid operation", prefixString[start]);
					regfree(&regex);
					if(searchKey)
						free(searchKey);
					free(stack);
					ksDel(ks);
					return result;	
					break;
			}
		}
		else
		{
			char *subString = malloc(len+1);
			strncpy(subString, prefixString+start, len);
			subString[len] = '\0';
			if(subString[0] == '\'' && subString[len-1] == '\'')
			{
				subString[len-1] = '\0';
				char *subPtr = (subString+1);
				stackPtr->value = elektraEFtoF(subPtr);
				free(subString);
			}
			else
			{
				ksRewind(ks);
				searchKey = realloc(searchKey, len+2+strlen(keyName(parentKey)));
				strcpy(searchKey, keyName(parentKey));
				strcat(searchKey, "/");
				strcat(searchKey, subString);
				key = ksLookupByName(ks, searchKey, 0);
				if(!key)
				{
					ELEKTRA_SET_ERRORF(124, parentKey, "Operant key %s doesn't exist", searchKey);
					regfree(&regex);
					free(searchKey);
					ksDel(ks);
					free(stack);
					free(subString);
					return result;
				}
				stackPtr->value = elektraEFtoF(keyString(key));
				free(subString);
			}
			stackPtr->op = VAL;
			++stackPtr;
		}
		int offset = stackPtr - stack;
		stack = realloc(stack, (offset + 1)*sizeof(PNElem));
		stackPtr = stack;
		stackPtr += offset;
		ptr += match.rm_eo;
	}
	regfree(&regex);
	free(searchKey);
	ksDel(ks);
	stackPtr->op = END;
	result = doPrefixCalculation(stack, stackPtr);	
	if(result.op != ERROR)
		result.op = resultOp;
	else
		ELEKTRA_SET_ERRORF(122, parentKey, "%s\n", prefixString);
	free(stack);
	return result;
}

int elektraMathcheckSet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned, Key *parentKey)
{
	Key *cur;
	const Key *meta;
	PNElem result;
	while((cur = ksNext(returned)) != NULL)
	{
		meta = keyGetMeta(cur, "check/math");
		if(!meta)
			continue;
		result = parsePrefixString(keyString(meta), ksDup(returned), parentKey);
		char val1[MAX_CHARS_DOUBLE];
		char val2[MAX_CHARS_DOUBLE];
		strncpy(val1, keyString(cur), sizeof(val1));
		elektraFtoA(val2, sizeof(val2), result.value);
		if(result.op == ERROR)
		{
			return 1;
		}
		else if(result.op == EQU)
		{
			if(fabs(elektraEFtoF(keyString(cur)) - result.value) > EPSILON)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s != %s", val1, val2);
				return -1;
			}
		}
		else if(result.op == NOT)
		{
			if(fabs(elektraEFtoF(keyString(cur)) - result.value) < EPSILON)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s == %s but requirement was !=", val1, val2);
				return -1;
			}
		}
		else if(result.op == LT)
		{
			if(elektraEFtoF(keyString(cur)) >= result.value)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s not < %s", val1, val2);
				return -1;
			}
		}
		else if(result.op == GT)
		{
			if(elektraEFtoF(keyString(cur)) <= result.value)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s not > %s", val1, val2);
				return -1;
			}
		}
		else if(result.op == LE)
		{
			if(elektraEFtoF(keyString(cur)) > result.value)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s not <=  %s", val1, val2);
				return -1;
			}
		}
		else if(result.op == GE)
		{
			if(elektraEFtoF(keyString(cur)) < result.value)
			{
				ELEKTRA_SET_ERRORF(123, parentKey, "%s not >= %s", val1, val2);
				return -1;
			}
		}
		else if(result.op == SET)
		{
			keySetString(cur, val2);
		}
	}
	return 1; /* success */
}

Plugin *ELEKTRA_PLUGIN_EXPORT(mathcheck)
{
	return elektraPluginExport("mathcheck",
			ELEKTRA_PLUGIN_GET,	&elektraMathcheckGet,
			ELEKTRA_PLUGIN_SET,	&elektraMathcheckSet,
			ELEKTRA_PLUGIN_END);
}

