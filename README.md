---To add this repository---

edit the /etc/layman/layman.cfg and add the following line under the overlays section: https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml

example: 

overlays  : http://www.gentoo.org/proj/en/overlays/repositories.xml
	    https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml

then run: 
layman -S 
layman -a netcrave
