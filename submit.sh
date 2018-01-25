set path=( /cad2/ece344s/cs161/bin $path)

cd os161/
make clean

setenv ECE344_SVN svn+ssh://ug250.eecg.utoronto.ca/srv/ece344s/os-042/svn
#svn copy -m "ending assignment 0" $ECE344_SVN/trunk $ECE344_SVN/tags/asst0-end