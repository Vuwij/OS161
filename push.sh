sh pull.sh

cd $HOME/Documents/ECE344/

# SVN
cd os161
sh svn-ignore.sh
svn add . --force
svn status
svn commit -m "Commit"
cd ..

# GIT
git add .
git commit -m "Commit"
git config --global credential.helper 'cache --timeout=10000000'
git push origin master