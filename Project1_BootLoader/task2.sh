make clean
make dirs
make elf
cp createimage -d build
cd build && ./createimage --extended bootblock main
cd ..
make run