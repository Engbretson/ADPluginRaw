/* NDFileRaw.cpp
 * Writes NDArrays to raw files.
 *
 * Keenan Lang
 * October 5th, 2016
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h> 

#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsTime.h>
#include <iocsh.h>
#define epicsAssertAuthor "the EPICS areaDetector collaboration (https://github.com/areaDetector/ADCore/issues)"
#include <epicsAssert.h>

#include <asynDriver.h>

#include <epicsExport.h>
#include "NDFileRaw.h"


static const char *driverName = "NDFileRaw";

int roundUp(int numToRound, int multiple) { 
assert(multiple && ((multiple & (multiple - 1)) == 0)); 
return (numToRound + multiple - 1) & -multiple; 
}


asynStatus NDFileRaw::openFile(const char *fileName, NDFileOpenMode_t openMode, NDArray *pArray)
{
	static const char *functionName = "openFile";
//printf("In Raw File Open . . . \n");
	asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s Filename: %s\n", driverName, functionName, fileName);

	// We don't support reading yet
	if (openMode & NDFileModeRead) 
	{
		setIntegerParam(NDFileCapture, 0);
		setIntegerParam(NDWriteFile, 0);
		return asynError;
	}

	// We don't support opening an existing file for appending yet
	if (openMode & NDFileModeAppend) 
	{
		setIntegerParam(NDFileCapture, 0);
		setIntegerParam(NDWriteFile, 0);
		return asynError;
	}

	// Check if an invalid (<0) number of frames has been configured for capture
	int numCapture;
	getIntegerParam(NDFileNumCapture, &numCapture);
	if (numCapture < 0) 
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
				  "%s::%s Invalid number of frames to capture: %d. Please specify a number >= 0\n",
				  driverName, functionName, numCapture);
		return asynError;
	}
	
	// Check to see if a file is already open and close it
//	if (this->file.is_open())    { this->closeFile(); }
//	 if (pRawFile != NULL) {
	 if (rfile > 0) 
	{
//		fclose(pRawFile);
		close(rfile);
	}

	// Create the new file
//	this->file.fopen(fileName, std::ofstream::binary);
//	pRawFile = fopen(fileName, "wb");
//	rfile = open(fileName, O_CREAT|O_TRUNC|O_WRONLY|O_DIRECT, S_IRWXU);
	rfile = open(fileName, O_CREAT|O_TRUNC|O_WRONLY|O_DIRECT, 0777);
	
//	if (! this->file.is_open())
//	if (pRawFile == NULL) 
	if (rfile == -1) 
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
				  "%s::%s ERROR Failed to create a new output file\n",
				  driverName, functionName);
		return asynError;
	}
	
	// Write 8192 byte header, currently just zeroes.
//	char header[8192] = {0};
//512 byte header int is 4 bytes
/*
struct fullheader {
int header[17] = {0};

header[0] = pArray->dataType;
header[1] = pArray->ndims;
header[2] = pArray->dims[0].size;
header[3] = pArray->dims[1].size;
header[4] = pArray->dims[0].offset;
header[5] = pArray->dims[1].offset;
header[6] = pArray->dims[0].binning;
header[7] = pArray->dims[1].binning;
header[8] = pArray->dims[0].reverse;
header[9] = pArray->dims[1].reverse;
header[10] = 123456;
header[11] = 234567;
header[12] = 345678;
header[13] = 456789;
header[14] = 567890;
header[15] = 678901;
header[16] = pArray->uniqueid;
header[17] = pArray->datasize;
double headerd = pArray->timeStamp;
epicsTimeStamp headere = pArray->epicsTS;
int filler [190]= {0};
};

*/
struct fullheader{
//int header[17];
int datatype;
int ndims;
int dims0size;
int dims1size;
int dims0offset;
int dims1offset;
int dims0binning;
int dims1binning;
int dims0reverse;
int dims1reverse;
int constant1;
int constant2;
int constant3;
int constant4;
int constant5;
int constant6;
int uniqueid;
int datasize;
double timestamp;
epicsTimeStamp epicsts;
int flat;
int dark;
int filler[104];
};

fullheader full_header;
fullheader *ptr;
int flat, dark;

#define STRING_BUFFER_SIZE 2048

    NDAttribute *pAttribute = NULL;

