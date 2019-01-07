Flappy8659-Run-On-C-SKY-gx6605s-dev-board
===
a simple flappybird-like game run on C-SKY gx6605s dev board

<img src="assets/游玩方式.png" width="50%" height="50%">

# Catalog
* [Hardware Requirement](#hardware-requirement)
* [Introduction](#introduction)
* [Compile and Run](#compile-and-run)

## Hardware Requirement

* [C-SKY gx6605s dev board](https://c-sky.github.io/docs/gx6605s.html "C-SKY gx6605s dev board")
* Another device to run an UART console (like [PuTTY](https://putty.org "PuTTY") etc.)

## Introduction

Main menu.

<img src="assets/首页截图（离线版本）.png" width="50%" height="50%">

Press the button '8' or '6' or '5' or '9' on dev board to start the game.

<img src="assets/游戏页面.png" width="50%" height="50%">

Press the button to let correspond bird('8','6','5','9') fly higher. 
Dodge the pipes('#').

<img src="assets/结束页面.png" width="50%" height="50%">

If the bird crash something, it will die.
(This picture was taken in Online version. The code in this repository is Offline version.)



## Compile and Run
* Compile (on Ubuntu 16.04)
```Bash
csky-linux-gcc flappyBirdByCSY.c -lpthread -o out flappy8659
```
Then mv the output file to board.
* Run (on dev board)
```Bash
./flappy8659
```
