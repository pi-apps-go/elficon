# elfres/elficon - Manage application resources in ELF binaries

This is a modern version of elfres which uses GTK 3 and improves on features compared to original elfres/elficon which hasn't been maintained since 2011.
See the INSTALL file for general installation instructions.

> [!NOTE]
> This application is a technology demonstration, at this point please DO NOT
> consider this implementation to be a specification for how ELF icons will be
> supported by desktop environments.  With that said, this application and the
> associated "libr" resource library provide a solid mechanism for managing
> application resources that you are free to use in your own applications.

See https://wiki.ubuntu.com/ELFIconSpec for a detailed proposal.

## How to compile and package the .deb?
1. Install dependencies:
```bash
sudo apt-get install build-essential autotools-dev libtool pkg-config doxygen
sudo apt-get install libgtk-3-dev libglib2.0-dev libgdk-pixbuf2.0-dev librsvg2-dev gettext
```
2. Clone lelficon:
```bash
git clone https://github.com/pi-apps-go/elficon
```
3. Compile the build:
```bash
cd elficon
dpkg-buildpackage -us -uc
```
