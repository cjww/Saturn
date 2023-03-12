# Saturn
Saturn is a 3D game engine built on vulkan. It's primary purpose is for me to learn as much as possible about game development. A constant thought throughout the development has been of who uses the engine, what tools they need and how to use those tools. It is built on the ECS library EnTT and makes use of Lua JIT and Nvidia PhysX. The primary rendering technique is Forward+, but the rendering pipeline is designed for the future use of other rendering techniques like raytracing and hopefully soon custom pipelines to make custom shader effects.

Saturn can be used as a static library if you want to create the application in C++ (Lua scripts are of course also supported). This of course offers the most flexibility. Saturn also comes with a editor to manipulate objects, simulate scenes and view changes in real-time. The engine and editor is in its infancy and while you cannot yet export an editor project, you can create new projects, save and load them and thier contained scenes.
