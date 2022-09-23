#!/usr/bin/env bash

prepare_debian()
{
	installPackages+=(build-essential)
	installPackages+=(file)
	installPackages+=(apt-utils)
	installPackages+=(build-essential)
	installPackages+=(build-essential)
	installPackages+=(git)

	installPackages+=(cmake)
	installPackages+=(ninja-build)

	# Conan:
	installPackages+=(libegl-dev)
	installPackages+=(libfontenc-dev)
	installPackages+=(libva-dev)
	installPackages+=(libvdpau-dev)
	installPackages+=(libx11-xcb-dev)
	installPackages+=(libxaw7-dev)
	installPackages+=(libxcb-dri3-dev)
	installPackages+=(libxcb-icccm4-dev)
	installPackages+=(libxcb-image0-dev)
	installPackages+=(libxcb-keysyms1-dev)
	installPackages+=(libxcb-randr0-dev)
	installPackages+=(libxcb-render-util0-dev)
	installPackages+=(libxcb-shape0-dev)
	installPackages+=(libxcb-sync-dev)
	installPackages+=(libxcb-xfixes0-dev)
	installPackages+=(libxcb-xinerama0-dev)
	installPackages+=(libxcb-xkb-dev)
	installPackages+=(libxcomposite-dev)
	installPackages+=(libxcursor-dev)
	installPackages+=(libxdamage-dev)
	installPackages+=(libxfixes-dev)
	installPackages+=(libxi-dev)
	installPackages+=(libxinerama-dev)
	installPackages+=(libxmuu-dev)
	installPackages+=(libxrandr-dev)
	installPackages+=(libxres-dev)
	installPackages+=(libxss-dev)
	installPackages+=(libxtst-dev)
	installPackages+=(libxvmc-dev)
	installPackages+=(libxxf86vm-dev)
	installPackages+=(libxcb-util-dev)
	installPackages+=(libxcb-util0-dev)
	installPackages+=(libglu1-mesa-dev)

	sudo DEBIAN_FRONTEND=noninteractive apt-get -q -y install "${installPackages[@]}"
}

get_os_distributive()
{
	ID_LIKE_STR=$(grep ID_LIKE /etc/os-release)
	eval ${ID_LIKE_STR}
	for ID in $ID_LIKE
	do
		if [ "$ID" = "debian" ]; then
			echo $ID
			return
		fi
	done
}

DISTRIBUTION=$(get_os_distributive)

case "$DISTRIBUTION" in
	debian|ubuntu )
		prepare_debian
		;;
	* )
		echo "The OS distribution is not supported"
		;;
esac
