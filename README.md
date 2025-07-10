
---

# 🧠 micro-ROS Digipot & Servo Control Node

This project implements a `micro-ROS` node on an Arduino-compatible microcontroller that listens to `/final_cmd_vel` topic and controls:

* A **Digital Potentiometer** (via SPI)
* A **Servo Motor**
* Three **Digital Output Pins** to control direction logic

### ✅ Features

* Subscribes to `geometry_msgs/msg/Twist` from topic `/final_cmd_vel`
* Uses `linear.x` value to control:

  * Servo pulse width (for motor control or angle modulation)
  * Digital potentiometer (resistance for current/voltage adjustment)
  * GPIOs for directional control
* Gradual decay of `pulseWidth` when command is neutral
* Debug prints via Serial Monitor

---

## 📦 Hardware Requirements

* Arduino (compatible with micro-ROS and SPI)
* Digital Potentiometer (controlled via SPI) — e.g., MCP41xxx
* Servo motor
* 3x digital output control lines (OUT\_PIN0, OUT\_PIN1, OUT\_PIN2)
* ROS 2 running on host computer (e.g., Jetson, NUC) with Micro-ROS Agent

---

## 🧩 ROS 2 Integration

This node listens to:

```bash
/final_cmd_vel  (geometry_msgs/msg/Twist)
```

### Command Mapping (`linear.x`)

| Value of `x` | Action                                                                                                          |
| ------------ | --------------------------------------------------------------------------------------------------------------- |
| `2.0`        | Sets `OUT_PIN0` HIGH, `OUT_PIN1` LOW, and triggers `OUT_PIN2` after delay. Digital potentiometer is set to 100. |
| `-1.0`       | Sets `OUT_PIN1` HIGH, `OUT_PIN0` LOW, then `OUT_PIN2` HIGH. Digital potentiometer also set to 100.              |
| Other        | All pins set to LOW, potentiometer to 0. Servo `pulseWidth` increases gradually to simulate smooth stopping.    |

---

## 🔧 Code Structure Overview

### Initialization

```cpp
set_microros_transports();     // Setup transport layer
SPI.begin();                   // Start SPI for digipot
servoMotor.attach(...);       // Attach and configure servo
```

### micro-ROS Setup

```cpp
rclc_support_init(...)
rclc_node_init_default(...)
rclc_subscription_init_default(...)
rclc_executor_add_subscription(...)
```

### Servo Update Loop

```cpp
servoMotor.writeMicroseconds(pulseWidth);  // Updates every loop
```

---

## 🧪 Debug Mode

Enable `DEBUG_MODE` in the code to print serial debug messages.

```cpp
#define DEBUG_MODE true
```

---

## 🔄 Example Output (Serial Monitor)

```
Received linear.x: 2.0
x == 1: OUT_PIN0 ON -> OUT_PIN2 ON -> (128)
```

---

## 📌 Notes

* Servo's pulse width starts at `800` and slowly increases to `900` when stopped.
* You can customize `pulseWidth` range or digipot value range (0–255) based on your application.

---

## 🛠️ Future Improvements

* Add velocity smoothing with filters
* Add timeout or safety lock when messages are lost
* Expand to handle `angular.z` for turning control

---
