qemu-system-x86_64 -L . -m 64 -fda Disk.img -hda HDD.img -boot a -localtime -M pc -serial tcp:127.0.0.1:44444,server,nowait
