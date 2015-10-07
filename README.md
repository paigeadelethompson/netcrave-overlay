--- To add this repository ---

* UPDATE Oct 7 2015:
** changing keywords for all ebuilds to "netcrave" since this overlay is all
experimental at the moment.

* UPDATE 9/8/2014:
** This repo is listed in Gentoo's global layman directory, to add it layman -f && layman -a netcrave

* Instructions for consuming

** easy way:
```
layman -f
layman -a netcrave
```

** old way:
*edit the /etc/layman/layman.cfg and add the following line under the overlays section: https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml

example:
```
overlays :http://www.gentoo.org/proj/en/overlays/repositories.xml
	    https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml
```

then run:
```
layman -S
layman -a netcrave
```

* Pull requests are welcome, I mainly use this for my personal projects / entertainment.
