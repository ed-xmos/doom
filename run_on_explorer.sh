# xmake -j -f Makefile_xmos && xrun --xscope --args bin/doom.xe -iwad data/Doom1.WAD -file data/boomlump.wad -width 320 -height 200 -noload -playdemo 
cmake -G "Unix Makefiles" -B build
make -j -C build

xrun --xscope  bin/doom.xe
 #|| xrun --dump-state bin/doom.xe \