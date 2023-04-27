## the region to verify
### dormitoryIOT
the main function is receive messages from device and store them to sub_topics. And createing the security, message handler, controle panel thread to operate some actual operations.(this means that it entrust the actual task to system)
- receive messages from device and store them to sub_topics
- create security, message handler, controle panel thread.

## the systemIOT to verify
### lighting system
the lighting system manage all the devices belong to lighting system. And groups this devices to improve the ability of managing devices.
The main operation is the control_panel(), it manage this devices based on some tatics.
- groups devices
- manage deivces based on control panel interface
### security system

## the devices to verify
the led device is model to manage the actual device. it store the device properties and status.
- record the device properties
- synced the device properties to actual device
### led device
- record the led pin number
- record the led status
- synced the desired status to actual device
### dth11 device
- record the temperature
- record the humidity
### mq2 device
- record the smoke value
- recodr the alarm stattus
- synced the alarm status to actual device

## the dormitoryIOT setup 
- initialize the dormitory instance
- add all the systems belong to region
- add the security system belong to region
- running region_thread() to create security thread, message handler thread, controle panel thread

## the systemIOT setup
### lighting system
- intialize the lighting instance
- add all devices belong to the system
- add all the subsystems belong to the system
- add all the related systems with the lighting system
- setup lighting groups
- 
### security system
- intialize the security system instance
- add all devices belong to the system
- add all the subsystems belong to the system
- add all the related systems with the lighting system
- 

## the devices setup
### led device
- intialize the led deivce instance
- 
### dth11 device
- intialize the dth11 device instance
- 
### mq2 device
- intialize the mq2 device instance
- 
