/*
@author Ondrej Ritomsky
@version 0.42, 29.01.2019

No warranty implied

// ASCI only
// No escaped characters in string

USAGE:
#define JSON_IMPLEMENTATION
before json.h include in one .cpp file

OPTIONS:
Create error message if parsing failed
#define JSON_ERROR_MESSAGE

Destructive parsing for strings, parsed string points to modified input buffer "str" -> "str\0 ...
#define JSON_INPUT_STRING_IS_STORAGE

Custom allocator
#define JSON_ALLOC(context, size) <alloc function call> // malloc(size) by default
#define JSON_FREE(context, data) <free function call>   // free(data) by default

Contributions, bug finding / fixing, improvements
Jakub Lukasik

*/

#ifndef JSON_INCLUDE_JSON_H
#define JSON_INCLUDE_JSON_H

enum JsonValueType : char {
	JSON_VALUE_INVALID,
	JSON_VALUE_ARRAY,
	JSON_VALUE_OBJECT,
	JSON_VALUE_BOOL,
	JSON_VALUE_NULL,
	JSON_VALUE_INT,
	JSON_VALUE_DOUBLE,
	JSON_VALUE_STRING
};

struct Json_Array;

struct JsonValue {
	// Can be safely read, if the JsonValue is properly initialized
	JsonValueType type_;

	bool _allocated; // if array or object have any data allocated or if string is allocated or const

	union {
		bool _boolVal;
		long long _intVal;
		double _doubleVal;
		char* _strVal;
		Json_Array* _arrayVal;
		void* _allocContext; // store allocContext, while there is 0 vals in array or object
	};
};

struct JsonKeyValue {
	JsonValue key;
	JsonValue value;
};

struct JsonPrintContext {
	unsigned int bufferSize;
	unsigned int bufferCapacity;
	char* buffer;
	void* allocContext;
};

extern "C" {

	void JsonDeinit(JsonValue* value);


	// Stream will be modified used only if JSON_INPUT_STRING_IS_STORAGE is used
	bool JsonParse(char* stream, void* allocContext, JsonValue* outValue);

	bool JsonParseError(char* stream, void* allocContext, JsonValue* outValue, char error[64]);


	JsonPrintContext JsonPrintContextInit(void* allocContext);

	// The result data is owned by the context, deinit context after usage
	char* JsonValuePrint(JsonPrintContext* context, const JsonValue* value, bool pretty, unsigned int* optOutSize);

	void JsonPrintContextDeinit(JsonPrintContext* context);



	bool JsonGetBool(const JsonValue* value);

	long long JsonGetInt(const JsonValue* value);

	double JsonGetDouble(const JsonValue* value);

	const char* JsonGetString(const JsonValue* value);



	void JsonSetInvalid(JsonValue* value);

	void JsonSetBool(JsonValue* value, bool b);

	void JsonSetNull(JsonValue* value);

	void JsonSetInt(JsonValue* value, long long number);

	void JsonSetDouble(JsonValue* value, double number);

	void JsonSetString(JsonValue* value, const char* str, void* allocContext);

	void JsonSetCString(JsonValue* value, const char* str);

	void JsonSetArray(JsonValue* value, void* allocContext);

	void JsonSetObject(JsonValue* value, void* allocContext);



	bool JsonIsType(const JsonValue* value, JsonValueType type);

	bool JsonIsInvalid(const JsonValue* value);

	bool JsonIsBool(const JsonValue* value);

	bool JsonIsNull(const JsonValue* value);

	bool JsonIsInt(const JsonValue* value);

	bool JsonIsDouble(const JsonValue* value);

	bool JsonIsString(const JsonValue* value);

	bool JsonIsArray(const JsonValue* value);

	bool JsonIsObject(const JsonValue* value);


	void JsonArrayAdd(JsonValue* array, JsonValue* element);

	void JsonArrayReserve(JsonValue* array, unsigned int capacity);

	unsigned int JsonArrayCount(const JsonValue* array);

	JsonValue* JsonArrayBegin(JsonValue* array);

	const JsonValue* JsonArrayCBegin(const JsonValue* array);

	// Value is deallocated
	bool JsonArraySwapRemove(JsonValue* array, unsigned int index);



	void JsonObjectAdd(JsonValue* object, JsonValue* name, JsonValue* namesValue);

	unsigned int JsonObjectCount(const JsonValue* object);

	JsonKeyValue* JsonObjectBegin(JsonValue* object);

	const JsonKeyValue* JsonObjectCBegin(const JsonValue* object);

	// KeyValue must be valid iterator value from JsonObjectFind, no boundary check is made
	// KeyValue key and value are deallocated
	void JsonObjectRemove(JsonValue* object, JsonKeyValue* keyValue);

	// Returns iterator value or nullptr if not found
	JsonKeyValue* JsonObjectFind(JsonValue* object, const char* name);

	const JsonKeyValue* JsonObjectCFind(const JsonValue* object, const char* name);



}

