#include <string.h>
#include <stdio.h>

#include "parson.h"

extern void get_data_vbr(float * vibration);
extern void get_data_imu(float *gx, float *gy, float *gz, float *degx, float *degy, float *degz);
extern void get_data_gps(float *lat, float *longt, float *speed, float *gn, float *gp, float *gl);

extern const char * get_dev_id(void);

void json_convert(char * str) {
    float latitude;
    float longitude;
    float speed;
    float gn_hdop;
    float gp_hdop;
    float gl_hdop;
    float gx;
    float gy;
    float gz;
    float degx;
    float degy;
    float degz;
    float vibration;

    get_data_vbr(&vibration);
    get_data_imu(&gx, &gy, &gz, &degx, &degy, &degz);
    get_data_gps(&latitude, &longitude, &speed, &gn_hdop, &gp_hdop, &gl_hdop);


    JSON_Value *root_value = json_value_init_object();
    JSON_Value *data_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Object *data_object = json_value_get_object(data_value);
    char * serialized_string = NULL;

    json_object_set_number(data_object, "latitude", latitude);
    json_object_set_number(data_object, "longitude", longitude);
    json_object_set_number(data_object, "speed", speed);
    json_object_set_number(data_object, "gn_hdop", gn_hdop);
    json_object_set_number(data_object, "gp_hdop", gp_hdop);
    json_object_set_number(data_object, "gl_hdop", gl_hdop);
    json_object_set_number(data_object, "gx", gx);
    json_object_set_number(data_object, "gy", gy);
    json_object_set_number(data_object, "gz", gz);
    json_object_set_number(data_object, "degx", degx);
    json_object_set_number(data_object, "degy", degy);
    json_object_set_number(data_object, "degz", degz);
    json_object_set_number(data_object, "vibration", vibration);

    json_object_set_string(root_object, "sdid", get_dev_id());
    json_object_set_string(root_object, "type", "message");
    json_object_set_value(root_object, "data", data_value);
    
    serialized_string = json_serialize_to_string_pretty(root_value);
    strcpy(str,serialized_string);

    json_free_serialized_string(serialized_string);
    json_value_free(data_value);
    json_value_free(root_value);
}
