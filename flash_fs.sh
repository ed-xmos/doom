cd ../lib_filesystem/lib_filesystem/disk_image_creators/fat && make && cd -
../lib_filesystem/lib_filesystem/disk_image_creators/fat/fat_image_creator bin/ffs_image.img data/Doom1.WAD data/boomlump.wad
xflash --boot-partition-size 524288 --data bin/ffs_image.img --factory bin/doom.xe