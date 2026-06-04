# LIONS

> Laith's Idea Of Networked Services

LIONS is my distributed system that automates my life, powers my development, and keeps me curious.

It is built on top of it's own custom protocol that I designed myself called [LIONS Middleware Protocol](https://github.com/LOTaher/lmp). This lets me have full control over my system at the packet level.

LIONS runs on all my hardware. My hardware shares a naming convention: Counter-Strike maps. You will see these hostnames referenced in my code:

- **mirage** is my personal laptop running MacOS Sequoia 15.6.1. It is an M1 13-inch Macbook Air. It is where I do all my development.
- **nuke** is a custom built PC running Debian Stable. It has an AMD Ryzen 5 CPU, 1 TB SSD, two 4 TB HDDS, and a NVIDIA GTX 1060 GPU.
- **inferno** is a Raspberry Pi 4 running a headless Debian Lite.
- **ancient** is a Dell Optiplex 7040 (SFF) running FreeBSD 14.4-RELEASE. It has two 500 GB HDDs.
  - The jails on my FreeBSD host are named after the different forms of rotom and all have a different use case:
    - **wash** is running most LIONS services
    - **heat** is hosting my personal website and other public facing services
    - **mow** is running my homelab's file syncing system using syncthing
- **anubis** is a custom built PC duel booting Debian Stable and Windows. It has an AMD Ryzen 7 CPU, 1 TB SSD, and a AMD Radeon RX 6600 GPU.

LIONS is currently composed of the following services:

- **admiral**: central message broker running 24/7 that allows different LIONS supported services to talk to each other using LMP.
- **archimedes**: automated ssh port forwarding tool to allow me to access my self-hosted services on any device I need.
- **s2**: scheduling service that sends LMP packets to different services through admiral.
- **gibson**: http bridge for LMP
- **laitt**: mqtt bridge for LMP
- **lightctl**: cli tool to control the lights in my room

Legacy versions of services can be found in the `legacy/` directory. These versions do not work with the current library implementation. The current `lib/c/liblmp` library is going through a rewrite and is not functional at the moment.

This project is not currently structured or documented for public use. Though, it is published for transparency and source availability under the terms of the GPL.
