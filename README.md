Inline Engine
===

Inline Engine is a game graphics library that leverages the power of DirectX 12 and similar APIs. It got its name from the overuse of inline methods to speed up the software, however, it's not a victim of such behaviour. The engine is aimed at integration into complete game engines and real-time simulation environments.

Usage
---
Currently, the engine is not yet available for integration or public use, as it's still in its infancy.

Core ideas
---
### High-level interface
The engine exposes geometries, material and textures to work with. These are used as properties to describe an entity. Entities are grouped into multiple scenes that act like a virtual world. One might put terrain entities, lights, geometries or other types of objects into a scene. The purpose of having multiple scenes is to achieve the separation of 3D world, GUI and debug draw objects. With this, one might define completely different ways of rendering each scene, and can quickly toggle whether to display a scene at all. The results of each scene can be freely combined for desired final output.

### Visual render pipeline scripting
Inspired by CryEngine's flow graph and Unreal's BluePrint, users can leverage the power of task graphs to visually assemble the render pipeline. The task graph's tasks have access to the scenes and the objects inside them, so they can render it. The data flow from one task to the other allows the transport of opacity or depth maps which make combining scenes in an arbitrary fashion a breeze.

### Adding your own rendering algorithms
The above-mentioned rendering task graph provides an interface to implement custom task nodes. The nodes inputs and outputs are defined, and the programmer can code the data transform performed on the inputs. The node can access the underlying Direct3D 12 API through a simplified interface. The framework that executes the task graph takes the responsibility of distributing work accross multiple CPU cores and the scheduling of generated GPU command lists.