ptr = &full_header;
/*
full_header.header[0] = pArray->dataType;
full_header.header[1] = pArray->ndims;
full_header.header[2] = pArray->dims[0].size;
full_header.header[3] = pArray->dims[1].size;
full_header.header[4] = pArray->dims[0].offset;
full_header.header[5] = pArray->dims[1].offset;
full_header.header[6] = pArray->dims[0].binning;
full_header.header[7] = pArray->dims[1].binning;
full_header.header[8] = pArray->dims[0].reverse;
full_header.header[9] = pArray->dims[1].reverse;
full_header.header[10] = 123456;
full_header.header[11] = 234567;
full_header.header[12] = 345678;
full_header.header[13] = 456789;
full_header.header[14] = 567890;
full_header.header[15] = 678901;
full_header.header[16] = pArray->uniqueId;
full_header.header[17] = pArray->dataSize;
full_header.headerd = pArray->timeStamp;
full_header.headere = pArray->epicsTS;
*/

   this->pFileAttributes->clear();
   this->getAttributes(this->pFileAttributes);
   pArray->pAttributeList->copy(this->pFileAttributes);
 
    full_header.flat = 0;
    full_header.dark = 0;
    
    printf("Number of attributes %d\n",this->pFileAttributes->count());
   printf("Number of attributes List %d\n",pArray->pAttributeList->count());
    
    pAttribute = this->pFileAttributes->find("flat");
    if (pAttribute) {
        pAttribute->getValue(NDAttrInt32, &flat);
        full_header.flat = flat;
 //       printf("Flat is %d \n",flat);
  } else
  {
//          printf("No Flat %d \n");
  }
  
     pAttribute = this->pFileAttributes->find("dark");
    if (pAttribute) {
        pAttribute->getValue(NDAttrInt32, &dark);
        full_header.dark = dark;
 //       printf("Dark is %d \n",dark);
  } else
  {
//          printf("No Dark  %d \n");
  }
    

full_header.datatype = pArray->dataType;
full_header.ndims = pArray->ndims;
full_header.dims0size = pArray->dims[0].size;
full_header.dims1size = pArray->dims[1].size;
full_header.dims0offset = pArray->dims[0].offset;
full_header.dims1offset = pArray->dims[1].offset;
full_header.dims0binning = pArray->dims[0].binning;
full_header.dims1binning = pArray->dims[1].binning;
full_header.dims0reverse = pArray->dims[0].reverse;
full_header.dims1reverse = pArray->dims[1].reverse;
full_header.constant1 = 123456;
full_header.constant2 = 234567;
full_header.constant3 = 345678;
full_header.constant4 = 456789;
full_header.constant5 = 567890;
full_header.constant6 = 678901;
full_header.uniqueid = pArray->uniqueId;
full_header.datasize = pArray->dataSize;
full_header.timestamp = pArray->timeStamp;
full_header.epicsts = pArray->epicsTS;

  for( int a = 0; a < 106; a++){
      full_header.filler[a] = 0;
   }


	
//	this->file.write(header, 8192);
//	fwrite(header, 8192, pRawFile);

alignedbuffer = NULL;



posix_memalign(&alignedbuffer, 512, largestsize);


//printf("Buffer allocated\n");

//printf("sizeof fullheader %d \n",sizeof(full_header));

//	memcpy(alignedbuffer, (const int *)full_header,512);
	memcpy(alignedbuffer, ptr,512);
	
	write(rfile,alignedbuffer,roundUp(sizeof(full_header),512));
	
	return asynSuccess;
}




/** Writes NDArray data to a raw file.
  * \param[in] pArray Pointer to an NDArray to write to the file. This function can be called multiple
  *            times between the call to openFile and closeFile if NDFileModeMultiple was set in 
  *            openMode in the call to NDFileRaw::openFile.
  */
asynStatus NDFileRaw::writeFile(NDArray *pArray)
{
	asynStatus status = asynSuccess;
	static const char *functionName = "writeFile";
	long size;
	
//	printf("In Raw File Write . . . \n");

//	if (! this->file.is_open())
//	if (pRawFile == NULL) 
	if (rfile == 1)
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file is not open!\n", 
				  driverName, functionName);
		return asynError;
	}

//	this->file.write((const char*) pArray->pData, pArray->dims[0].size * pArray->dims[1].size );

//	fwrite((const char*)pArray->pData, 1, pArray->dataSize, pRawFile);

