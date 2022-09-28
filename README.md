# solder-workspace-lighting_firmware
The C++ firmware for our *solder-workspace-lighting* **[central control](https://github.com/netz39/solder-workspace-lighting)**.

An [overview](https://wiki.netz39.de/projects:2021:solder_workspace_lighting) (German) of this complete project is also available.

## Firmware structure
This firmware contains the CubeHal and intialization code generated by CubeMX. Both are in "cubemx" folder. The rest is logic code written in C++.

This firmware is designed to work without global variables and so-called "externs" declarations. This is also known as [Dependency injection](https://en.wikipedia.org/wiki/Dependency_injection).
To implement this, an application class is created, from there finally the whole classes with the logic code and also tasks are created.
C-style callbacks from CubeHAL are passed on over the Application class to the relevant class objects as call of its member functions.

With this structure one can clearly understand, which dependencies between the objects exist. This handling is especially important for unit testing.

## RTOS Tasks
RTOS tasks are integrated in classes. These classes derive from a base class, which automatically creates a task when constructing the class. With this a class has to override its member `taskMain` function. After Application class is completely finished with the initializing the tasks will be called.

| classes with RTOS task     | description |
|-------------------|-------------|
| AnalogToDigital     | convert analog values to digital (LED temperatures) |
| Encoder       | reading encoder output |
| EncoderButtonHandler | polling the input of encoder button and update state |
| LedFading   | controls LED incl. fading  |
| OledDisplay   | controls and draw content on oled display over SPI |