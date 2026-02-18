# Conway-s-Game-of-Life

&emsp; This project implements John Conway's Game of Life. <br/>
&emsp; The initial configuration is a square, but the user can add random rectangles of active cells. <br/>
&emsp; The implementation is sparse (we only store in 2 C++ unordered sets where are the current active cells and the potentially active cells for the next epoch, respectively). <br/>
&emsp; This sparse approach of storing cells (active or potentially active) allows us to have an infinite map (or until the 8 bytes double coordinates and 8 bytes long long coordinates overflow). <br/>

<br/>
<br/>
<br/>

<p align = "center">
  <img width="505" height="425" src="https://github.com/Razvan48/Game-of-Life-Cellular-Automata/blob/main/Demo/Demo-1.gif">
  <img width="505" height="425" src="https://github.com/Razvan48/Game-of-Life-Cellular-Automata/blob/main/Demo/Demo-2.gif">
</p>

<br/>
<br/>
<br/>

**Controls:** <br/>
- WSAD for moving around <br/>
- QE for zooming out / zooming in <br/>
- Z for spawning a random rectangle of randomly chosen active cells <br/>
- Esc for closing the application <br/>

<br/>
<br/>
<br/>

