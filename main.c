#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "cJSON/cJSON.h"

#define MAX_DOMAIN_NAME_LEN      64
#define MAX_WIFI_SSID_LEN        32
#define MAX_WIFI_PWD_LEN         63
#define MAX_LWM2M_IDENTITY_LEN   64
#define MAX_LWM2M_PSK_LEN        64
#define MAX_DEVICE_MODE_LEN      20
#define MAX_DEVICE_POWER_LEN     10

/*Config fields names*/
#define WIFI_SSID_STR            "wifi_ssid"
#define WIFI_PASS_STR            "wifi_pwd"
#define WIFI_AP_SSID_STR         "wifi_ap_ssid"
#define WIFI_AP_PASS_STR         "wifi_ap_pwd"
#define LWM2M_SERVER_STR         "lwm2m_server"
#define LWM2M_PORT_STR           "lwm2m_port"
#define LWM2M_IDENTITY_STR       "lwm2m_id"
#define LWM2M_PSK_STR            "lwm2m_psk"
#define LWM2M_SECURE_STR         "lwm2m_secure"
#define LWM2M_BOOTSTR_STR        "lwm2m_bootstrap"
#define DEVICE_POWER_STR         "device_power"
#define SLEEP_INTERVAL_STR       "sleep_interval"
#define LWM2M_LIFETIME           "lwm2m_lifetime"
#define SYSLOG_SERVER_STR        "syslog_server"

typedef enum {
    DM_CONFIGURATION,
    DM_OPERATION,
    DM_DEBUG,
} device_mode_t;

typedef enum {
    DP_BATTERY,
    DP_DC,
} device_power_t;

typedef struct {
    char wifi_ssid[MAX_WIFI_SSID_LEN];
    char wifi_pwd[MAX_WIFI_PWD_LEN];
    char wifi_ap_ssid[MAX_WIFI_SSID_LEN];
    char wifi_ap_pwd[MAX_WIFI_PWD_LEN];
    char lwm2m_server[MAX_DOMAIN_NAME_LEN];
    int  lwm2m_port;
    char lwm2m_id[MAX_LWM2M_IDENTITY_LEN];
    char lwm2m_psk[MAX_LWM2M_PSK_LEN];
    bool lwm2m_secure;
    bool lwm2m_bootstrap;
    int lwm2m_lifetime;
    device_power_t device_power;
    int sleep_interval;
    char syslog_server[MAX_DOMAIN_NAME_LEN];
    bool cfg_is_dirty;
    device_mode_t device_mode;
} device_config_t;

static device_config_t _device_config = {
    .sleep_interval = 100,
    .lwm2m_port = -1,
    .lwm2m_secure = false,
    .lwm2m_bootstrap = false,
    .device_power = DP_DC,
    .cfg_is_dirty = false,
};

device_config_t *g_device_config = &_device_config;

typedef enum type_t { INT, STRING, BOOL } type_t;

struct field_desc_t;
typedef int (*config_handler_t)(cJSON *object, struct field_desc_t descriptor);
typedef struct field_desc_t {
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

    if ((int) (object->valuedouble) >= descriptor.min && (int) (object->valuedouble) <= descriptor.max )
	{
        *((int *)field_address) = (int) object->valuedouble;
        return 0;
	}
    else
	{
        fprintf(stderr, "Provided %s not in %d-%d range!\n", descriptor.name, descriptor.min, descriptor.max);
        return  -1;
    }
}

int write_string_callback(cJSON *object, struct field_desc_t descriptor)
{
    void *field_address = (char *) g_device_config + descriptor.offset;

    if (object->type == cJSON_String && (int) strlen(object->valuestring) < descriptor.max)
	{
        strncpy(field_address, object->valuestring, descriptor.max);
        return 0;
    }
    else
	{
        fprintf(stderr, "Provided %s is too long!\n", descriptor.name);
        return  -1;
    }
}

int write_bool_callback(cJSON *object, struct field_desc_t descriptor)
{
    void *field_address = (char *) g_device_config + descriptor.offset;

    if (object->type != cJSON_True && object->type != cJSON_False)
	{
        fprintf(stderr, "Provided %s has incorrect type!\n", descriptor.name);
        return  -1;
    }
    else
	{
        *((bool *)field_address) = object->type == cJSON_True;
        return 0;
    }
}

