if [ -z "$1" ]; then
    echo "Usage: $0 [texfile]"
    exit
fi

FILE=`basename $1`
PDF=`echo $FILE | sed 's/\.[a-z0-9]*$//g'`.pdf

scp $1 haldean.org:/tmp/$FILE
ssh haldean.org "pdflatex /tmp/$FILE"
scp haldean.org:/tmp/$PDF .