#endif // JSON_INCLUDE_JSON_H

#if defined(JSON_IMPLEMENTATION)

enum Json_TokenType : short {
	JSON_TOKEN_NONE,
	JSON_TOKEN_EOS,
	JSON_TOKEN_COLON,
	JSON_TOKEN_COMMA,
	JSON_TOKEN_LEFT_BRACE,
	JSON_TOKEN_RIGHT_BRACE,
	JSON_TOKEN_LEFT_BRACKET,
	JSON_TOKEN_RIGHT_BRACKET,
	JSON_TOKEN_INT,
	JSON_TOKEN_DOUBLE,
	JSON_TOKEN_STR,
	JSON_TOKEN_TRUE,
	JSON_TOKEN_FALSE,
	JSON_TOKEN_NULL
};

#include <stdio.h> // vsprintf
#include <math.h>  // HUGE_VAL
#include <ctype.h> // isDigit
#include <stdlib.h> // strtod, strtoull, LLONG_MAX
#include <string.h>  // memcpy 
#include <stdarg.h> // va_args


#ifndef JSON_ALLOC 

#include <malloc.h>

#define JSON_ALLOC(context, size) malloc(size);
#define JSON_FREE(context, data) free(data);

#define JSON_ALLOC_DEFAULT_DEF
#endif // JSON_ALLOC


struct Json_Token {
	Json_TokenType type;
	char* valBegin;
	char* valEnd;
	union {
		char charVal;
		long long intVal;
		double doubleVal;
		const char* strVal;
	};
};


struct Json_ParseContext {
	Json_Token token;  // Lexer
	char* stream;      // Lexer

	void* allocContext;
	char error[64];
};


struct Json_Array {
	unsigned int count;
	unsigned int capacity;
	void* allocContext;
	JsonValue values[1]; // tail [0]
};


static void Json_LogError(Json_ParseContext* context, const char* fmt, ...);

static void Json_LexerInit(Json_ParseContext* context, void* allocContext, char* stream);

static void Json_LexerNextToken(Json_ParseContext* context);

static bool Json_LexerIsToken(Json_ParseContext* context, Json_TokenType type);

static bool Json_LexerMatchToken(Json_ParseContext* context, Json_TokenType type);

static bool Json_LexerExpectToken(Json_ParseContext* context, Json_TokenType type);

static char* Json_LexerParseHexaNumber(Json_ParseContext* context);

static char* Json_LexerParseNumber(Json_ParseContext* context);

static void Json_LexerParseString(Json_ParseContext* context);


static bool Json_ExpectAnyValue(Json_ParseContext* context, JsonValue* outValue);

static bool Json_ExpectString(Json_ParseContext* context, JsonValue* outValue);

static bool Json_MatchString(Json_ParseContext* context, JsonValue* outValue);

static bool Json_MatchObject(Json_ParseContext* context, JsonValue* outValue);

static bool Json_MatchArray(Json_ParseContext* context, JsonValue* outValue);

static bool Json_MatchValue(Json_ParseContext* context, JsonValue* outValue);



static Json_Array* Json_MakeArray(unsigned int capacity, void* allocContext);

static void Json_ArrayMaybeGrow(JsonValue* array, unsigned int grow);

static void Json_PrintFormat(JsonPrintContext* context, const char* format, ...);

static void Json_ValuePrintPretty(JsonPrintContext* context, const JsonValue* value, unsigned int indentAll, unsigned int indentCur);

static void Json_ValuePrint(JsonPrintContext* context, const JsonValue* value);


bool JsonParse(char* stream, void* allocContext, JsonValue* outValue) {
	char error[64];
	return JsonParseError(stream, allocContext, outValue, error);
}

bool JsonParseError(char* stream, void* allocContext, JsonValue* outValue, char error[64]) {
	Json_ParseContext context;
	Json_LexerInit(&context, allocContext, stream);

	if (Json_MatchObject(&context, outValue)) {
		return true;
	}
	else if (Json_MatchArray(&context, outValue)) {
		return true;
	}

	Json_LogError(&context, "JSON: Unknown parse error");
	memcpy(error, context.error, 64);
	return false;
}


