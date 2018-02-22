cd os161/

#Root configuration
./configure --ostree=$HOME/Documents/ECE344/root
make

#Kernel configuration
cd kern/conf
./config ASST1
cd ..
cd compile/ASST1
make depend

make
make install

cd ../../..

#Running Kernel

