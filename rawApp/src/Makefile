TOP=../..

include $(TOP)/configure/CONFIG

DBD += NDPluginRaw.dbd

INC += NDFileRaw.h

LIBRARY_IOC = NDPluginRaw

NDPluginRAW_SRCS  += NDFileRaw.cpp

USR_INCLUDES += -I $(ADCORE)/ADApp/ADSrc
USR_INCLUDES += -I $(ADCORE)/ADApp/

USR_INCLUDES += -I $(ADSUPPORT)/lib/


include $(ADCORE)/ADApp/commonLibraryMakefile

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


