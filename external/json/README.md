# JSON

Single header ASCI encoding json parser for c++

## Usage
	#define JSON_IMPLEMENTATION 
	//before json.h include in one .cpp file

### Options
	//Create error message if parsing failed
	#define JSON_ERROR_MESSAGE

	//Destructive parsing for strings, parsed string points to modified input buffer "str" -> "str\0 ...
	#define JSON_INPUT_STRING_IS_STORAGE

	//Custom allocator
	#define JSON_ALLOC(context, size) <alloc function call> // malloc(size) by default
	#define JSON_FREE(context, data) <free function call>   // free(data) by default

## Examples
	
```
// Setting
JsonValue value;
// Third parameter is context to send to custom allocator (used in string, array, object)
JsonSetString(&value, "abc", nullptr); 
JsonDeinit(&value);
```

```
// Memory leak
JsonValue value; // Uninitialized value
JsonSetString(&value, "test", nullptr);
// !Memory leak, Set function dont deinitialize, value might be uninitialized
JsonSetString(&value, "abc", nullptr); 
JsonDeinit(&value);
```

```
// UB
JsonValue value; // Uninitialized value
JsonDeinit(&value); // UB cant do proper check to deallocate on unitialized value

// JsonValue value; 
// JsonSetInvalid(&value) // If the value might or might not be set
```

```
// Array / Obj usage
JsonValue arr;
JsonSetArray(&arr, nullptr);

JsonValue value;
JonSetString(&value, "Test", nullptr);
JsonArrayAdd(&arr, &value); // Ownership is moved

JsonSetDouble(&value, 18.0); // Therefore value can be reused
JsonArrayAdd(&arr, &value);

JsonDeinit(&arr); // array will deallocate its members

// Object usage is almost the same just you have to specify name and value
// JsonObjectAdd( array, name, value); // name must be string
```

```
// Parsing
char input[] = "{\"key\" : 4}";

JsonValue value;
if (JsonParse(input, nullptr, &value)) {
	if (JsonIsObject(&value)) {
		// ... 
	}
	JsonDeinit(&value);

	// Alternatevely JsonIsType(&value, JSON_VALUE_OBJECT);
	// or value->type_ == JSON_VALUE_OBJECT
}

// Parsing returns string type with nullptr 
//   if the string was empty and JSON_INPUT_STRING_IS_STORAGE is not used
// if (JsonIsType(&value, JSON_VALUE_STRING)) { 
//   if (JsonGetString(&value)) { ... }
// }
```

``` 
// Printing
JsonValue value;
JsonSetString(&value, "abc", nullptr);
JsonDeinit(&value);

JsonPrintContext context = JsonPrintContextInit(nullptr);
/// returned buffer is owned by printcontext
char* buffer = JsonValuePrint(&context, &value, true, nullptr);
// ... use buffer

// if buffer is not needed anymore, printcontext can be reused
buffer = JsonValuePrint(&context, &value, true, nullptr); 

JsonPrintContextDeinit(&context);
```