char* JsonValuePrint(JsonPrintContext* context, const JsonValue* value, bool pretty, unsigned int* optOutSize) {
	context->bufferSize = 0;

	if (pretty) {
		Json_ValuePrintPretty(context, value, 0, 0);
	}
	else {
		Json_ValuePrint(context, value);
	}

	if (context->bufferSize > 0) {
		if (context->bufferSize == context->bufferCapacity) {
			Json_PrintFormat(context, " ");
			context->buffer[context->bufferSize - 1] = '\0';
		}
	}

	if (optOutSize)
		*optOutSize = context->bufferSize;

	return context->buffer;
}


JsonPrintContext JsonPrintContextInit(void* allocContext) {
	JsonPrintContext printContext;

	printContext.bufferSize = 0;
	printContext.bufferCapacity = 0;
	printContext.buffer = nullptr;
	printContext.allocContext = allocContext;
	return printContext;
}

void JsonPrintContextDeinit(JsonPrintContext* context) {
	if (context->buffer) {
		JSON_FREE(context->allocContext, context->buffer);
		context->buffer = nullptr;
	}
}

void JsonDeinit(JsonValue* value) {
	if (value->_allocated) {
		if (value->type_ == JSON_VALUE_STRING && value->_strVal) {
			JSON_FREE(value->_allocContext, value->_strVal);
		}
		else if (value->type_ == JSON_VALUE_OBJECT || value->type_ == JSON_VALUE_ARRAY) {
			Json_Array* arr = value->_arrayVal;
			JsonValue* end = arr->values + arr->count;
			for (JsonValue* it = arr->values; it != end; it++)
				JsonDeinit(it);

			void* allocContext = arr->allocContext;
			JSON_FREE(allocContext, arr);
		}
	}
	value->type_ = JSON_VALUE_INVALID;
}

JsonKeyValue* JsonObjectBegin(JsonValue* value) {
	return value->_allocated ? (JsonKeyValue*)value->_arrayVal->values : nullptr;
}

const JsonKeyValue* JsonObjectCBegin(const JsonValue* value) {
	return value->_allocated ? (JsonKeyValue*)value->_arrayVal->values : nullptr;
}

JsonValue* JsonArrayBegin(JsonValue* value) {
	return value->_allocated ? value->_arrayVal->values : nullptr;
}

const JsonValue* JsonArrayCBegin(const JsonValue* value) {
	return value->_allocated ? value->_arrayVal->values : nullptr;
}

bool JsonArraySwapRemove(JsonValue* value, unsigned int index) {
	if (value->_allocated && index < value->_arrayVal->count) {
		Json_Array* arr = value->_arrayVal;
		JsonDeinit(&arr->values[index]);
		arr->values[index] = arr->values[arr->count - 1];
		arr->count--;
		return true;
	}
	return false;
}

bool JsonGetBool(const JsonValue* value) {
	return value->_boolVal;
}

long long JsonGetInt(const JsonValue* value) {
	return value->_intVal;
}

double JsonGetDouble(const JsonValue* value) {
	return value->_doubleVal;
}

const char* JsonGetString(const JsonValue* value) {
	return value->_strVal;
}

void JsonSetInvalid(JsonValue* value) {
	value->type_ = JSON_VALUE_INVALID;
}

void JsonSetBool(JsonValue* value, bool b) {
	value->type_ = JSON_VALUE_BOOL;
	value->_boolVal = b;
}

void JsonSetNull(JsonValue* value) {
	value->type_ = JSON_VALUE_INVALID;
}

void JsonSetInt(JsonValue* value, long long number) {
	value->type_ = JSON_VALUE_INT;
	value->_intVal = number;
}

void JsonSetDouble(JsonValue* value, double number) {
	value->type_ = JSON_VALUE_DOUBLE;
	value->_doubleVal = number;
}

void JsonSetString(JsonValue* value, const char* str, void* allocContext) {
	value->type_ = JSON_VALUE_STRING;
	value->_allocated = true;
	unsigned int len = (unsigned int)strlen(str);
	value->_strVal = (char*)JSON_ALLOC(allocContext, len + 1);
	memcpy(value->_strVal, str, len + 1);
}

void JsonSetCString(JsonValue* value, const char* str) {
	value->type_ = JSON_VALUE_STRING;
	value->_strVal = (char*)str;
	value->_allocated = false;
}

void JsonSetArray(JsonValue* value, void* allocContext) {
	value->type_ = JSON_VALUE_ARRAY;
	value->_allocated = false;
	value->_allocContext = allocContext;
}

