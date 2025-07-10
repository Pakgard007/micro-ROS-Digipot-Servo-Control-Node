#include <Servo.h>
#include <SPI.h>
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <geometry_msgs/msg/twist.h>

// ----- Debug Mode -----
#define DEBUG_MODE true

// ----- Servo -----
Servo servoMotor;
const int servoPin = 3;
int pulseWidth = 800;  // ค่าเริ่มต้นของเซอร์โว

// ----- SPI ตั้งค่า DIGI POT -----   
#define CS_DIGIPOT  7  

void write_digipot(int val) {
  digitalWrite(CS_DIGIPOT, LOW);
  SPI.transfer(B00010001);  // คำสั่งเขียนไปยัง Wiper Register
  SPI.transfer(val);        // ส่งค่าความต้านทาน (0-255)
  digitalWrite(CS_DIGIPOT, HIGH);
}

// ----- กำหนด output pins -----
#define OUT_PIN0 0
#define OUT_PIN1 1
#define OUT_PIN2 2  // ใช้แทน OUT_PIN3 สำหรับเปิดหลังจาก OUT_PIN0 หรือ OUT_PIN1

// ----- micro-ROS ตัวแปร -----
rcl_subscription_t subscriber;
geometry_msgs__msg__Twist twist_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

float prev_x = 0.0;  

void error_loop() {
  while (1) {
    if (DEBUG_MODE) Serial.println("Error: Entering infinite loop...");
    delay(100);
  }
}

// Callback function รับข้อความจาก topic /cmd_vel
void cmd_vel_callback(const void * msgin) {
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *) msgin;
  float x = msg->linear.x;

  if (DEBUG_MODE) {
    Serial.print("Received linear.x: ");
    Serial.println(x);
  }

  // ควบคุมทิศทางและ DigiPot
  if (x == 2.0) {
    pulseWidth = 800;  // ตั้งค่าคงที่ 800
    digitalWrite(OUT_PIN0, HIGH);
    digitalWrite(OUT_PIN1, LOW);
    delay(100);
    digitalWrite(OUT_PIN2, HIGH);
    write_digipot(100);
    if (DEBUG_MODE) Serial.println("x == 1: OUT_PIN0 ON -> OUT_PIN2 ON -> (128)");
  } 
  else if (x == -1.0) {
    pulseWidth = 800;  // ตั้งค่าคงที่ 800
    digitalWrite(OUT_PIN1, HIGH);
    digitalWrite(OUT_PIN0, LOW);
    delay(100);
    digitalWrite(OUT_PIN2, HIGH);
    write_digipot(100);
    if (DEBUG_MODE) Serial.println("x == -1: OUT_PIN1 ON -> OUT_PIN2 ON -> (128)");
  } 
  else { 
    digitalWrite(OUT_PIN0, LOW);
    digitalWrite(OUT_PIN1, LOW);
    digitalWrite(OUT_PIN2, LOW);
    write_digipot(0);

    // ค่อยๆ เพิ่มค่า pulseWidth จาก 800 ไป 900
    if (pulseWidth < 880) {
      pulseWidth += 2;  // ปรับค่าขึ้นทีละ 2
    }

    if (DEBUG_MODE) Serial.println("x != 1 and x != -1: Increasing pulseWidth...");
  }

  prev_x = x;
}

void setup() {
  Serial.begin(115200);
  set_microros_transports();
  delay(2000);

  // ตั้งค่าพิน output
  pinMode(OUT_PIN0, OUTPUT);
  pinMode(OUT_PIN1, OUTPUT);
  pinMode(OUT_PIN2, OUTPUT);
  digitalWrite(OUT_PIN0, LOW);
  digitalWrite(OUT_PIN1, LOW);
  digitalWrite(OUT_PIN2, LOW);

  pinMode(CS_DIGIPOT, OUTPUT);
  digitalWrite(CS_DIGIPOT, HIGH);
  SPI.begin();

  // ตั้งค่า micro-ROS
  allocator = rcl_get_default_allocator();
  if (rclc_support_init(&support, 0, NULL, &allocator) != RCL_RET_OK) {
    error_loop();
  }
  
  if (rclc_node_init_default(&node, "cmd_vel_digipot_node", "", &support) != RCL_RET_OK) {
    error_loop();
  }
  
  if (rclc_subscription_init_default(&subscriber, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
      "/final_cmd_vel") != RCL_RET_OK) {
    error_loop();
  }
  
  if (rclc_executor_init(&executor, &support.context, 1, &allocator) != RCL_RET_OK) {
    error_loop();
  }
  
  if (rclc_executor_add_subscription(&executor, &subscriber, &twist_msg,
      &cmd_vel_callback, ON_NEW_DATA) != RCL_RET_OK) {
    error_loop();
  }
  
  Serial.println("micro-ROS node initialized, waiting for /cmd_vel messages...");

  // ตั้งค่าเซอร์โว
  servoMotor.attach(servoPin, 800, 2200);
}

void loop() {
  if (rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)) != RCL_RET_OK) {
    if (DEBUG_MODE) Serial.println("Error: micro-ROS executor failure");
    error_loop();
  }

  // อัปเดตค่าเซอร์โว
  servoMotor.writeMicroseconds(pulseWidth);
  delay(20);  // ทำให้การเคลื่อนที่สมูท

  delay(5);
}
