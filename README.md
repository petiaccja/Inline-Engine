# Inline Engine

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/578452689f1f4138a07dcb2fd0e067f2)](https://app.codacy.com/app/petiaccja/Inline-Engine?utm_source=github.com&utm_medium=referral&utm_content=petiaccja/Inline-Engine&utm_campaign=Badge_Grade_Dashboard)
[![Build status](https://ci.appveyor.com/api/projects/status/rdyulmuoq5cc64o9?svg=true)](https://ci.appveyor.com/project/petiaccja/inline-engine)


Introduction
---

Inline-Engine is a modern game engine that emphasizes the latest technologies and innovation in order to bring the most to the table regardless of weather you are a gamer, a developer or a game designer. It got its name from the overuse of inline methods to speed up the software, however, it only aims to live up to the performance expectations not the poor coding style.

- C++17 brings elegant coding solutions
- Built for parallelism from the ground up
- Build for DirectX 12 and Vulkan
- Modular, hackable design
- Highly configurable even without hacking

Platforms
---

1. Windows (supported)
2. Linux   (planned)
4. XBoxOne (planned)
3. PS4     (planned)

How To Build
---

**Visual Studio solution files**:

1. Download Visual Studio 2017 or 2019 preview with the latest updates
2. Download the latest CMake (3.10 or newer)
3. Generate Visual Studio 2017/2019 Win64 solution files via CMake for Inline-Engine/CMakeLists.txt
4. Open solution files and build

**CMake via IDE**:

1. Install latest MSVC 19 toolchain
2. Download a CMake compatible IDE (e.g. Visual Studio or CLion)
3. Open Inline-Engine/CMakeLists.txt
4. Configure CMake within IDE to use the MSVC toolset
5. Build

**CMake via Ninja or makefiles**:

1. Install latest MSVC 19 toolchain
2. Download the latest CMake (3.10 or newer)
3. Generate Ninja or Makefile for Inline-Engine/CMakeLists.txt
4. Build


Note: you can *****not***** compile it with the GNU toolchain because the Direct3D libraries won't compile, and Vulkan is not supported yet.

Note: you don't need any additional dependencies, all libraries are packed with the project.

Note: 32-bit build are *****not***** supported

Rendering core ideas
---
### High-level interface
The engine exposes geometries, material, and textures to work with. These are used as properties to describe an entity. Entities are grouped into multiple scenes that act like a virtual world. One might put terrain entities, lights, geometries or other types of objects into a scene. The purpose of having multiple scenes is to achieve the separation of 3D world, GUI and debug draw objects. With this, one might define completely different ways of rendering each scene, and can quickly toggle whether to display a scene at all. The results of each scene can be freely combined with desired final output.

### Visual render pipeline scripting
Inspired by CryEngine's flow graph and Unreal's BluePrint, users can leverage the power of task graphs to visually assemble the render pipeline. The task graph's tasks have access to the scenes and the objects inside them, so they can render it. The data flow from one task to the other allows the transport of opacity or depth maps which make combining scenes in an arbitrary fashion a breeze.

### Adding your own rendering algorithms
The above-mentioned rendering task graph provides an interface to implement custom task nodes. The nodes inputs and outputs are defined, and the programmer can code the data transform performed on the inputs. The node can access the underlying Direct3D 12 API through a simplified interface. The framework that executes the task graph takes the responsibility of distributing work across multiple CPU cores and the scheduling of generated GPU command lists.
