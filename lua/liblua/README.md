# **README for liblua** #
***


## **What's liblua?** ##
* It's a library or framework with C/C++ and lua
* It's just for funny


## **Building liblua** ##
> ### **Windows** ###
    \> cd .\liblua\src
    \> nmake -f makefile.mk
    \> nmake -f makefile.mk install 

> ### **Linux** ###
    $ cd ./liblua/src 
    $ make 
    $ make install 


## **Use liblua** ##
> ### **Windows** ###
    package.cpath = 'xxx/?.dll'
    local l = require 'liblua'
    ...
> ### **Linux** ###
    package.cpath = 'xxx/?.so'
    local l = require 'liblua'
    ...
> ### **Samples** ###
    see directory ./test/
