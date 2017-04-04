Inline Engine
===

Notice
---
Please note that facilities other than graphics have recently been added to this repository. These features aim to implement a full-featured game engine. We will update the readme as soon as it becomes more clear to us how we will manage the repository. The description below still stands for the graphics engine part.

Introduction (graphics)
---
Inline Engine is a game graphics library that leverages the power of DirectX 12 and similar APIs. It got its name from the overuse of inline methods to speed up the software, however, it's not a victim of such behaviour. The engine is aimed at integration into complete game engines and real-time simulation environments.

Usage
---
### Integrate into your project
Currently, the engine is not yet available for integration or public use, as it's still in its infancy.

### Play around
Download the project, open the solution in Visual Studio 2017 RC (2015 might do as well, but not tested), and hit compile. It does not do much yet, but there is a quadcopter simulator with which you can play or tweak the PID controller. Note that you need a graphics card that supports DirectX 12 (feature level 11_0) and appropriate drivers.

Core ideas
---
### High-level interface
The engine exposes geometries, material and textures to work with. These are used as properties to describe an entity. Entities are grouped into multiple scenes that act like a virtual world. One might put terrain entities, lights, geometries or other types of objects into a scene. The purpose of having multiple scenes is to achieve the separation of 3D world, GUI and debug draw objects. With this, one might define completely different ways of rendering each scene, and can quickly toggle whether to display a scene at all. The results of each scene can be freely combined for desired final output.

### Visual render pipeline scripting
Inspired by CryEngine's flow graph and Unreal's BluePrint, users can leverage the power of task graphs to visually assemble the render pipeline. The task graph's tasks have access to the scenes and the objects inside them, so they can render it. The data flow from one task to the other allows the transport of opacity or depth maps which make combining scenes in an arbitrary fashion a breeze.

### Adding your own rendering algorithms
The above-mentioned rendering task graph provides an interface to implement custom task nodes. The nodes inputs and outputs are defined, and the programmer can code the data transform performed on the inputs. The node can access the underlying Direct3D 12 API through a simplified interface. The framework that executes the task graph takes the responsibility of distributing work accross multiple CPU cores and the scheduling of generated GPU command lists.

Legal
---
There is no licence yet, so the code is 'const' or 'readonly', unavailable for redistribution in any form. We are too lazy to figure out which licence to add, and will not be doing so unless it becomes necessary.

