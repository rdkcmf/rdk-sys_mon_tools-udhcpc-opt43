##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#######################################
### Makefile to build udhcpc-opt43 ####
#######################################
CROSS_COMPILE=mipsel-linux

CC=$(CROSS_COMPILE)-gcc

CFLAGS= -I. -DETC_UDHCPC

LDFLAGS= 

TARGETS=udhcpc_opt43

all: $(TARGETS)

udhcpc_opt43: udhcpc_opt43.o 
	$(CC) -v $(CFLAGS) $(LDFLAGS)  udhcpc_opt43.o  -o udhcpc_opt43 
clean:
	rm -f udhcpc_opt43.o udhcpc_opt43

