# Force Directed Graph Layout in Hyperbolic Field

Computer Graphics homework, programmed in C++ with OpenGL.

## Task specification

Create a program that aesthetically displays a random graph and allows the user to magnify any part of it, while the rest is always visible. The graph consists of 50 nodes with a saturation of 5% (5% of the possible edges are real edges). For an aesthetic arrangement, the location of the nodes must be determined by heuristics and a force-driven graphing algorithm according to the rules of the hyperbolic plane.

To focus, the graph must be arranged on the hyperbolic plane and projected onto the screen using the Beltrami-Klein method. Focusing is done by shifting the graph on the hyperbolic plane so that the part of interest is at the bottom of the hyperboloid. The visual projection of the offset is the difference between pressing the right mouse button and the current position of the mouse movement when pressed.

Each node is a circle in the hyperbolic plane that has a texture that identifies the node.

## Solution

YouTube video showcasing the solution with different graph setups: https://youtu.be/ZYQTHAPdLRk

<img width="676" alt="Screenshot 2022-03-27 at 22 15 31" src="https://user-images.githubusercontent.com/27449756/160299175-494c084c-0437-45eb-99d9-51fb7ace38c6.png">
<img width="676" alt="Screenshot 2022-03-27 at 22 15 41" src="https://user-images.githubusercontent.com/27449756/160299180-f9be6896-80ba-4ed0-bef6-4595e7027f48.png">


