#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cJSON/cJSON.h"

#define SSID_LEN 100
#define PSK_LEN 100

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


char* json = "{ \"wifi_ssid\":\"123ssid\", \"wifi_psk\":\"psk\", \"lwm2m_secure\":true}";
device_config_t _device_config = {
	.wifi_ssid = "default_ssid",
	.lwm2m_port = 1234,
	.wifi_psk = "adminadmin",
	.lwm2m_secure = true,
	.sleep_interval = 40
};
device_config_t *g_device_config = &_device_config;

int main(void)
{
	/* for (int i = 0; field_descriptor[i].name != NULL ; ++i) { */

	/* 	printf("Name: %s\n", field_descriptor[i].name); */

	/* 	switch (field_descriptor[i].type) { */
	/* 	case INT: { */
	/* 		printf("Type: int\n"); */
	/* 		break; */
	/* 	} */
	/* 	case STRING: { */
	/* 		printf("Type: char*\n"); */
	/* 		break; */
	/* 	} */
	/* 	case BOOL: { */
	/* 		printf("Type: bool\n"); */
	/* 		break; */
	/* 	} */
	/* 	} */

	/* 	printf("Offset: %zd\n", field_descriptor[i].offset); */
	/* 	printf("\n"); */

	/* } */

	cJSON *object = cJSON_Parse(json);

	printf("old ssid %s\n", g_device_config->wifi_ssid);

	cJSON *ssid = cJSON_GetObjectItem(object, field_descriptor[0].name);
	if (cJSON_IsString(ssid)) {
		strncpy(g_device_config + field_descriptor[0].offset, "kek", sizeof(WIFI_SSID) - 1);
	}

	printf("new ssid %s\n", g_device_config->wifi_ssid);

}
