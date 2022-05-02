<!--
SPDX-FileCopyrightText: none

SPDX-License-Identifier: CC0-1.0
-->

# XML Wallpaper Plugin

An image/slideshow wallpaper plugin for KDE Plasma that supports GNOME XML wallpaper format.


## Features

- Support Time of Day XML wallpaper
- Support Light/Dark wallpapers
- Compatible with KPackage and local wallpapers

## Installation

### openSUSE

For openSUSE Tumbleweed

```shell
zypper addrepo https://copr.fedorainfracloud.org/coprs/fusionfuture/plasma-wallpapers-xml/repo/opensuse-tumbleweed/fusionfuture-plasma-wallpapers-xml-opensuse-tumbleweed.repo
zypper refresh
zypper install plasma-wallpapers-xml
```


### Fedora/CentOS 9+

```shell
dnf copr enable fusionfuture/plasma-wallpapers-xml
dnf install plasma-wallpapers-xml
```

### Arch / Manjaro Linux

```shell
yay -S plasma5-wallpapers-xml
```

Or you can use the prebuilt package:

1. Edit /etc/pacman.conf and add the following lines to the end of the file:

```
[home_fusionfuture_plasma-wallpapers_Arch]
SigLevel = Optional TrustAll
Server = https://download.opensuse.org/repositories/home:/fusionfuture:/plasma-wallpapers/Arch/$arch
```

2. Import the PGP key and install the package

```shell
wget -O /tmp/home_fusionfuture_plasma-wallpapers_Arch.key "https://download.opensuse.org/repositories/home:/fusionfuture:/plasma-wallpapers/Arch/x86_64/home_fusionfuture_plasma-wallpapers_Arch.key"
pacman-key --add /tmp/home_fusionfuture_plasma-wallpapers_Arch.key
pacman -Sy
pacman -S plasma5-wallpapers-xml
```
