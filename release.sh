
if [ -z $1 ]
then
    echo "usage: ./release.sh {branch_name}"
    exit 1
fi

git branch "$1"
git push origin $1:$1

