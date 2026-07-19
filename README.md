# FIRSTBORN: TUI Game Of Life

C++ Conway's Game Of Life in the terminal!
I tried making this without using AI in any form but I got stuck in a couple spots while debugging, those were the only times I used it.
I also didn't use any libraries like ncurses which is why the UI ~~IS FREAKING HORRIBLE~~ seems uninspired.. in case.. you were.. wondering......

Outdated showcase for when the repo used to be called 'conways_game_of_life' (wow so original)
![showcase](assets/showcase00.gif)


You can load RLE files now! Sorry if its buggy. Edit: They're upside down now.. uh.. I'll try fixing that.
```bash
./build glider.rle
```

## Controls

* `h`,`j`,`k`,`l` or `w`,`a`,`s`,`d` to move camera around
* `space` to activate/deactivate a tile
* `enter` to play/pause
* `q` to quit 

## Building From Source

On linux using `g++`
```bash
git clone https://github.com/evacq8/firstborn.git firstborn
g++ -O3 main.cpp grid.cpp parser.cpp -o build
# run it!
./build
```

## To-do

- [x] Update when terminal window is resized
- [ ] Microsoft Windows support
- [x] Ability load RLE
- [ ] Zoomed out mode using block characters
- [ ] Clean up `parser.cpp` code because its so bad right now 
- [ ] Ability to save RLE
- [x] Infinite-ish tiling using chunks (I think?)
- [ ] Pruning of unused chunks and other optimizations
- [ ] Custom rules (e.g. HighLife) and speed
- [ ] Make RLE load properly instead of upside down