//printf(" Data size %d Buffer size %d \n",pArray->dataSize, largestsize);
try {


	memcpy(alignedbuffer, (const char*)pArray->pData,pArray->dataSize);
	
//printf("copied buffer \n");	
	
	write(rfile,alignedbuffer,roundUp(pArray->dataSize,512));
}
catch (...) {
//printf("NVME Aligned buffer write failed. \n");
}

//printf("got past here \n");

//	fwrite((const char*)pArray->pData, 1, pArray->dataSize, pRawFile);


	return asynSuccess;
}

/** Read NDArray data from a HDF5 file; NOTE: not implemented yet.
  * \param[in] pArray Pointer to the address of an NDArray to read the data into.  */ 
asynStatus NDFileRaw::readFile(NDArray **pArray)
{
  //static const char *functionName = "readFile";
  return asynError;
}

/** Closes the HDF5 file opened with NDFileRaw::openFile 
 */ 
asynStatus NDFileRaw::closeFile()
{
	epicsInt32 numCaptured;
	static const char *functionName = "closeFile";
	
//	printf("In Raw File close . . . \n");

//	if (!this->file.is_open())
//	if (pRawFile == NULL)
	if (rfile == -1) 
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file was not open! Ignoring close command.\n", 
				  driverName, functionName);
		return asynSuccess;
	}

//	this->file.close();
//	fclose(pRawFile);
	close(rfile);

	asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s file closed!\n", driverName, functionName);

	free(alignedbuffer);
	

//printf("file closed, buffer freed \n");

	return asynSuccess;
}


/** Constructor for NDFileHDF5; parameters are identical to those for NDPluginFile::NDPluginFile,
    and are passed directly to that base class constructor.
  * After calling the base class constructor this method sets NDPluginFile::supportsMultipleArrays=1.
  */
NDFileRaw::NDFileRaw(const char *portName, int queueSize, int blockingCallbacks, 
                     const char *NDArrayPort, int NDArrayAddr,
                     int priority, int stackSize)
  /* Invoke the base class constructor.
   * We allocate 2 NDArrays of unlimited size in the NDArray pool.
   * This driver can block (because writing a file can be slow), and it is not multi-device.  
   * Set autoconnect to 1.  priority and stacksize can be 0, which will use defaults. */
  : NDPluginFile(portName, queueSize, blockingCallbacks,
                 NDArrayPort, NDArrayAddr, 1, 
				 2, 0, asynGenericPointerMask, 
				 asynGenericPointerMask, ASYN_CANBLOCK, 1, 
				 priority, stackSize,1)
{
  //static const char *functionName = "NDFileRaw";
   setStringParam(NDPluginDriverPluginType, "NDFileRaw");
   
   this->supportsMultipleArrays = 1;
   
   this->pAttributeId = NULL;
   this->pFileAttributes = new NDAttributeList;


 //  posix_memalign(&nullbuffer, size, size);
//printf("Null Buffer created and aligned\n");

}



/** Configuration routine.  Called directly, or from the iocsh function in NDFileEpics */
extern "C" int NDFileRawConfigure(const char *portName, int queueSize, int blockingCallbacks, 
                                  const char *NDArrayPort, int NDArrayAddr,
                                  int priority, int stackSize)
{
 // new NDFileRaw(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, priority, stackSize);
  
//  return(asynSuccess);

    NDFileRaw *pPlugin = new NDFileRaw(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
                                         priority, stackSize);
    return pPlugin->start();

}


/** EPICS iocsh shell commands */
static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "frame queue size",iocshArgInt};
static const iocshArg initArg2 = { "blocking callbacks",iocshArgInt};
static const iocshArg initArg3 = { "NDArray Port",iocshArgString};
static const iocshArg initArg4 = { "NDArray Addr",iocshArgInt};
static const iocshArg initArg5 = { "priority",iocshArgInt};
static const iocshArg initArg6 = { "stack size",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
                                            &initArg3,
                                            &initArg4,
                                            &initArg5,
                                            &initArg6};
static const iocshFuncDef initFuncDef = {"NDFileRawConfigure",7,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
  NDFileRawConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval, 
                      args[4].ival, args[5].ival, args[6].ival);
}

extern "C" void NDFileRawRegister(void)
{
  iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
epicsExportRegistrar(NDFileRawRegister);
}

