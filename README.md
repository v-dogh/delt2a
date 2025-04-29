## Purpose

Del2a is meant to provide a platform-independent extensible framework for writing TUIs
(and possibly UIs in the future). Currently the project is in early stages so there is still much to add, improve and fix,
but so far the core features include:
## Features
- components system (for abstraction of platform dependent features)
- multithreading capabilities and thread safety
- theming through dependencies and dynamic properties
- extensible tree object system
- standard elements and popups which include:
  - text + input
  - checkboxes, switches, sliders, buttons, dropdowns
  - boxes with support for automatic layout + draggable + scrollable
  - graphs
  - ASCII models
- reflection (generic access to properties through interfaces) and type safety
- macro based DSL or C++ templates for building UIs and interacting with the framework
## Roadmap
- optimization (especially the naive implementations, e.g. utils/function.hpp)
- full UTF8 support
- memory optimization (better compression, caching etc.)
- images/videos/raster surfaces
- better rendering API
- more graph types
- implementations for standard components for more platforms

## Usage

Refer to the example files in the "examples" folder.
Make sure to check them out in order since they are sequential.
Docs are a work in progress.
You can let me know if you are going to use it for some cool stuff
