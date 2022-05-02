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

### Arch Linux

```shell
yay -S plasma5-wallpapers-xml
```

Or you can use the prebuilt package:

1. Edit /etc/pacman.conf and add the following:

```
[home_fusionfuture_plasma-wallpapers_Arch]
Server = https://download.opensuse.org/repositories/home:/fusionfuture:/plasma-wallpapers/Arch/$arch
```

2. Run `pacman -Syu`
3. Run `pacman -S plasma5-wallpapers-xml`
