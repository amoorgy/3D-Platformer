


German International University - Berlin
Media Engineering and Technology
Computer Graphics, Winter 2025
Dr. Turker Ince
Assignment 2
Deadline: Tuesday 18th of November @ 11:59 pm
Individual Assignment
Game Concept:
In this assignment, you will design and develop a 3D modeling collectibles game featuring a
main character and four distinct platforms. Each platform should contain a unique 3D object
and several collectible items that need to be collected before time is up.
When the player collects all the items on a specific platform, the associated 3D object must
perform a unique animation. Each platform’s object should feature a different type of
animation, chosen from the following categories:
●
●
●
●
Rotation
Scaling
Transformation
Color change
The animations should run continuously once activated, and can be toggled on and off only
after the corresponding platform’s collectibles have been gathered. As soon as the collectibles
from the final platform are gathered, the same animation logic applies. However, in addition to
triggering the last platform’s animation, a “Game Win” message must appear on the screen.
The player should still be able to control the game after the win condition is met.
When time is up, the entire scene should be replaced by a “Game Over” scene.
A camera system must also be implemented where the camera should support multiple view
controls, which can be managed using the keyboard, mouse, or a combination of both.
The overall theme of the game is Ancient East Asian Warriors, inspired by elements such as
ninjas, samurais, and traditional Eastern aesthetics. Your design choices for the character,
environment, and objects should reflect this theme.
German International University - Berlin
Media Engineering and Technology
Computer Graphics, Winter 2025
Dr. Turker Ince
Modeling:
Each model in the game must be constructed using OpenGL primitives. The scene should
contain at least 21 distinct objects, including:
●
●
●
●
●
Three bounding walls that the character cannot pass through,
A ground plane that the character can stand on but not levitate from or move through,
A main character,
Four 3D objects (one per platform), and
At least three collectibles per platform (a minimum of twelve collectibles in total).
Each of the four platforms must be visibly distinct from one another in design, color, or
structure.
All models in the scene must be constructed entirely from OpenGL primitives. Importing
pre-built models is not allowed, as this assignment focuses on practicing fundamental 3D
modeling techniques. Each model should appear realistic and be fully colored.
The following minimum modeling requirements apply:
●
Player Character: must be at least 6 primitives, clearly representing a head, torso,
●
●
●
●
●
and limbs.
Walls: each must be at least 1 primitive.
Ground: must be at least 1 primitive.
Collectibles: each collectible must be at least 3 primitives.
Main 3D Objects (on platforms): each should be at least 4 primitives.
Platforms: each should be at least 2 primitives.
Level Design & Game Mechanics:
1. Level Design
The game environment should be composed of a ground plane, bounding walls, and four
visually distinct platforms. The player can move freely across the X and Z axes within this
space but cannot move beyond the walls that define the boundaries of the game world.
Each platform must contain a unique 3D object and a set of collectible items (at least three
per platform). The platforms should be clearly distinguishable from one another through
variations in color, design, or height.
German International University - Berlin
Media Engineering and Technology
Computer Graphics, Winter 2025
Dr. Turker Ince
2. Game Mechanics
The player character can be controlled using either the arrow keys or the WASD keys.
Movement should be limited to the X and Z axes, and the player must not pass through any
physical element in the scene, including walls, platforms, or 3D objects.
As the player moves, they must rotate in the direction of motion (for example, turning to face
left when moving left, right when moving right, and so on).
When the player collides with a collectible item, that collectible should disappear to indicate it
has been collected. Collectibles can be placed at fixed or randomized positions within each
platform.
Once all collectibles on a platform have been gathered, the associated 3D object must begin
its continuous animation. The animation should be unique to that platform (e.g., rotation,
scaling, transformation, or color change) and should be toggleable on or off only after all
collectibles for that platform have been collected. The keyboard keys of your choice may be
used to toggle each animation.
Once the collectibles from the final platform are collected, the same logic applies, however, a
“Game Win” message must also appear on the screen while maintaining full player control.
If the time limit is up before the game is completed, the entire scene should transition to a
“Game Over” scene and the player loses all control.
3. Camera
You are required to move the camera freely through the scene along the three axes in
addition to three different views of the camera:
-
-
-
Top View
Side View
Front View
The camera should be controlled using either the keyboard, the mouse, or a combination of
both.
German International University - Berlin
Media Engineering and Technology
Computer Graphics, Winter 2025
Dr. Turker Ince
Bonus:
●
●
Sound effects:
○
Background music that starts playing when the game starts.
○
A sound effect when collecting objects from the scene.
○
A different sound effect when winning the game.
○
A different sound effect when losing the game.
Exceptional Graphics:
○
Complex 3D models (very detailed models). The minimum number of complex
models is four different models of at least 10 primitives each.
Note: The bonus is acquired by EITHER having all sound effects OR exceptional graphics.
Submission:
●
●
●
●
●
●
The assignment should be implemented in OpenGL
This is an INDIVIDUAL assignment.
This assignment is worth 7.5%
Deadline for the assignment: Tuesday 18th of November @ 11:59 pm.
Cheating cases will lead to a ZERO, this includes:
○ Copying the code from the Internet or github repo or a colleague.
○ You are expected to understand any code generated by ChatGPT.
Submission guidelines:
○ You should name your .cpp file in the following format PXX
_
YYYY. XX represents your
tutorial group, YYYY represents your ID. Only submit this .cpp file.
○ Do not submit the .sln file of your assignment, to avoid getting a zero.
○ Submission form: https://forms.gle/vjdSmUStKFuAmigy5