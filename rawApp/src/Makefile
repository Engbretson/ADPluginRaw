TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC support library

#LIBRARY_IOC = NDFileRaw
LIBRARY_IOC = NDPluginRAW

# aliveRecord.h will be created from aliveRecord.dbd
##DBDINC += NDPluginRaw
# Install aliveSupport.dbd into <top>/dbd
DBD += NDPluginRAW.dbd

# Compile and add the code to the support library
LIB_SRCS  += NDFileRaw.cpp

#LIBS += $(EPICS_BASE_IOC_LIBS)

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


