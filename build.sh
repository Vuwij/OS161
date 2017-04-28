set path=( /cad2/ece344s/cs161/bin $path)

cd os161/

#Root configuration
./configure --ostree=$HOME/Documents/ECE344/root
make

#Kernel configuration
cd kern/conf
./config ASST0
cd ..
cd compile/ASST0
make depend
make
make install

cd ../../..

#Running Kernel

