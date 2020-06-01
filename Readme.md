## Summary
the Project 2 code, pictures,video and Execute file.

## Programming Assignment 2
## Rollercoaster
## Name: Hong Zhang
## Email: hmz5180@psu.edu
## Date: 10/21/2019
## Course Information: Computer Graphic
## Instructor: Prof. Yanxi Liu
## Programming Language: C++

## Visualization
![GIF](https://github.com/HongZH2/Graphics_P2/blob/master/Project_2_Hong_Zhang.gif)

## Description
(1) Interaction
Due to the final submission, I have already achieve all the basic  required functions to make an aircraft to move along the track and change the camer view by interacting with Keyboard Inputs. When It starts, the camera is bounded to the up-backward some certain point of the aircraft model. And it will follow the movement of aircraft. The aircraft will move along the track endlessly.

I have made my comments on these lines I added.

### Line Number
- Project2.cpp
line 602-618: set two models. There are  three camera models. one is the camera is bounded to the aircraft. the second model  is that the camera stand still and stare at the aircraft. the third model is the camera is free to move.
line 435 -450 set the camera orientation

Press 'R': tranfer between model one and model two.
Press 'T': tranfer between model two and model three.

(3)Track
I create a track by modify the files in the spine_part directory. And fill up the track.hpp. a column stand on the ground for each controlpoints. and draw a track. 
### Line Number
track.hpp
1) interpolate()
2) get_point()
3) estimated_point() : this function is given a distance s to compute the current u value by iterations that make the aircraft move more smoother.
line 111 - 130
4) get _Orientation(): a function to set the current and next Orientation.
5) makeRailStandPart() : a function to create the standing columns on the ground.
line 319-342
the rest functions are same as the intended version.

Project2.cpp
line 387-426 compute the velocity of object and height.

(4)Others
Load a aircraft mode instead of a cart.


# Notices
## 1. the top and bottom of skybox is wrong, sorry, have no time to adjust.
## 2. I don't fill up the void ProcessTrackMovement(float deltaTime, Track &track)  in the camera.hpp. Because I write a function called estimated_points() in the track.hpp instead.

# If the exec file can't run , Please Let me know!! 
