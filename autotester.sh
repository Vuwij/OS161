pkill sys161
#
#cd os161/kern/compile/ASST1
#make
#make install
#cd ../../../..
cd root

# Assignment 1
(sleep 0.1; cat ../test/ASS03;) | ../tools/pv/pv -q -L 6 | /cad2/ece344s/cs161/bin/sys161 kernel