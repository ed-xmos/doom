 xmake -j -f Makefile_xmos
 xrun --xscope --args bin/doom.xe -iwad data/boomlump || xrun --dump-state bin/doom.xe 