void JsonSetObject(JsonValue* value, void* allocContext) {
	value->type_ = JSON_VALUE_OBJECT;
	value->_allocated = false;
	value->_allocContext = allocContext;
}

bool JsonIsType(const JsonValue* value, JsonValueType type) {
	return value->type_ == type;
}

bool JsonIsInvalid(const JsonValue* value) {
	return value->type_ == JSON_VALUE_INVALID;
}

bool JsonIsBool(const JsonValue* value) {
	return value->type_ == JSON_VALUE_BOOL;
}

bool JsonIsNull(const JsonValue* value) {
	return value->type_ == JSON_VALUE_NULL;
}

bool JsonIsInt(const JsonValue* value) {
	return value->type_ == JSON_VALUE_INT;
}

bool JsonIsDouble(const JsonValue* value) {
	return value->type_ == JSON_VALUE_DOUBLE;
}

bool JsonIsString(const JsonValue* value) {
	return value->type_ == JSON_VALUE_STRING;
}

bool JsonIsArray(const JsonValue* value) {
	return value->type_ == JSON_VALUE_ARRAY;
}

bool JsonIsObject(const JsonValue* value) {
	return value->type_ == JSON_VALUE_OBJECT;
}

void JsonArrayReserve(JsonValue* value, unsigned int capacity) {
	unsigned int currentCapacity = 0;
	if (value->_allocated) {
		currentCapacity = value->_arrayVal->capacity;
	}

	if (currentCapacity < capacity) {
		Json_ArrayMaybeGrow(value, capacity - currentCapacity);
	}
}

void JsonObjectRemove(JsonValue* object, JsonKeyValue* keyValue) {
	JsonDeinit(&keyValue->key);
	JsonDeinit(&keyValue->value);
	JsonKeyValue* last = JsonObjectBegin(object) + JsonObjectCount(object) - 1;
	*keyValue = *last;
	object->_arrayVal->count--;
}

JsonKeyValue* JsonObjectFind(JsonValue* object, const char* name) {
	JsonKeyValue* it = JsonObjectBegin(object);
	JsonKeyValue* end = it + JsonObjectCount(object);
	for (; it < end; it++) {
		if (strcmp(it->key._strVal, name) == 0)
			return it;
	}

	return nullptr;
}

const JsonKeyValue* JsonObjectCFind(const JsonValue* object, const char* name) {
	const JsonKeyValue* it = JsonObjectCBegin(object);
	const JsonKeyValue* end = it + JsonObjectCount(object);
	for (; it < end; it++) {
		if (strcmp(it->key._strVal, name) == 0)
			return it;
	}

	return nullptr;
}

void JsonArrayAdd(JsonValue* array, JsonValue* element) {
	Json_ArrayMaybeGrow(array, 1);
	Json_Array* arr = array->_arrayVal;
	arr->values[arr->count++] = *element;
	element->type_ = JSON_VALUE_INVALID;
}

void JsonObjectAdd(JsonValue* object, JsonValue* name, JsonValue* namesValue) {
	Json_ArrayMaybeGrow(object, 2);

	Json_Array* arr = object->_arrayVal;
	arr->values[arr->count++] = *name;
	arr->values[arr->count++] = *namesValue;

	name->type_ = JSON_VALUE_INVALID;
	namesValue->type_ = JSON_VALUE_INVALID;
}

unsigned int JsonArrayCount(const JsonValue* array) {
	return array->_allocated ? array->_arrayVal->count : 0;
}

unsigned int JsonObjectCount(const JsonValue* object) {
	return object->_allocated ? object->_arrayVal->count / 2 : 0;
}

static Json_Array* Json_MakeArray(unsigned int capacity, void* allocContext) {
	size_t size = capacity * sizeof(JsonValue) + offsetof(Json_Array, values);
	Json_Array* arr = (Json_Array*)JSON_ALLOC(allocContext, size);
	arr->allocContext = allocContext;
	arr->count = 0;
	arr->capacity = capacity;

	return arr;
}

static void Json_ArrayMaybeGrow(JsonValue* array, unsigned int grow) {
	if (!array->_allocated) {
		array->_arrayVal = Json_MakeArray(grow < 32 ? 32 : grow, array->_allocContext);
		array->_allocated = true;
	}
	else {
		Json_Array* arr = array->_arrayVal;
		unsigned int capacity = arr->count + grow;
		if (capacity >= arr->capacity) {
			Json_Array* oldArr = arr;

			arr = Json_MakeArray(grow < oldArr->capacity ? 2 * oldArr->capacity : capacity, oldArr->allocContext);
			memcpy(arr->values, oldArr->values, oldArr->count * sizeof(JsonValue));
			arr->count = oldArr->count;
			JSON_FREE(arr->allocContext, oldArr);
			array->_arrayVal = arr;
		}
	}
}

