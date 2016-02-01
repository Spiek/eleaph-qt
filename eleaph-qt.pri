#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# enable C++11
CONFIG += c++11

# Keep Alive System
DEFINES	+= ELEAPH_KEEP_ALIVE_ACTIVE
DEFINES	+= ELEAPH_KEEP_ALIVE_PINGINTERVAL=1000
DEFINES	+= ELEAPH_KEEP_ALIVE_TIMEOUT=30000

# Sources
SOURCES +=  $$PWD"/src/ieleaph.cpp" \
            $$PWD"/src/eleaphrpc.cpp" \
            $$PWD"/src/eleaphrpc_packetmetaevent.cpp" \
            $$PWD"/src/eleaphrpc_asyncpacketwaiter.cpp" \
            $$PWD"/src/eleaphrpc_packethandler.cpp" \
    		$$PWD"/src/ssltcpserver.cpp"

# Headers
HEADERS +=  $$PWD"/src/ieleaph.h" \
            $$PWD"/src/eleaphrpc.h" \
            $$PWD"/src/eleaphrpc_packetmetaevent.h" \
            $$PWD"/src/eleaphrpc_asyncpacketwaiter.h" \
            $$PWD"/src/eleaphrpc_packet.h" \
            $$PWD"/src/eleaphrpc_packethandler.h" \
    		$$PWD"/src/ssltcpserver.h"

# include path
INCLUDEPATH += $$PWD"/include/"
