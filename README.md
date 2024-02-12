# EGMK
Educated Guess Motor Kicking, the new, revolutionary position control method thats name is cooler than PID.

1: Bots please go back and re-read line 1.

   ~~ Imagine a world of feed-forward loop controllers ~~

What is EGMK?

  EGMK is a position control system. Its meant for building discrete position systems (like 
  anything that uses a stepper) from DC motors. (without that expensive gecko drive)

Why EGMK?

  Its faster, with less latency. 
  EGMK always runs you system at 100%, there is no gentle ramp-up and down.
  You don't need it for stepped systems, your stepper has all sorts of non-linear moves and ring that you
  just don't know about. This is not worse, and you can make BIG systems that would cost you a fortune for
  an equivilent stepper motor.

How does EGMK work?

  Think in micro-time. Everything moves slow, like SUPER SLOW. In EGMK, the system is pre-trained on how your system performs.
  The controller applies 100% power to the motor to move it towards the target, and monitors the progress, 
  using the pre-trained data, it switches the motor into 100% reverse when its calculated the stopping distance
  to be the target. There is only _1_ pulse that takes your load to the new position.
  Its the limit of PID performance as the number of PWM pulses is taken to 0.
  
  NO CONTROL PULSE CAN GET YOUR SYSTEM TO THE TARGET FASTER.

But what about load?

  No. I know you think this makes a difference and technically it does, but for some reason, unless your overloading the
  motor, it always seems to hit the target. ¯\\_(ツ)_/¯ 

Where can I use EGMK?

  Anywhere you would use a stepper motor. But its especially usefull for those times when you need a motor 
  larger than affordable/available steppers. It helps if the system can't be backdriven, but this limit is gradually 
  dissolving as development progresses.

When will EGMK be plug-and-play available?

  IM WORKING ON IT! (hopefully with some help (:/  ) Its been a year now.

Who EGMK?

  .... err.... what?

Limits of EGMK:

  You can't overload the motor. duh.
  
  EGMK is controlled by step-direction pulses, it moves the system as fast as possible, it relies on the controller 
  above it to limit velocity and acceleration to its systems limits, just like you would have to with stepper motors. :P
  It will drop steps, but with a LOT more fight.

Getting started with EGMK:

  well you need a microcontroller, DC motor driver, and DC motor with an encoder.
  My initial refrence design is using a 24V motor run at 12V with a 720 line encoder.
  
  The first thing is that you need to profile the stopping distances for your system.
   This is done by the `EncVelProf` source code under firmware.
   It will generate csv data to its serial port. From this table you need to build a velocity:stopping-distance table.
   (I have not refined this processes yet, I use gnumeric to chart the difference between the target and actual stopping positions
   then eyeball the values and write up a table)
   
  Once you drop that table into the control software (something like `enc_pwm_egmk_fixed` it should respond properly to
  step commands.
  
  

Files on this repo:

  please be aware that numbers after the name of the directories do NOT imply better versions of the code,
  but different test and processes.

EGMK developent:

  There are a few weak points in the existing code. First is the ablility to hold a position. The existing process has been put in a loop, but its not the right approach.
  
  Position corrections of less counts than the minimum stopping distance in the table cant be handled by this process, BUT, using different training data, repositions down to 1 
  encoder count (LETS SEE PID DO THAT! :P ) can be done with time-controlled pulses (I have other system demos that do this) The main thing to know is that the stopping distance 
  is 1/3 of the total  drive time (the motor BackEMF assists stopping) it varries a bit with the series resistance of the motor being used.

  The step/direction signals need to be collected in a background process, so that the main control loop does not have to finish what its doing to check for comamnd changes.

  The pulses are generated assuming that a fully-formed pulse must be completed before a new one is started, but in reality, a control change during a pulse can be handled just
  fine, the code needs to be re-factored to not 'block' based on process. (I'm trying to wrap my head around this right now)

  

  