static void Json_LogError(Json_ParseContext* context, const char* fmt, ...) {
#ifdef JSON_ERROR_MESSAGE
	if (!context->error[0]) {
		va_list args;
		va_start(args, fmt);
		vsnprintf(context->error, sizeof(Json_ParseContext::error), fmt, args);
		va_end(args);
	}
#endif
}

static const char* Json_TokenName(Json_TokenType type) {
	switch (type) {
	case JSON_TOKEN_NONE: return "none";
	case JSON_TOKEN_EOS: return "end of stream";
	case JSON_TOKEN_COLON: return ":";
	case JSON_TOKEN_COMMA: return ",";
	case JSON_TOKEN_LEFT_BRACE: return "{";
	case JSON_TOKEN_RIGHT_BRACE: return "}";
	case JSON_TOKEN_LEFT_BRACKET: return "[";
	case JSON_TOKEN_RIGHT_BRACKET: return "]";
	case JSON_TOKEN_INT: return "integer";
	case JSON_TOKEN_DOUBLE: return "double";
	case JSON_TOKEN_STR: return "string";
	case JSON_TOKEN_TRUE: return "true";
	case JSON_TOKEN_FALSE: return "false";
	case JSON_TOKEN_NULL: return "null";
	default:
		break;
	}
	return nullptr;
}


static char* Json_LexerParseHexaNumber(Json_ParseContext* context) {
	char* stream = context->stream;
	char* start = stream;

	++stream; // skip zero
	++stream; // skip x

	if (!isxdigit(*stream)) {
		Json_LogError(context, "JSON: Hex digit expected after 0x\n");
		return stream;
	}
	++stream;

	while (isxdigit(*stream))
		++stream;

	long long int i = strtoll(start, nullptr, 16);
	if (i == LLONG_MAX) {
		Json_LogError(context, "JSON: Integer value too large\n");
	}
	else {
		context->token.intVal = i;
		context->token.type = JSON_TOKEN_INT;
	}

	return stream;
}

static char* Json_LexerParseNumber(Json_ParseContext* context) {
	char* stream = context->stream;
	char* start = stream;

	++stream;
	while (isdigit(*stream))
		++stream;

	bool isDouble = false;
	if (*stream == '.') {
		++stream;
		isDouble = true;

		if (!isdigit(*stream)) {
			Json_LogError(context, "JSON: Digit expected dot\n");
			return stream;
		}
		++stream;
	}

	if (*stream == 'e' || *stream == 'E') {
		++stream;
		isDouble = true;

		if (*stream == '+' || *stream == '-') {
			++stream;
		}

		if (!isdigit(*stream)) {
			Json_LogError(context, "JSON: Digit expected after e or sign \n");
			return stream;
		}
		++stream;
	}

	if (isDouble) {
		while (isdigit(*stream))
			++stream;

		double d = strtod(start, nullptr);
		if (d == HUGE_VAL) {
			Json_LogError(context, "JSON: Floating point value too large\n");
		}
		else {
			context->token.doubleVal = d;
			context->token.type = JSON_TOKEN_DOUBLE;
		}
	}
	else {
		long long int i = strtoll(start, nullptr, 10);
		if (i == LLONG_MAX) { // this kills the last value (if we dont check errno), can be done better
			Json_LogError(context, "JSON: Integer value too large\n");
		}
		else {
			context->token.intVal = i;
			context->token.type = JSON_TOKEN_INT;
		}
	}

	return stream;
}

static void Json_LexerParseString(Json_ParseContext* context) {
	char* stream = context->stream;
	const char* start = stream;
	++stream;
	for (;;) {
		if (*stream == '\0') {
			Json_LogError(context, "JSON: While reading string literal, found EoF\n");
			break;
		}
		if (*stream == '"') {
			context->token.strVal = start + 1;
			context->token.type = JSON_TOKEN_STR;
			++stream;
			break;
		}
		++stream;
	}

	context->stream = stream;
}

