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

	installPackages+=(libsdl2-dev)
	installPackages+=(libsdl2-image-dev)
	installPackages+=(libfreetype6-dev)
	installPackages+=(libglew-dev)
	installPackages+=(libglm-dev)
	installPackages+=(libaudio-dev)
	installPackages+=(libswscale-dev)
	installPackages+=(libalure-dev)
	installPackages+=(libopenal-dev)
	installPackages+=(libenet-dev)

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
