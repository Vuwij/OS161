cd os161/

#Root configuration
./configure --ostree=$HOME/Documents/ECE344/root
make clean
#cd ..

#Kernel configuration
cd kern/conf
./config ASST3
cd ..
cd compile/ASST3
make depend

make
make install

cd ../../..

#Running Kernel

