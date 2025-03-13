# IT 366 2D Game Programming Project (Official Name TBD)
This repository is for my IT 366 game programming project. This project is a bullet hell with a physics based movement system.
This section contains instructions on downloading and playing the game. This game currently only has a linux build

## Installation
Visit the following release page:
https://github.com/Dolphcode/game-prog-2d/releases/tag/midterm-release

Then download the "midterm-build-linux-20250313T025143Z-001.zip" file and extract it anywhere on your disk.
If you are running a Linux system this should work out the box as long as you don't move any of the files in the
extracted folder. Simply run `./gf2d` in the command line or double click to execute

## Movement Controls
While grounded, press A or D to slowly walk left and right
While holding R, press any directional key (W, A, S, D) to dash in that direction
While holding any directional keys (to point in one of 8 directions), hold Y to initiate a boost (release Y to release the boost prematurely).
Press I to launch the grappling hook upwards, dashing with A swings you clockwise while dashing with D swings you counter clockwise (dash control is the same)
While grappled press U to retract the hook
Hold Left Mouse Button to fire your weapon, use mouse to aim it

## Changing Weapons
To change what weapon your character is using, you may execute the binary in the command line with the `-w` or `--weapon` option. This option
takes an argument specifying a def file to load the weapon. A general template for selecting a weapon is:
```
./gf2d -w def/weapons/<INSERT WEAPON HERE>.def
```
You may select from the following weapons to replace `<INSERT WEAPON HERE>` with:
 - `shotgun`
 - `sniper`
 - `gatling`
 - `flamethrower`
 - `rocketlauncher`
 - `moablauncher`
 - `spikeball_launcher`
 - `sword`*
 - `swordfire`*
 - `swordflame`*
* indicates a melee weapon, though `swordfire` and `swordflame` include melee and ranged projectiles
* 
# GF2D
I left this section from the original repo I forked

a collection of utlitity functions designed to facilitate creating 2D games with SDL2
This project is specifically intended to function as an educational tool for my students taking 2D Game Programming.

FOR FULL FEATURES CHECKOUT THE MASTER BRANCH
Students: Don't do that.  You are not ready.... yet.

The main branch is kept deliberately minimal to allow students to build out systems as they are learning.
Other branches have more complete system: Collisions, Windows, Armatures, Entities, Particles...

# Build Process

Before you can build the example code we are providing for you, you will need to obtain the libraries required
by the source code
 - SDL2
 - SDL2_image
 - SDL2_mixer
 - SDL2_ttf
There are additional sub modules that are needed for this project to work as well, but they can be pulled right from within the project.
Performable from the following steps from the root of the cloned git repository within a terminal. 

Make sure you fetch submodules: `git submodule update --init --recursive`
Go into each submodule's src directory and type:
`make`
`make static`

Once each submodule has been made you can go into the base project src folder anre simply type:
`make`

You should now have a `gf2d` binary within the root of your git repository. Executing this will start your game.

# video overviews and tutorials
Overview: https://www.youtube.com/watch?v=nvVQ_n6ycC4

Linux Setup: https://www.youtube.com/watch?v=0Znnv5C4mCo

Windows Setup: https://www.youtube.com/watch?v=zj_egJ4sw3I

