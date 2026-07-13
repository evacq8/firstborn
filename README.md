# Conways Game Of Life (In The ~~Flesh~~ Terminal)

C++ Conway's Game Of Life in the terminal!
I tried making this without using AI in any form but I got stuck in a couple spots while debugging, those were the only times I used it.
I also didn't use any libraries like ncurses which is why the UI ~~IS FREAKING HORRIBLE~~ seems uninspired.. in case.. you were.. wondering......

![showcase](assets/showcase00.gif)

You can load RLE files now! Sorry if its buggy.
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
# made destination more unique in-case you already had 'conways_game_of_life'
git clone https://github.com/evacq8/conways_game_of_life.git evacs_conways_game_of_life
g++ main.cpp parser.cpp -o build
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
- [ ] Optimizing (it's really REALLY slow for large maps)
- [ ] Infinite Tiling??
