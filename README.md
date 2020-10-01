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

Make sure the Qucs dir is pointing to the correct folder. It should be the folder that contains the ASCO files that are created during simulation / optimization
Click start, all variables should get plotted. Results from the simulation can be selected on the right to be plotted.
