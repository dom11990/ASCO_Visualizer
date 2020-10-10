# ASCO_Visualizer

Build dependencies:
  -Qt5 (https://www.qt.io/)
  -Qwt (https://qwt.sourceforge.io/)
  
1. Go into main directory
2. mkdir build
3. cd build
4. cmake ../
5. make -j


Usage:

Make sure the Qucs dir is pointing to the correct folder. It should be the folder that contains the ASCO files that are created during simulation / optimization.

When a simulation starts the visualizer will automatically detect it and begin plotting the variables and measurements as well as their limits. Results from the simulation can be selected on the right to be plotted. The initial design was for micorstrip filter optimization so only testing for S-Parameter optimization was done. It is possible it works with other simulations also, out of the box.

![Main UI](https://github.com/dom11990/ASCO_Visualizer/blob/master/doc/asco_visualizer.png?raw=true)
