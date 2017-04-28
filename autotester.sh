pkill sys161

cd root

# Assignment 1
(sleep 0.1; cat ../test/ASS01;) | ../tools/pv/pv -q -L 3 | /cad2/ece344s/cs161/bin/sys161 kernel