static void Json_LexerNextToken(Json_ParseContext* context) {
	char* stream = context->stream;

	bool whitespace = true;
	while (whitespace) {
		switch (*stream) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
		case '\b':
		case '\a':
		case '\v':
			stream++;
			break;
		default:
			whitespace = false;
			break;
		}
	}

	context->stream = stream;
	context->token.type = JSON_TOKEN_NONE;
	context->token.valBegin = stream;

	switch (*stream) {
	case 't':
	{
		++stream;
		bool b = (*stream != '\0') && (*stream++ == 'r') &&
			(*stream != '\0') && (*stream++ == 'u') &&
			(*stream != '\0') && (*stream++ == 'e');

		if (b) {
			context->token.type = JSON_TOKEN_TRUE;
		}
		else {
			Json_LogError(context, "JSON: Error while parsing keyword true\n");
		}
		break;
	}
	case 'f':
	{
		++stream;
		bool b = (*stream != '\0') && (*stream++ == 'a') &&
			(*stream != '\0') && (*stream++ == 'l') &&
			(*stream != '\0') && (*stream++ == 's') &&
			(*stream != '\0') && (*stream++ == 'e');

		if (b) {
			context->token.type = JSON_TOKEN_FALSE;
		}
		else {
			Json_LogError(context, "JSON: Error while parsing keyword false\n");
		}
		break;
	}
	case 'n':
	{
		++stream;
		bool b = (*stream != '\0') && (*stream++ == 'u') &&
			(*stream != '\0') && (*stream++ == 'l') &&
			(*stream != '\0') && (*stream++ == 'l');

		if (b) {
			context->token.type = JSON_TOKEN_NULL;
		}
		else {
			Json_LogError(context, "JSON: Error while parsing keyword null\n");
		}
		break;
	}
	case '-':
		++stream;
		if (!isdigit(*stream)) {
			Json_LogError(context, "JSON: Digit expected after -\n");
		}
		else {
			stream = Json_LexerParseNumber(context);
		}
		break;
	case '0':
		++stream;
		if (*stream == 'x') {
			stream = Json_LexerParseHexaNumber(context);
		}
		else {
			stream = Json_LexerParseNumber(context);
		}
		break;
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		stream = Json_LexerParseNumber(context);
		break;
	case '"':
		Json_LexerParseString(context);
		stream = context->stream;
		break;
	case '[':
		++stream;
		context->token.type = JSON_TOKEN_LEFT_BRACKET;
		break;
	case ']':
		++stream;
		context->token.type = JSON_TOKEN_RIGHT_BRACKET;
		break;
	case '{':
		++stream;
		context->token.type = JSON_TOKEN_LEFT_BRACE;
		break;
	case '}':
		++stream;
		context->token.type = JSON_TOKEN_RIGHT_BRACE;
		break;
	case ',':
		++stream;
		context->token.type = JSON_TOKEN_COMMA;
		break;
	case ':':
		++stream;
		context->token.type = JSON_TOKEN_COLON;
		break;
	case '\0':
		context->token.type = JSON_TOKEN_EOS;
		break;
	default:
		context->token.type = JSON_TOKEN_NONE;
		Json_LogError(context, "Invalid character %c\n", *stream);
		++stream;
		break;
	}

	context->token.valEnd = stream;
	context->stream = stream;
}

static void Json_LexerInit(Json_ParseContext* context, void* allocContext, char* stream) {
	context->allocContext = allocContext;
	context->token.type = JSON_TOKEN_NONE;
	context->stream = stream;
	context->error[0] = '\0';
	Json_LexerNextToken(context);
}

static bool Json_LexerIsToken(Json_ParseContext* context, Json_TokenType type) {
	return context->token.type == type;
}

static bool Json_LexerMatchToken(Json_ParseContext* context, Json_TokenType type) {
	if (Json_LexerIsToken(context, type)) {
		Json_LexerNextToken(context);
		return true;
	}

	return false;
}

static bool Json_LexerExpectToken(Json_ParseContext* context, Json_TokenType type) {
	if (Json_LexerIsToken(context, type)) {
		Json_LexerNextToken(context);
		return true;
	}

	Json_LogError(context, "JSON: Expected token %s, got %s\n", Json_TokenName(type), Json_TokenName(context->token.type));
	return false;
}

static bool Json_ExpectAnyValue(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_MatchValue(context, outValue))
		return true;

	Json_LogError(context, "JSON: Expected json value, got %s\n", Json_TokenName(context->token.type));
	return false;
}

static bool Json_ExpectString(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_MatchString(context, outValue))
		return true;

	Json_LogError(context, "JSON: Expected string, got %s\n", Json_TokenName(context->token.type));
	return false;
}

