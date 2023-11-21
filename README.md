# Saturn
![Saturn PBR scene](https://i.gyazo.com/c15de75c72d6bd36e984097bcd29abcb.png)

Saturn is a lightweight 3D game engine. Its primary purpose is for me to learn as much as possible about rendering 3D graphics, engine design, and game development. A constant thought throughout the development has been about who uses the engine, what tools they need, and how to use those tools. The engine is built with C++ and Vulkan. It is based on an Entity Component System with [EnTT](https://github.com/skypjack/entt/tree/master) and uses physics powered by [Nvidia PhysX](https://github.com/NVIDIAGameWorks/PhysX). The primary rendering technique is Forward+ with PBR, but the pipeline is designed for the future use of other rendering techniques like raytracing and, hopefully soon, custom shaders to make custom material effects possible.

The project is built using cmake. Simply clone the repository and execute the Build_Project.bat, you will find the generated files in the build directory or run cmake in the root directory manually with: `cmake -B "build"`

Saturn can be used as a static library if you want to create the application in C++ or used with the accompanying editor. The static library is located under Engine/lib after you build it with an appropriate configuration. 
The editor is used to manipulate objects, simulate scenes, and view changes in real-time. The engine and editor are in their infancy and while you cannot yet export an editor project, you can create new projects, save and load them and their contained scenes.
