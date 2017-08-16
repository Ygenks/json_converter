#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cJSON/cJSON.h"

#define SSID_LEN 100
#define PSK_LEN 16

#define WIFI_SSID "wifi_ssid"
#define WIFI_PSK "wifi_psk"
#define LWM2M_PORT "lwm2m_port"
#define LWM2M_SECURE "lwm2m_secure"
#define SLEEP_INTERVAL "sleep_interval"

typedef struct {
    char wifi_ssid[SSID_LEN];
    int lwm2m_port;
    char wifi_psk[PSK_LEN];
    bool lwm2m_secure;
    int sleep_interval;
} device_config_t;

device_config_t _device_config = {
    .wifi_ssid = "default",
    .lwm2m_port = 1234,
    .wifi_psk = "adminadmin",
    .lwm2m_secure = false,
    .sleep_interval = 40
};

device_config_t *g_device_config = &_device_config;

enum type_t { INT, STRING, BOOL };
typedef struct {
    char *name;
    enum type_t type;
    size_t offset;
} field_descriptor_t;

field_descriptor_t field_descriptor[]  = {
    {
	.name = WIFI_SSID,
	.type = STRING,
	.offset = offsetof(device_config_t, wifi_ssid)
    },
    {
	.name = LWM2M_PORT,
	.type = INT,
	.offset = offsetof(device_config_t, lwm2m_port)
    },
    {
	.name = WIFI_PSK,
	.type = STRING,
	.offset = offsetof(device_config_t, wifi_psk)
    },
    {
	.name = LWM2M_SECURE,
	.type = BOOL,
	.offset = offsetof(device_config_t, lwm2m_secure)
    },
    {
	.name = SLEEP_INTERVAL,

	.type = INT,
	.offset = offsetof(device_config_t, sleep_interval)
    },
    {0}
};


char* json_str = "{ \"wifi_ssid\":\"kekus\", \"wifi_psk\":\"psk\", \"lwm2m_secure\":true, \"sleep_interval\":1488}";

void print_config(void)
{
    printf("%s: %s\n", WIFI_SSID, g_device_config->wifi_ssid);
    printf("%s: %s\n", WIFI_PSK, g_device_config->wifi_psk);
    printf("%s: %d\n", LWM2M_PORT, g_device_config->lwm2m_port);
    printf("%s: %s\n", LWM2M_SECURE, g_device_config->lwm2m_secure ? "true" : "false");
    printf("%s: %d\n", SLEEP_INTERVAL, g_device_config->sleep_interval);
}

int main(void)
{

	print_config();

    cJSON *root;
    cJSON *subitem;

    root = cJSON_Parse(json_str);
    if (!root || (root->type != cJSON_Object)) {
	exit(-1);
    }

    subitem = root->child;

    int i = 0;

    while (subitem) {

	for (int i = 0; field_descriptor[i].name != 0; i++) {

	    field_descriptor_t descriptor = field_descriptor[i];

	    if (strncmp(subitem->string, descriptor.name,strlen(subitem->string)) == 0) {

		void *field_address = (void *) g_device_config + descriptor.offset;

		switch (descriptor.type) {

		case INT: {
		    *((int *)field_address) = (int) subitem->valuedouble;
		    break;
		}
		case STRING: {
		    strncpy(field_address, subitem->valuestring, sizeof(field_address) - 1);
		    break;
		}
		case BOOL: {
		    *((bool *)field_address) = subitem->type == cJSON_True;
		    break;
		}

		}
	    }
	}
	subitem = subitem->next;
	i++;
    }

    print_config();
}
