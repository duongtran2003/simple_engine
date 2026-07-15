# Simple Engine

A real-time rendering engine written in C++, utilizing **Vulkan** as its backend for rendering. This is my hobby project, serves as a playground for me to explore the world of modern graphics programming. 

## What I've done

While this project is still in its very early stages, it has a few things done for a barebone rendering engine, including:
* "Good enough" material system.
* Render graph that tracks each pass' inputs and outputs, enabling itself to automatically sort its passes on compile using topology sort.
* A good amount of Vulkan boilerplate done such as Descriptors Indexing (Bindless Descriptors), SSBO.
* Some UI with ImGui. Very bare minimum, only have camera settings at the moment, I will leave it for later as UI is not my main interest at this stage of the project.
* GLTF models and scenes loader (And will only support gltf, preferably not binary gltf).
* Basic blinn-phong implemented.
* Basic normal mapping. The engine is also capable of calculating tangent if not provided by the model
* Automatic memory barrier placement for image layout transition. While this sounds and looks convenient, won't provide the best performance you can achieve.
* Basic camera movement. You can fly around, adjusting view direction using mouse lock (L)

## How does the render graph work?

While it's not the best implementation of a render graph in term of flexibility, robustness and functionality, by providing my own implementation, it helps me fully understand what it can and can't do, making refactoring and extending much easier. Using the same pool of resources for all the passes greatly reduces memory usage, though might make synchronization a huge headache.

My render graph work on three stages:
1. **Resources and passes creation:** During this stage, you setup resources (could be color image, depth image, etc...) and renderpasses. Each pass should provide its inputs, outputs and callback function (execution function).
2. **Compile:** During this stage, the graph looks at each pass inputs and outputs and figures out the optimal execution order using topology sort.
3. **Execution:** For each frame, the graph loops through its passes and executes one at a time, in the end, producing a single result image.

## What about the main rendering loop?

The main rendering loop looks very simple with this render graph implementation:
1. **Acquiring the swapchain image from the swapchain:** Once an image is done presenting, the swapchain gives it back to you. What you do with it depends on how you configure the graph.
2. **Graph execution:** The graph executes each pass according to the execution order, each pass takes in one or more resources and write to exactly one resource. At the end of the graph execution loop, one image is produced.
3. **Acquiring and presenting the result:** Once the output image of the graph is done being drawn onto, the render loop grabs the image, copying its content and pastes it into the swapchain image acquired earlier for presenting.

## Roadmap

While this is a WIP project as I'm still learning the basic fundamentals of graphics programming, I charted down a few things that can be improved/implemented in the near future:
1. **Multi-pass rendering:** While the render graph looks promising, introducing more passes to the graph might come with synchronization issue.
2. **Better lighting implementation:** While blinn-phong provides somewhat believable result, having to implement PBR is inevitable as it's the standard for this era.
3. **Shadow and reflection:** Raytraced shadow and reflection which can be achieved through hardware-accelerated raytracing or compute-based.
4. **Post-processing:** Once multi-pass rendering and PBR are implemented, post-processing pass provides a final touch to the image. There are a few things that can be done: TAA, Bloom, Screen-space shading technique, etc.

## How to build
I come from the world of web development so I tried to make this project as "work-out-of-the-box" as possible. Here's a few things you need:
1. Ninja build
2. Vcpkg
3. Lunar Vulkan SDK

Check the CMakePresets and vcpkg's, Lunar Vulkan SDK's document to wire them up in the CMakeLists file. 
These scripts are provided to simplify the initialization and build process:
1. `init.sh` for project initialization.
2. `build_and_run.sh` to build the project and run (will call to init.sh if not initialized).

## Final thoughts
Graphics programming is hard and self-taught graphics programming is even harder so any helps and criticisms are welcome and appreciated.
