# sm64ds-dynamic-level-loading
 
The goal is to bypass the overlay level system to be able to dyanmically load a level from RAM.

Current status : it does not work.

The interesting code is in `/sources/test.cpp`.
New elements have been added to `symbols.x` and `SM64DS_2.h`

## Setup

Open a clean rom with SM64DSe, patch, extract an overlay level (one from 103 to 154), let's take the 103. Add it in the filesystem (Tools ->  Edit Filesystem), add it somewhere. Then, in the file System explorer, you should see it, click on it to see on the bottom its id, you need to take the 0v0ID value, and in the `test.cpp` you need to change the arguments used in the function `LoadFile(0x080a)` by your value.

Compile using NSMBe5, start it. You can see the logs in the console.

## Description 

Currently the code, load a level from the filesystem, change the static addresses to correspond to the one in the file. Then try to replace the overlay but will lead to nothing or crash.
I'm using a trigger (enter in water) to start the loading level workflow.
