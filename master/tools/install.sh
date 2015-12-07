#!/bin/bash

set -e # stop at the first error

function tryToInstall
{
  srcFile=$1
  dstDir=$2
  dstFile=$dstDir/$(basename $srcFile)

  VAL="a"
  if [[ -f $dstFile ]]
  then
    while [[ $VAL != "y" && $VAL != "Y" && $VAL != "n" && $VAL != "N" && $VAL != "" ]]
    do
      echo -n "$dstFile existe déjà. Voulez-vous l'écraser ? [y/N] "
      read VAL
      #echo "val=[$VAL]"
    done
  else
    VAL="y"
  fi

  if [[ $VAL == "y" || $VAL == "Y" ]]
  then
    mkdir -p $dstDir
    cmd="cp $srcFile $dstFile"
    echo $cmd
    $cmd
  fi
}

APP_NAME=master

tryToInstall $APP_NAME.conf /etc/netglub

SHARE_DIR=/usr/local/share/netglub/$APP_NAME
mkdir -p $SHARE_DIR

cd $(dirname $0)

cp netglub-runLoopDaemon /usr/local/bin/
cp -r tls_credentials /etc/netglub/master_creds
cp -r ../data/* $SHARE_DIR/

cp netglub-$APP_NAME /etc/init.d/

for i in 2 3 4 5
do
  LINK=/etc/rc$i.d/S70netglub-$APP_NAME
  if [[ -L $LINK ]]
  then
    rm $LINK
  fi
  ln -s ../init.d/netglub-$APP_NAME $LINK
done

cp ../$APP_NAME /usr/local/bin/netglub-$APP_NAME

echo "Installation réussie"
