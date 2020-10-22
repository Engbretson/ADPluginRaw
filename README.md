"# NDRawFile_Plugin" 

Tentative AD R3-10 compatible

Used latest Mark Rivers NDFileNull for the current AD template, cut and paste code from legacy Keenan Lang NFFileRaw code

You make modify the ADDetector main make file to be aware of the existing of the plugin.

Add a reference to the plugin in your RELEAE file(s)

ADPLUGINRAW=$(AREA_DETECTOR)/ADPluginRaw then . . .


Example: using SimDtector, edit the Makefile in

\ADSimDetector-R2-10\simDetectorApp\src

add

ifdef ADPLUGINRAW
  $(DBD_NAME)_DBD  += NDPluginRAW.dbd
  PROD_LIBS         += NDPluginRAW
endif


and either in ADCore commonDriverMakefile or in the specific area detector you intend of building you need to 
add code similar to

Example: using SimDetector, edit the makefile in

\ADSimDetector-R2-10\iocs\simDetectorIOC\simDetectorApp\src

ifdef ADPLUGINRAW
  $(DBD_NAME)_DBD  += NDPluginRAW.dbd
  PROD_LIBS         += NDPluginRAW
endif

So that you both link against the library and resolve the reference to initialization routines.

You should then be able to put code similar to this in your st.cmd to start the plugin.



NDFileRawConfigure("Raw1", 20, 0, "$(PORT)", 0, 0, 0)
dbLoadRecords("$(ADPLUGINRAW)/rawApp/Db/NDPluginRaw.template", "P=$(PREFIX), R=raw1:, PORT=Raw1, ADDR=0, TIMEOUT=1, NDARRAY_PORT=$(PORT), NDARRAY_ADDR=0")

 

