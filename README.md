---To add this repository---

edit the /etc/layman/layman.cfg and add the following line under the overlays section: https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml

example: 

overlays  : http://www.gentoo.org/proj/en/overlays/repositories.xml
	    https://raw.githubusercontent.com/paigeadele/netcrave/master/layman/overlay.xml

then run: 
layman -S 
layman -a netcrave

#TODO mkinitcpio

windowlicker netcrave # mkinitcpio 
==> Starting dry run: 3.17.0-rc3-gentoo+
  -> Running build hook: [base]
==> ERROR: file not found: `/usr/lib/initcpio/busybox'
/usr/lib/initcpio/install/base: line 3: /usr/lib/initcpio/busybox: No such file or directory
==> ERROR: file not found: `/usr/lib/modprobe.d/usb-load-ehci-first.conf'
==> ERROR: Hook 'udev' cannot be found
  -> Running build hook: [autodetect]
  -> Running build hook: [modconf]
  -> Running build hook: [block]
  -> Running build hook: [filesystems]
  -> Running build hook: [keyboard]
  -> Running build hook: [fsck]
==> Generating module dependencies
==> Dry run complete, use -g IMAGE to generate a real image
windowlicker netcrave # 


#TODO
http://sourceforge.net/p/mpd/code/?source=navbar
