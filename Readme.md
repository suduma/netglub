# netglub

netglub is an open source OSINT framework.

### Authors

This repository is a fork of
[www.netglub.org](http://www.netglub.org) from the original authors at [Diateam](http://www.diateam.net/)

### Changes

some small changes I have done so far:

+ I have updated the source and build files to be compatible with modern versions of graphviz
+ build files have been enhanced for other linux build environments (e.g. Debian testing)

I just leave this here for others until I can test the whole framework. For now
I was able to build all three components with Debian testing.

### Compile from source

Install dependencies (Debian testing):
```
sudo aptitude install build-essential zlib1g-dev qt4-qmake qt4-dev-tools \
qt4-default graphviz-dev libz3-dev
```
Then compile the three software components:

+ qng (GUI)
```
cd qng && qmake && make
```

+ master
```
cd master && qmake && make
```

+ slave
```
cd slave && qmake && make
```

### License

The original authors distributed netglub as GPLv3. The same license applies here.
