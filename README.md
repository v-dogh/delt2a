## Purpose

Del2a is meant to provide a platform-independent extensible framework for writing TUIs
(and possibly UIs in the future). Currently the project is in early stages so there is still much to add,
but so far the core features include:
## Features
-components system (for abstraction of platform dependent features)
-multithreading capabilities and thread safety
-theming through dependencies
-extensible tree object system
-standard elements and popups which include:
  -text + input
  -checkboxes, switches, sliders, buttons, dropdowns
  -boxes with support for automatic layout + draggable + scrollable
  -graphs
  -ASCII models
-reflection
-macro based DSL for building UIs and interacting with the framework
## Roadmap
-full UTF8 support
-memory optimization (compression, caching etc.)
-images/videos/raster surfaces
-ASCII surface element (ASCII rendering API)
-more graph types
-implementations for standard components for more platforms

## Usage

Refer to the example files in the "examples" folder.
Make sure to check them out in order since they are sequential.
Docs are a work in progress.

## Attribution

If you use this software, please provide clear and visible credit 
to the project in any user-facing or developer-facing documentation, if possible.

Example: "Built using delt2a"
