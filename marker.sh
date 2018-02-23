rm -rf marker/
mkdir marker/
sh push.sh
cd marker/
svn rm -m "removing assignment 0 tag" $ECE344_SVN/tags/asst0-end
svn copy -m "ending assignment 0" $ECE344_SVN/trunk $ECE344_SVN/tags/asst0-end
os161-tester -m 1
