cd os161/
make clean

svn rm -m "removing assignment 0 tag" $ECE344_SVN/tags/asst0-end
svn copy -m "ending assignment 0" $ECE344_SVN/trunk $ECE344_SVN/tags/asst0-end

sh ../push.sh

#setenv ECE344_SVN svn+ssh://ug250.eecg.utoronto.ca/srv/ece344s/os-042/svn
os161-tester -m 1