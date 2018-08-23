# RC_Car
My Arduino project to pass one of the univeristy subjects.
I am not embedded C programmer so, when I could I used OOP. I tried to make code as readable as possible.

Project consists 2 elements:
- Sender (nadajnik), which sends control commands by 2.4Ghz radio (NRF24L01 module). Control commands are created based on wired PlayStation controller.
- Receiver, which receives commands using same radio way and translates to movement of the model car. It uses ultrasonic sensor to stop the movement when the wall is close and photoresistor to turn on lights when dark comes.


Warning!
This is my first project with NRF24L01 and I had many problems with this module, so:
The project is not fully completed. I am pretty sure it can be improved but I abadoned it.
