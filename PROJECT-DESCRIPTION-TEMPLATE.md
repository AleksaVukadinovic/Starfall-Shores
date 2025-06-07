# Starfall Shores

mi22103 - Aleksa Vukadinović
Starfall Shores features a 3D scene with three distinct islands surrounded by animated water. 
The environment includes vegetation (trees, bushes, mushrooms etc.), tents, and a dynamic day-night system. 
As night falls, a campfire automatically lights up, showing animated fire effect with bloom. 

## Controls

### Movement controls
W -> Move forward  
S -> Move backward  
A -> Move left  
D -> Move right  
Q -> Turn left 
E -> Turn right
Mouse movement -> Look around  
LShift -> hold to speed up other actions (movement, turning around etc.)

### Scene controls
N -> Toggle day/night (campfire lights automatically)
P -> Turn mouse on/off
G -> Turn GUI on/off (includes bloom effect settings)
ESC -> Exit the program

## Features

### Fundamental:

[X] Model with lighting
[X] Two types of lighting with customizable colors and movement through GUI or ACTIONS
[?] {ACTION_X} --- AFTER_M_SECONDS---Triggers---> {EVENT_A} ---> AFTER_N_SECONDS---Triggers---> {EVENT_B}

### Group A:

[X] Frame-buffers with post-processing   
[X] Instancing  
[ ] Off-screen Anti-Aliasing  
[ ] Parallax Mapping

### Group B:

[ ] Deferred Shading  
[ ] Point Shadows  
[ ] SSAO
[X] Bloom with the use of HDR

### Engine improvement:

Added blending, instancing, post-processing including bloom with HDR.
Added other minor changes related to project necessities 

## Models:

Links to all models can be found inside resources/models folder for each individual model inside license.txt file

## Textures:

No additional textures (outside of models) have been used

## Skyboxes:

All skyboxes can be found here [Google Drive](https://drive.google.com/drive/folders/130JXuZAqmpzhYGFsPKeJxRDVmApGXKl1)