#!/bin/bash

DIRROOT="dgnuutils-${1}"
DIRBIN="${DIRROOT}/bin"
DIRSRC="${DIRROOT}/src"

mkdir -p ${DIRSRC} ${DIRBIN}

cp ./src/*.{c,h} ${DIRSRC}
cp AUTHORS COPYING Makefile README TODO ${DIRROOT}/

tar cf "${DIRROOT}.tar" "${DIRROOT}"
gzip "${DIRROOT}.tar"
tar cf "${DIRROOT}.tar" "${DIRROOT}"
bzip2 "${DIRROOT}.tar"

rm -rf ${DIRROOT}
