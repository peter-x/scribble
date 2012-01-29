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

Make sure that you compiled the SDK libraries
from `https://github.com/onyx-intl/booxsdk/`
and installed them to `/usr/local/lib`. Then just
use `qmake` to compile or use QtCreator.

#### Compiling for arm:

TODO

