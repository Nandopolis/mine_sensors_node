#include <ros.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/UInt16MultiArray.h>

enum {
  SENSORS_NUM = 2,
};

//Set up the ros node and publisher
std_msgs::UInt8 mine_detected_msg;
ros::Publisher pub_mine_detected("temperature", &mine_detected_msg);
std_msgs::UInt16MultiArray mine_sensors_msg;
ros::Publisher pub_mine_sensors("temperature", &mine_sensors_msg);
ros::NodeHandle nh;

uint32_t last_time;
uint32_t mine_sensors_period = 11;
uint16_t sensors_adc[SENSORS_NUM];
uint16_t sensor_thr[SENSORS_NUM] = {512, 512};
uint8_t publishing[SENSORS_NUM] = {0, 0};
uint8_t published[SENSORS_NUM] = {0, 0};
uint8_t i = 0;

void setup() {
  // put your setup code here, to run once:
  nh.initNode();
  nh.advertise(pub_mine_detected);
  mine_sensors_msg.layout.dim = (std_msgs::MultiArrayDimension *)malloc(sizeof(std_msgs::MultiArrayDimension) * 2);
  mine_sensors_msg.layout.dim[0].label = "height";
  mine_sensors_msg.layout.dim[0].size = 2;
  mine_sensors_msg.layout.dim[0].stride = 1;
  mine_sensors_msg.layout.data_offset = 0;
  mine_sensors_msg.data = (int *)malloc(sizeof(int) * 4);
  mine_sensors_msg.data_length = 2;
  nh.advertise(pub_mine_sensors);

  last_time = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - last_time > mine_sensors_period) {
    sensors_adc[0] = analogRead(A0);
    sensors_adc[1] = analogRead(A1);

    mine_detected_msg.data = 0;
    if (sensors_adc[0] > sensor_thr[0]) {
      mine_detected_msg.data += 1;
    }
    if (sensors_adc[1] > sensor_thr[1]) {
      mine_detected_msg.data += 2;
    }
    if (mine_detected_msg.data != 0) {
      if (mine_detected_msg.data == 3 && (!published[0] || !published[1])) {
        published[0] = 1;
        published[1] = 1;
        pub_mine_detected.publish(&mine_detected_msg);
      }
      else {
        if (mine_detected_msg.data == 2 && (!published[1])) {
          published[1] = 1;
          pub_mine_detected.publish(&mine_detected_msg);
        }
        else if (mine_detected_msg.data == 1 && (!published[0])) {
          published[0] = 1;
          pub_mine_detected.publish(&mine_detected_msg);
        }
      }
    }
    else if (published[0] || published[1]) {
      published[0] = 0;
      published[1] = 0;
      pub_mine_detected.publish(&mine_detected_msg);
    }

    i++;
    if (i == 3) {
      mine_sensors_msg.data[0] = sensors_adc[0];
      mine_sensors_msg.data[1] = sensors_adc[1];
      pub_mine_sensors.publish(&mine_sensors_msg);
    }
  }
  nh.spinOnce();
}
