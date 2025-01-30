 xmake -j -f Makefile_xmos
 xrun --xscope --args bin/doom.xe -iwad data/boomlump.wad || xrun --dump-state bin/doom.xe 