int write_power_callback(cJSON *object, struct field_desc_t descriptor)
{
    void *field_address = (char *) g_device_config + descriptor.offset;

    if(object->type == cJSON_String && (int) strlen(object->valuestring) < descriptor.max)
	{
        if (strncmp(object->valuestring, "DC", strlen(object->valuestring)) == 0)
		{
            *((device_power_t *)field_address) = DP_DC;
        }
		else if (strncmp(object->valuestring, "BATTERY", strlen(object->valuestring)) == 0)
		{
            *((device_power_t *)field_address) = DP_BATTERY;
        }
		else
		{
            fprintf(stderr, "Provided %s has incorrect type!\n", descriptor.name);
            return  -1;
        }
    }
	else
	{
        fprintf(stderr, "Provided %s has incorrect type!\n", descriptor.name);
        return  -1;
    }

    return 0;

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

int read_power_callback(cJSON *object, struct field_desc_t descriptor)
{
    void *field_address = (char *) g_device_config + descriptor.offset;

    *((device_power_t *)field_address) == DP_BATTERY
        ? cJSON_AddStringToObject(object, descriptor.name, "BATTERY")
        : cJSON_AddStringToObject(object, descriptor.name, "DC");

    return 0;
}



field_descriptor_t field_descriptor[]  = {
    {
        .name = WIFI_SSID_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, wifi_ssid),
        .max = MAX_WIFI_SSID_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = WIFI_PASS_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, wifi_pwd),
        .max = MAX_WIFI_PWD_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = WIFI_AP_SSID_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, wifi_ap_ssid),
        .max = MAX_WIFI_SSID_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = WIFI_AP_PASS_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, wifi_ap_pwd),
        .max = MAX_WIFI_SSID_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = LWM2M_SERVER_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, lwm2m_server),
        .max = MAX_DOMAIN_NAME_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = LWM2M_PORT_STR,
        .type = INT,
        .offset = offsetof(device_config_t, lwm2m_port),
        .max = 65535,
        .min = -1,
        .write_callback = write_int_callback,
        .read_callback = read_int_callback
    },
    {
        .name = LWM2M_IDENTITY_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, lwm2m_id),
        .max = MAX_LWM2M_IDENTITY_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = LWM2M_PSK_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, lwm2m_psk),
        .max = MAX_LWM2M_PSK_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {
        .name = LWM2M_SECURE_STR,
        .type = BOOL,
        .offset = offsetof(device_config_t, lwm2m_secure),
        .max = 1,
        .min = 0,
        .write_callback = write_bool_callback,
        .read_callback = read_bool_callback
    },
    {
        .name = LWM2M_BOOTSTR_STR,
        .type = BOOL,
        .offset = offsetof(device_config_t, lwm2m_bootstrap),
        .max = 1,
        .min = 0,
        .write_callback = write_bool_callback,
        .read_callback = read_bool_callback
    },
    {
        .name = LWM2M_LIFETIME,
        .type = INT,
        .offset = offsetof(device_config_t, lwm2m_lifetime),
        .max = 1000,
        .min = 0,
        .write_callback = write_int_callback,
        .read_callback = read_int_callback
    },
    {
        .name = DEVICE_POWER_STR,
        .type = INT,
        .offset = offsetof(device_config_t, device_power),
        .max = MAX_DEVICE_POWER_LEN,
        .min = 0,
        .write_callback = write_power_callback,
        .read_callback = read_power_callback
    },
    {
        .name = SLEEP_INTERVAL_STR,
        .type = INT,
        .offset = offsetof(device_config_t, sleep_interval),
        .max = INT_MAX,
        .min = 0,
        .write_callback = write_int_callback,
        .read_callback = read_int_callback
    },
    {
        .name = SYSLOG_SERVER_STR,
        .type = STRING,
        .offset = offsetof(device_config_t, syslog_server),
        .max = MAX_DOMAIN_NAME_LEN,
        .min = 0,
        .write_callback = write_string_callback,
        .read_callback = read_string_callback
    },
    {0}
};

