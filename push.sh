sh pull.sh

cd $HOME/Documents/ECE344/

# SVN
cd os161
svn add . --force
sh svn-ignore.sh
svn status
svn commit -m "Completed stoplight and modified lock implementation"
cd ..

# GIT
git add .
git commit -m "Completed stoplight and modified lock implementation"
git config --global credential.helper 'cache --timeout=10000000'
git push origin master