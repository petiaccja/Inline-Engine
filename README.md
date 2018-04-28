
# Inline Engine

[![Build status](https://ci.appveyor.com/api/projects/status/rdyulmuoq5cc64o9?svg=true)](https://ci.appveyor.com/project/petiaccja/inline-engine)

Introduction
---
Inline Engine is a modern game engine built with latest C++ that provides/will provide solutions to existing common industrial problems:
- Smoother Editor User Experience  
    (Achieved by our experience from multiple big engine editors + GUI lib written from scratch)
    
- Higher Performance Editor & Engine  
   (Whole Editor will be async thus making you more productive + Heavy experience with optimizations )
    
- Better Stability & More Focus On The Community  
    (Will give smaller percent time for feature implementation compared to bug fixes)

- Better PBR Graphics than the typical industrial PBR equations   
    (W.I.P state of the art researches (already have great results))

The Engine leverages the power of DirectX 12 and similar APIs. It got its name from the overuse of inline methods to speed up the software, however, it's not a victim of such behavior. The Engine is component based so you can tear it to pieces and for example integrate the Graphics, Network, Physics, etc modules individually into an another complete game engine or real-time simulation environments.

Platforms
---
1. Windows (supported)
2. Linux   (planned)
4. XBoxOne (planned)
3. PS4     (planned)

How To Build
---

**Recommended**:

Prerequisites: Visual Studio 2017 with latest updates, CMake 3.10 or later

1. Generate Visual Studio solution files via CMake for Inline-Engine/CMakeLists.txt
2. Open and build solution files

**Alternatively**:

Prerequisites: latest MSVC 19 toolchain, CMake compatible IDE (e.g. Visual Studio, CLion)

1. Open the folder Inline-Engine with the CMakeLists.txt
2. Build from the IDE

Note: you can *not* compile it with the GNU toolchain because the Direct3D libraries won't compile, and Vulkan is not supported yet.

Note: you don't need any additional dependencies, all libraries are packed with the project.

**What to do after build**:
You can run  QC_Simulator and play around flying a quadcopter. That project tests the rendering pipeline, and effect can be seen in action.

Rendering core ideas
---
### High-level interface
The engine exposes geometries, material, and textures to work with. These are used as properties to describe an entity. Entities are grouped into multiple scenes that act like a virtual world. One might put terrain entities, lights, geometries or other types of objects into a scene. The purpose of having multiple scenes is to achieve the separation of 3D world, GUI and debug draw objects. With this, one might define completely different ways of rendering each scene, and can quickly toggle whether to display a scene at all. The results of each scene can be freely combined with desired final output.

### Visual render pipeline scripting
Inspired by CryEngine's flow graph and Unreal's BluePrint, users can leverage the power of task graphs to visually assemble the render pipeline. The task graph's tasks have access to the scenes and the objects inside them, so they can render it. The data flow from one task to the other allows the transport of opacity or depth maps which make combining scenes in an arbitrary fashion a breeze.

### Adding your own rendering algorithms
The above-mentioned rendering task graph provides an interface to implement custom task nodes. The nodes inputs and outputs are defined, and the programmer can code the data transform performed on the inputs. The node can access the underlying Direct3D 12 API through a simplified interface. The framework that executes the task graph takes the responsibility of distributing work across multiple CPU cores and the scheduling of generated GPU command lists.
