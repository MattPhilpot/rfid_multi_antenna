#include <string.h>
#include <jansson.h>
#include "tagstruct.h"
#include <confuse.h>
#include "memory_helper/memory_helper_macros.h"

///--------------- Example Code -------------------------------------

/* forward refs */
void print_json(json_t *root);
void print_json_aux(json_t *element, int indent);
void print_json_indent(int indent);
const char *json_plural(int count);
void print_json_object(json_t *element, int indent);
void print_json_array(json_t *element, int indent);
void print_json_string(json_t *element, int indent);
void print_json_integer(json_t *element, int indent);
void print_json_real(json_t *element, int indent);
void print_json_true(json_t *element, int indent);
void print_json_false(json_t *element, int indent);
void print_json_null(json_t *element, int indent);

void print_json(json_t *root) {
    print_json_aux(root, 0);
}

void print_json_aux(json_t *element, int indent) {
    switch (json_typeof(element)) {
    case JSON_OBJECT:
        print_json_object(element, indent);
        break;
    case JSON_ARRAY:
        print_json_array(element, indent);
        break;
    case JSON_STRING:
        print_json_string(element, indent);
        break;
    case JSON_INTEGER:
        print_json_integer(element, indent);
        break;
    case JSON_REAL:
        print_json_real(element, indent);
        break;
    case JSON_TRUE:
        print_json_true(element, indent);
        break;
    case JSON_FALSE:
        print_json_false(element, indent);
        break;
    case JSON_NULL:
        print_json_null(element, indent);
        break;
    default:
        controller_log("unrecognized JSON type %d\n", json_typeof(element));
    }
}

void print_json_indent(int indent) {
    int i;
    for (i = 0; i < indent; i++) { putchar(' '); }
}

const char *json_plural(int count) {
    return count == 1 ? "" : "s";
}

void print_json_object(json_t *element, int indent) {
    size_t size;
    const char *key;
    json_t *value;

    print_json_indent(indent);
    size = json_object_size(element);

    controller_log("JSON Object of %ld pair%s:\n", size, json_plural(size));
    json_object_foreach(element, key, value) {
        bool print = false;
        switch (json_typeof(value))
        {
        case JSON_OBJECT:
        case JSON_ARRAY:
            break;
        case JSON_STRING:
            print = setConfigValue(CFGT_STR, key, json_string_value(value));
            break;
        case JSON_INTEGER:
            print = setConfigValue(CFGT_INT, key, json_integer_value(value));
            break;
        case JSON_REAL:
            print = setConfigValue(CFGT_FLOAT, key, json_real_value(value));
            break;
        case JSON_TRUE:
            print = setConfigValue(CFGT_BOOL, key, true);
            break;
        case JSON_FALSE:
            print = setConfigValue(CFGT_BOOL, key, false);
            break;
        case JSON_NULL:
            print = setConfigValue(CFGT_INT, key, 0);
            break;
        }

        if (print)
        {
            print_json_indent(indent + 2);
            controller_log("JSON Key: \"%s\"\n", key);
            print_json_aux(value, indent + 2);
        }
    }

}

void print_json_array(json_t *element, int indent) {
    size_t i;
    size_t size = json_array_size(element);
    print_json_indent(indent);

    controller_log("JSON Array of %ld element%s:\n", size, json_plural(size));
    for (i = 0; i < size; i++) {
        print_json_aux(json_array_get(element, i), indent + 2);
    }
}

void print_json_string(json_t *element, int indent) {
    print_json_indent(indent);
    controller_log("JSON String: \"%s\"\n", json_string_value(element));
}

void print_json_integer(json_t *element, int indent) {
    print_json_indent(indent);
    controller_log("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}

void print_json_real(json_t *element, int indent) {
    print_json_indent(indent);
    controller_log("JSON Real: %f\n", json_real_value(element));
}

void print_json_true(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    controller_log("JSON True\n");
}

void print_json_false(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    controller_log("JSON False\n");
}

void print_json_null(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    controller_log("JSON Null\n");
}

///------------------------------------------------------------------

bool parseJsonResult(char* result)
{
    json_t* root;
    json_error_t error;
    root = json_loads(result, JSON_DISABLE_EOF_CHECK | JSON_DECODE_ANY, &error);

    if (!root)
    {
        controller_log("result is null: %s\n", error.text);
        return false;
    }

    print_json(root);
    json_decref(root);
    //const char* key;
    //json_t* value;

    SAFE_FREE(result);
    return true; //fixme
}

char* getHeartBeatJson(char* deviceIdentifier, char* time, double latitude, double longitude, short mode)
{
    json_t *json;
    char *result;

    json = json_object();

    json_object_set_new(json, "deviceIdentifier", json_string(deviceIdentifier));
    json_object_set_new(json, "deviceStatus", json_integer(mode));

    if (latitude)
        json_object_set_new(json, "latitude", json_real(latitude));

    if (longitude)
        json_object_set_new(json, "longitude", json_real(longitude));

    result = json_dumps(json, 0);

    json_decref(json);

    return result;
}

char* assignTagsToLocationsJson(char *deviceIdentifier, double latitude, double longitude, char *scanTime, TagHistory *tags) {
    json_t *json;
    char *result;

    json = json_object();

    json_object_set_new(json, "deviceIdentifier", json_string(deviceIdentifier));

/*
    if (latitude != 0 && longitude != 0)
    {
        json_object_set_new(json, "latitude", json_real(latitude));
        json_object_set_new(json, "longitude", json_real(longitude));
    }
    */

/*
    if (scanTime != NULL)
        json_object_set_new(json, "scanTime", json_string(scanTime));
        */

    json_object_set_new(json, "messagePart", json_string("end"));

    json_t *jtags = json_array();

    json_object_set_new(json, "tags", jtags);

    size_t i;
    for (i = 0; i < tags->itemsInList; ++i)
    {
        json_t *jtag = json_object();
        json_object_set_new(jtag, "tag", json_string(tags->tagList[i]->epc));
        json_object_set_new(jtag, "latitude", json_real(tags->latitude));
        json_object_set_new(jtag, "longitude", json_real(tags->longitude));

        //char *time;
        //getFromTimeT(&time, tags[i].lastScanDateTime);

        //json_object_set_new(jtag, "scanTime", json_string(time));
        json_object_set_new(jtag, "scanTime", json_string(scanTime));
        //free(time);
        json_array_append_new(jtags, jtag);
    }

    result = json_dumps(json, 0);

    json_decref(jtags);
    json_decref(json);

    return result;
}
