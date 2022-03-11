# **Author: Sean Chung**
# **Robot-Simulator** #
Display _robots_ that are created through UDP request-response cycles and threads.
- Made-by: Sean Chung ([seanchung2001](http://www.github.com/seanchung2001))

## Minimum Requirements ##
- OS that allows compilation of C programs
- Compiler

## Tools ##
- GDB debugger
- Valgrind
- Gcc compiler
- Gedit

## Features ##
- Robots are spawned in an environment server, with random mobility. Upon contact with another _robot_, the _robot_ will: (1) turn 15 degrees in a direction to find open space. (2) Continue down that path until contact is made.
- A maximum of 20 _robots_ are allowed to spawn in the enviornment.
- Informs us of the coordinates where the robot spawns in the terminal
- If the spawn space for a robot is in contact with another robot, the spawning space for the robot will change.
- Gracefully closes the environment upon shutdown protocol.

## Compiling Instructions ##
- **(1)**: Download all files included in this repository.
- **(2)**: Open the terminal and _cd_ into the proper directory.
- **(3)**: Type '_make_' into the terminal and wait for the compiler.
- **(4)**: We can now run the executable by doing '_./environmentServer&_' in the terminal. An empty window should appear on your screen.
- **(5)**: At this point, it will have started the _environmentServer_ in the background. Go to the terminal and create a robot by typing: "_./robotClient&_" in the terminal.
- **(6)**: We can now add up to 20 robots by repeating step 5 individually.
- **(7)**: Shutdown the server by typing '_./stop_' in the terminal. Press enter once.
- **PS**: Typing '_make clean_' in the terminal will remove all object files created through this program.
