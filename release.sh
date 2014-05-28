#!/bin/bash

. ./version.sh

if [ -d .git ] ; then
	VERSION="${VERSION}-`git rev-list HEAD | wc -l | sed -e 's/ //g'`-`git log HEAD -n1 --format=\"%h\"`"
fi

RELNAME="honeynet-sinkhole-$VERSION"

echo "Creating release $RELNAME.tar.gz"
git archive --prefix=$RELNAME/ --format=tar HEAD | gzip > $RELNAME.tar.gz
echo "Done"