static bool Json_MatchString(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_LexerIsToken(context, JSON_TOKEN_STR)) {
#ifdef JSON_INPUT_STRING_IS_STORAGE 
		*(context->token.valEnd - 1) = '\0';
		outValue->_strVal = context->token.valBegin + 1;
		outValue->_allocated = false;
#else
		if (context->token.valEnd - context->token.valBegin == 2) {
			outValue->_strVal = nullptr;
			outValue->_allocated = false;
		}
		else {
			long long len = context->token.valEnd - context->token.valBegin - 2;
			char* s = (char*)JSON_ALLOC(context->allocContext, len + 1);
			s[len] = 0;
			outValue->_strVal = s;
			memcpy(s, context->token.valBegin + 1, len);
			outValue->_allocated = true;
		}
#endif

		outValue->type_ = JSON_VALUE_STRING;
		Json_LexerNextToken(context);
		return true;
	}

	return false;
}

static bool Json_MatchObject(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_LexerMatchToken(context, JSON_TOKEN_LEFT_BRACE)) {
		JsonSetObject(outValue, context->allocContext);
		JsonValue k, v;

		if (Json_MatchString(context, &k)) {
			if (!Json_LexerExpectToken(context, JSON_TOKEN_COLON) || !Json_ExpectAnyValue(context, &v))
				return false;

			JsonObjectAdd(outValue, &k, &v);

			while (Json_LexerMatchToken(context, JSON_TOKEN_COMMA)) {
				if (!Json_ExpectString(context, &k) || !Json_LexerExpectToken(context, JSON_TOKEN_COLON) || !Json_ExpectAnyValue(context, &v))
					return false;

				JsonObjectAdd(outValue, &k, &v);
			}
		}
		return Json_LexerExpectToken(context, JSON_TOKEN_RIGHT_BRACE);
	}

	return false;
}

static bool Json_MatchArray(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_LexerMatchToken(context, JSON_TOKEN_LEFT_BRACKET)) {
		JsonSetArray(outValue, context->allocContext);

		JsonValue v;
		if (Json_MatchValue(context, &v)) {
			JsonArrayAdd(outValue, &v);

			while (Json_LexerMatchToken(context, JSON_TOKEN_COMMA)) {
				if (!Json_ExpectAnyValue(context, &v))
					return false;

				JsonArrayAdd(outValue, &v);
			}
		}

		return Json_LexerExpectToken(context, JSON_TOKEN_RIGHT_BRACKET);
	}

	return false;
}

inline bool Json_MatchValue(Json_ParseContext* context, JsonValue* outValue) {
	if (Json_LexerIsToken(context, JSON_TOKEN_TRUE)) {
		JsonSetBool(outValue, true);
		Json_LexerNextToken(context);
		return true;
	}
	else if (Json_LexerIsToken(context, JSON_TOKEN_FALSE)) {
		JsonSetBool(outValue, false);
		Json_LexerNextToken(context);
		return true;
	}
	else if (Json_LexerIsToken(context, JSON_TOKEN_INT)) {
		JsonSetInt(outValue, context->token.intVal);
		Json_LexerNextToken(context);
		return true;
	}
	else if (Json_LexerIsToken(context, JSON_TOKEN_DOUBLE)) {
		JsonSetDouble(outValue, context->token.doubleVal);
		Json_LexerNextToken(context);
		return true;
	}
	else if (Json_MatchString(context, outValue)) {
		return true;
	}
	else if (Json_MatchArray(context, outValue)) {
		return true;
	}
	else if (Json_MatchObject(context, outValue)) {
		return true;
	}

	return false;
}


static void Json_PrintFormat(JsonPrintContext* context, const char* format, ...) {
	va_list args;
	va_start(args, format);

	unsigned int capacity = context->bufferCapacity - context->bufferSize;
	unsigned int needed = vsnprintf(context->buffer + context->bufferSize, capacity, format, args);

	if (needed >= 0) {
		if (needed >= capacity) {
			unsigned int newCapacity = 2 * (needed + context->bufferCapacity) + 256;
			char* newBuffer = (char*)JSON_ALLOC(context->allocContext, newCapacity);
			if (context->buffer) {
				memcpy(newBuffer, context->buffer, context->bufferSize);
				JSON_FREE(context->allocContext, context->buffer);
			}
			context->bufferCapacity = newCapacity;
			context->buffer = newBuffer;

			needed = vsnprintf(context->buffer + context->bufferSize, context->bufferCapacity - context->bufferSize, format, args);
		}

		context->bufferSize += needed;
	}
	va_end(args);
}

