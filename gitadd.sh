git add -A

if [ -z $1 ]
then
    git commit -m "aaaa"
else
    git commit -m "$1"
fi
git push origin master:master