/* const char *json_str = "{ \"wifi_ssid\":\"qwertyuiop\", \"wifi_pwd\":\"psk\", " */
/*             "\"lwm2m_secure\":true, \"sleep_interval\":1488, " */
/*             "\"lwm2m_port\":4321 }"; */



const char* json_str = "{"
    "  \"wifi_ssid\":       \"HUAWEI-D2mT\","
    "  \"wifi_pwd\":        \"485754431A3C7880\","
    "  \"wifi_ap_ssid\":    \"device_ap\","
    "  \"wifi_ap_pwd\":     \"1234567890\","
    "  \"lwm2m_server\":    \"leshan.eclipse.org\","
    "  \"lwm2m_port\":      -1,"
    "  \"lwm2m_id\":        \"magic_device\","
    "  \"lwm2m_psk\":       \"psk\","
    "  \"lwm2m_secure\":    true,"
    "  \"lwm2m_bootstrap\": true,"
    "  \"lwm2m_lifetime\":  228,"
    "  \"device_power\":    \"BATTERY\","
    "  \"sleep_interval\":   1488,"
    "  \"syslog_server\":   \"192.168.121.1\""
    "}";

void print_config(void)
{
    printf("Current config:\n");
    printf("-----------------------------\n");

    printf("%s: %s\n", WIFI_SSID_STR, g_device_config->wifi_ssid);
    printf("%s: %s\n", WIFI_PASS_STR, g_device_config->wifi_pwd);
    printf("%s: %s\n", WIFI_AP_SSID_STR, g_device_config->wifi_ap_ssid);
    printf("%s: %s\n", WIFI_AP_PASS_STR, g_device_config->wifi_ap_pwd);
    printf("%s: %s\n", LWM2M_SERVER_STR, g_device_config->lwm2m_server);
    printf("%s: %d\n", LWM2M_PORT_STR, g_device_config->lwm2m_port);
    printf("%s: %s\n", LWM2M_IDENTITY_STR, g_device_config->lwm2m_id);
    printf("%s: %s\n", LWM2M_PSK_STR, g_device_config->lwm2m_psk);
    printf("%s: %s\n", LWM2M_SECURE_STR, g_device_config->lwm2m_secure ? "true" : "false");
    printf("%s: %s\n", LWM2M_BOOTSTR_STR, g_device_config->lwm2m_bootstrap ? "true" : "false");
    printf("%s: %d\n", LWM2M_LIFETIME, g_device_config->lwm2m_lifetime);
    printf("%s: %s\n", DEVICE_POWER_STR, g_device_config->device_power ? "DC" : "BATTERY");
    printf("%s: %d\n", SLEEP_INTERVAL_STR, g_device_config->sleep_interval);
    printf("%s: %s\n", SYSLOG_SERVER_STR, g_device_config->syslog_server);

    printf("-----------------------------\n");
}




int json_to_config(const char *json_str)
{
    cJSON *root;
    cJSON *subitem;

    root = cJSON_Parse(json_str);
    if (!root || (root->type != cJSON_Object))
	{
        return -1;
    }

    subitem = root->child;

    while (subitem)
	{

        for (int i = 0; field_descriptor[i].name != 0; i++)
		{

            field_descriptor_t descriptor = field_descriptor[i];
            if (strncmp(subitem->string, descriptor.name,strlen(subitem->string)) == 0)
			{
                descriptor.write_callback(subitem, descriptor);
            }
        }
        subitem = subitem->next;
    }

    cJSON_Delete(root);

    return 0;
}

cJSON* config_to_json(void)
{
    cJSON *root;

    root = cJSON_CreateObject();

    for (int i = 0; field_descriptor[i].name != 0; i++)
	{
        field_descriptor_t descriptor = field_descriptor[i];
        descriptor.read_callback(root, descriptor);
    }

    return root;
}

int main(void)
{
    cJSON *root = config_to_json();

    char *rendered = cJSON_Print(root);

    printf("%s", rendered);

    cJSON_Delete(root);

    return 0;
}
