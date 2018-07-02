#include <string.h>
#include <stdlib.h>

#include "MQTTPacket.h"

#include "typedefs.h"

#define DEVICE_ID 12345

void mqtt_packet(char * payload, char * packet)
{
   MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

   MQTTString topic_string = MQTTString_initializer;
   topic_string.cstring = "/v1.1/messages/3dcc6661784f4bd2b21dc0a8ccf47084";

   char packet_buf[PACKET_MAX_LEN];
   int payload_len = strlen(payload);
   int current_len = 0;

   data.clientID.cstring = "type1";
   data.keepAliveInterval = 20;
   data.cleansession = 1;
   data.username.cstring = "3dcc6661784f4bd2b21dc0a8ccf47084";
   data.password.cstring = "c8f17e3149174eb18959dce27b6381ff";
   data.MQTTVersion = 4;

   current_len = MQTTSerialize_connect((unsigned char *)packet_buf, PACKET_MAX_LEN, &data);


   current_len += MQTTSerialize_publish((unsigned char *)(packet_buf + current_len),
                                PACKET_MAX_LEN - current_len, 0, 0, 0, 0,
                                topic_string, (unsigned char *)payload, payload_len);

   current_len += MQTTSerialize_disconnect((unsigned char *)(packet_buf + current_len),
                                   PACKET_MAX_LEN - current_len);

   strcpy(packet, packet_buf);
}
