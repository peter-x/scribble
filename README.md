Scribble
========

Application intended as an alternative to the default
scribble application on the Onyx Boox M92.

### Design Goals

The application should behave similar to a paper notebook.

 * notebook consists of pages named by creation date and time
 * pages can be turned easily
 * one can use pen (black, different sizes) and eraser
 * pen, eraser and thinkness can be changed easily


### Implementation Ideas

 * at the top of the page is a toolbar containing buttons to
    * change pen thickness
    * switch pen and eraser
    * turn pages
 * file format used is PNG or SVG (or xournal?)


## Compilation

#### Compiling for x86 in QtCreator:

Make sure that you installed the SDK to `/opt/onyx`, and compiled the SDK
libraries from `https://github.com/onyx-intl/booxsdk/` and installed them to
`/usr/local/lib`. Because of some reasons, the x86 version also needs Qt DBUS,
so you have to use `qmake scribble.pro QT+=dbus` and then `make` to compile. You
can also use the script `build_x86.sh`.

#### Compiling for arm:

Make sure that you installed the SDK to `/opt/onyx`. Then simply use the script
`build_arm.sh`.