static void Json_ValuePrintPretty(JsonPrintContext* context, const JsonValue* value, unsigned int indentAll, unsigned int indentCur) {
	switch (value->type_) {
	case JSON_VALUE_INVALID: break;
	case JSON_VALUE_ARRAY:
	{
		const JsonValue* it = JsonArrayCBegin(value);
		const JsonValue* end = it + JsonArrayCount(value);
		if (end - it > 0) {
			Json_PrintFormat(context, "%*s[\n", indentCur, "");
			indentAll += 2;
			while (it < end - 1) {
				Json_ValuePrintPretty(context, it++, indentAll, indentAll);
				Json_PrintFormat(context, ",\n");
			}
			Json_ValuePrintPretty(context, it, indentAll, indentAll);
			Json_PrintFormat(context, "\n%*s]", indentAll - 2, "");
		}
		else {
			Json_PrintFormat(context, "%*s[]", indentCur, "");
		}

		break;
	}
	case JSON_VALUE_OBJECT:
	{
		const JsonKeyValue* it = JsonObjectCBegin(value);
		const JsonKeyValue* end = it + JsonObjectCount(value);
		if (end - it > 0) {
			Json_PrintFormat(context, "%*s{\n", indentCur, "");
			indentAll += 2;

			while (it < end - 1) {
				Json_PrintFormat(context, "%*s\"%s\" : ", indentAll, "", it->key._strVal);
				Json_ValuePrintPretty(context, &it->value, indentAll, 0);
				Json_PrintFormat(context, ",\n");
				++it;
			}
			Json_PrintFormat(context, "%*s\"%s\" : ", indentAll, "", it->key._strVal);
			Json_ValuePrintPretty(context, &it->value, indentAll, 0);
			Json_PrintFormat(context, "\n%*s}", indentAll - 2, "");
		}
		else {
			Json_PrintFormat(context, "%*s{}", indentCur, "");
		}
		break;
	}
	case JSON_VALUE_BOOL: Json_PrintFormat(context, value->_boolVal ? "%*strue" : "%*false", indentCur, ""); break;
	case JSON_VALUE_NULL: Json_PrintFormat(context, "%*snull", indentCur, ""); break;
	case JSON_VALUE_INT: Json_PrintFormat(context, "%*s%lld", indentCur, "", value->_intVal); break;
	case JSON_VALUE_DOUBLE: Json_PrintFormat(context, "%*s%f", indentCur, "", value->_doubleVal); break;
	case JSON_VALUE_STRING: Json_PrintFormat(context, "%*s\"%s\"", indentCur, "", value->_strVal ? value->_strVal : ""); break;
	default: break;
	}
}

static void Json_ValuePrint(JsonPrintContext* context, const JsonValue* value) {
	switch (value->type_) {
	case JSON_VALUE_INVALID: break;
	case JSON_VALUE_ARRAY:
	{
		const JsonValue* it = JsonArrayCBegin(value);
		const JsonValue* end = it + JsonArrayCount(value);
		if (end - it > 0) {
			Json_PrintFormat(context, "[");
			while (it < end - 1) {
				Json_ValuePrint(context, it++);
				Json_PrintFormat(context, ",");
			}
			Json_ValuePrint(context, it);
			Json_PrintFormat(context, "]");
		}
		else {
			Json_PrintFormat(context, "[]");
		}
		break;
	}
	case JSON_VALUE_OBJECT:
	{
		const JsonKeyValue* it = JsonObjectCBegin(value);
		const JsonKeyValue* end = it + JsonObjectCount(value);
		if (end - it > 0) {
			Json_PrintFormat(context, "{");

			while (it < end - 1) {
				Json_PrintFormat(context, "\"%s\":", it->key._strVal);
				Json_ValuePrint(context, &it->value);
				Json_PrintFormat(context, ",");
				++it;
			}
			Json_PrintFormat(context, "\"%s\":", it->key._strVal);
			Json_ValuePrint(context, &it->value);
			Json_PrintFormat(context, "}");
		}
		else {
			Json_PrintFormat(context, "{}");
		}
		break;
	}
	case JSON_VALUE_BOOL: Json_PrintFormat(context, value->_boolVal ? "true" : "false"); break;
	case JSON_VALUE_NULL: Json_PrintFormat(context, "null"); break;
	case JSON_VALUE_INT: Json_PrintFormat(context, "%lld", value->_intVal); break;
	case JSON_VALUE_DOUBLE: Json_PrintFormat(context, "%f", value->_doubleVal); break;
	case JSON_VALUE_STRING: Json_PrintFormat(context, "\"%s\"", value->_strVal ? value->_strVal : ""); break;
	default: break;
	}
}


#endif // JSON_IMPLEMENTATION

#ifdef JSON_ALLOC_DEFAULT_DEF
#undef JSON_ALLOC
#undef JSON_FREE
#undef JSON_ALLOC_DEFAULT_DEF
#endif

