#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "cJSON/cJSON.h"

#define MAX_WIFI_SSID_LEN 32
#define MAX_WIFI_PWD_LEN 63


#define WIFI_SSID "wifi_ssid"
#define WIFI_PWD "wifi_pwd"
#define LWM2M_PORT "lwm2m_port"
#define LWM2M_SECURE "lwm2m_secure"
#define SLEEP_INTERVAL "sleep_interval"

typedef struct {
    char wifi_ssid[MAX_WIFI_SSID_LEN];
    int lwm2m_port;
    char wifi_pwd[MAX_WIFI_PWD_LEN];
    bool lwm2m_secure;
    int sleep_interval;
} device_config_t;

device_config_t _device_config = {
    .wifi_ssid = "default",
    .lwm2m_port = 1234,
    .wifi_pwd = "adminadmin",
    .lwm2m_secure = false,
    .sleep_interval = 40
};

device_config_t *g_device_config = &_device_config;

enum type_t { INT, STRING, BOOL };

struct field_desc_t;
typedef int (*config_handler_t)(cJSON *object, struct field_desc_t descriptor);
typedef struct field_desc_t{
    char *name;
    enum type_t type;
    size_t offset;
	int max;
	int min;
	config_handler_t write_callback;
	config_handler_t read_callback;
} field_descriptor_t;


int write_int_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

	if ((int) (object->valuedouble) >= descriptor.min && (int) (object->valuedouble) <= descriptor.max ) {
		*((int *)field_address) = (int) object->valuedouble;
		return 0;
	}
	else {
		fprintf(stderr, "Provided %s not in %d-%d range!\n", descriptor.name, descriptor.min, descriptor.max);
		return  -1;
	}
}

int write_string_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

	if (object->type == cJSON_String && strlen(object->valuestring) < descriptor.max) {
		strncpy(field_address, object->valuestring, descriptor.max);
		return 0;
	}
	else {
		fprintf(stderr, "Provided %s is too long!\n", descriptor.name);
		return  -1;
	}
}

int write_bool_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

	if (object->type != cJSON_True && object->type != cJSON_False) {
		fprintf(stderr, "Provided %s has incorrect type!\n", descriptor.name);
		return  -1;
	}
	else {
		*((bool *)field_address) = object->type == cJSON_True;
		return 0;
	}
}

int read_string_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

    cJSON_AddStringToObject(object, descriptor.name, (char*) field_address);

    return 0;
}

int read_int_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

    cJSON_AddNumberToObject(object, descriptor.name,  *((int*)field_address));

    return 0;
}

int read_bool_callback(cJSON *object, struct field_desc_t descriptor)
{
	void *field_address = (char *) g_device_config + descriptor.offset;

    *((bool *)field_address) ? cJSON_AddTrueToObject(object, descriptor.name) :
        cJSON_AddFalseToObject(object, descriptor.name);

    return 0;
}



field_descriptor_t field_descriptor[]  = {
    {
	.name = WIFI_SSID,
	.type = STRING,
	.offset = offsetof(device_config_t, wifi_ssid),
	.max = MAX_WIFI_SSID_LEN,
	.min = 0,
	.write_callback = write_string_callback,
    .read_callback = read_string_callback
    },
    {
	.name = LWM2M_PORT,
	.type = INT,
	.offset = offsetof(device_config_t, lwm2m_port),
	.max = 65535,
	.min = 0,
	.write_callback = write_int_callback,
    .read_callback = read_int_callback
    },
    {
	.name = WIFI_PWD,
	.type = STRING,
	.offset = offsetof(device_config_t, wifi_pwd),
	.max = MAX_WIFI_PWD_LEN,
	.min = 0,
	.write_callback = write_string_callback,
    .read_callback = read_string_callback
    },
    {
	.name = LWM2M_SECURE,
	.type = BOOL,
	.offset = offsetof(device_config_t, lwm2m_secure),
	.max = 1,
	.min = 0,
	.write_callback = write_bool_callback,
    .read_callback = read_bool_callback
    },
    {
	.name = SLEEP_INTERVAL,
	.type = INT,
	.offset = offsetof(device_config_t, sleep_interval),
	.max = INT_MAX,
	.min = 0,
	.write_callback = write_int_callback,
    .read_callback = read_int_callback
    },
    {0}
};

const char *json_str = "{ \"wifi_ssid\":\"qwertyuiop\", \"wifi_pwd\":\"psk\", "
		       "\"lwm2m_secure\":true, \"sleep_interval\":1488, "
		       "\"lwm2m_port\":4321 }";


void print_config(void)
{
    printf("%s: %s\n", WIFI_SSID, g_device_config->wifi_ssid);
    printf("%s: %s\n", WIFI_PWD, g_device_config->wifi_pwd);
    printf("%s: %d\n", LWM2M_PORT, g_device_config->lwm2m_port);
    printf("%s: %s\n", LWM2M_SECURE, g_device_config->lwm2m_secure ? "true" : "false");
    printf("%s: %d\n", SLEEP_INTERVAL, g_device_config->sleep_interval);
}

int json_to_config(const char *json_str)
{
	cJSON *root;
    cJSON *subitem;

    root = cJSON_Parse(json_str);
    if (!root || (root->type != cJSON_Object)) {
		exit(-1);
    }

    subitem = root->child;

    while (subitem) {

		for (int i = 0; field_descriptor[i].name != 0; i++) {

			field_descriptor_t descriptor = field_descriptor[i];
			if (strncmp(subitem->string, descriptor.name,strlen(subitem->string)) == 0) {
				descriptor.write_callback(subitem, descriptor);
			}
		}
		subitem = subitem->next;
    }

	cJSON_Delete(root);

    return 0;
}

int config_to_json()
{
    cJSON *root;

    root = cJSON_CreateObject();

    for (int i = 0; field_descriptor[i].name != 0; i++) {
        field_descriptor_t descriptor = field_descriptor[i];
        descriptor.read_callback(root, descriptor);
    }

    return 0;
}


int main(void)
{

}
