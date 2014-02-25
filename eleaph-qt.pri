#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# enable C++11
CONFIG += c++11

# Sources
SOURCES +=  $$PWD"/src/ieleaph.cpp" \
            $$PWD"/src/eleaphprotorpc.cpp" \
            $$PWD"/src/eleaphrpc_packetmetaevent.cpp" \
            $$PWD"/src/eleaphrpc_asyncpacketwaiter.cpp"

# Headers
HEADERS +=  $$PWD"/src/ieleaph.h" \
            $$PWD"/src/eleaphprotorpc.h" \
            $$PWD"/src/eleaphrpc_packetmetaevent.h" \
            $$PWD"/src/eleaphrpc_asyncpacketwaiter.h" \
            $$PWD"/src/eleaphrpc_packet.h"

# include path
INCLUDEPATH += "include/"
