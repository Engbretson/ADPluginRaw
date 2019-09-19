/* NDFileRaw.cpp
 * Dummy file writer, whose main purpose is to allow deleting original driver files without re-writing them in 
 * an actual file plugin.
 *
 * Mark Rivers
 * November 30, 2011
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <epicsTypes.h>
#include <epicsMessageQueue.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsTime.h>
#include <iocsh.h>

#include <asynDriver.h>

#include <epicsExport.h>
#include "NDFileRaw.h"

static const char *driverName = "NDFileRaw";

/** Opens a Raw file.
  * \param[in] fileName The name of the file to open.
  * \param[in] openMode Mask defining how the file should be opened; bits are 
  *            NDFileModeRead, NDFileModeWrite, NDFileModeAppend, NDFileModeMultiple
  * \param[in] pArray A pointer to an NDArray; this is used to determine the array and attribute properties.
  */
asynStatus NDFileRaw::openFile(const char *fileName, NDFileOpenMode_t openMode, NDArray *pArray)
{
	static const char *functionName = "openFile";

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
	if (this->file.is_open())    { this->closeFile(); }

	// Create the new file
	this->file.open(fileName, std::ofstream::binary);
	
	if (! this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
				  "%s::%s ERROR Failed to create a new output file\n",
				  driverName, functionName);
		return asynError;
	}
	
	// Write 8192 byte header, currently just zeroes.
	char header[8192] = {0};
	
	this->file.write(header, 8192);
	

    return(asynSuccess);
}

/** Writes single NDArray to the Raw file.
  * \param[in] pArray Pointer to the NDArray to be written
  */
asynStatus NDFileRaw::writeFile(NDArray *pArray)
{
	asynStatus status = asynSuccess;
	static const char *functionName = "writeFile";

	if (! this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file is not open!\n", 
				  driverName, functionName);
		return asynError;
	}

	this->file.write((const char*) pArray->pData, pArray->dims[0].size * pArray->dims[1].size * 2);

    return(asynSuccess);
}

/** Reads single NDArray from a Raw file; NOT CURRENTLY IMPLEMENTED.
  * \param[in] pArray Pointer to the NDArray to be read
  */
asynStatus NDFileRaw::readFile(NDArray **pArray)
{
    return asynError;
}


/** Closes the Raw file. */
asynStatus NDFileRaw::closeFile()
{
	epicsInt32 numCaptured;
	static const char *functionName = "closeFile";

	if (!this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file was not open! Ignoring close command.\n", 
				  driverName, functionName);
		return asynSuccess;
	}

	this->file.close();

	asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s file closed!\n", driverName, functionName);

    return asynSuccess;
}


/** Constructor for NDFileRaw; all parameters are simply passed to NDPluginFile::NDPluginFile.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] queueSize The number of NDArrays that the input queue for this plugin can hold when 
  *            NDPluginDriverBlockingCallbacks=0.  Larger queues can decrease the number of dropped arrays,
  *            at the expense of more NDArray buffers being allocated from the underlying driver's NDArrayPool.
  * \param[in] blockingCallbacks Initial setting for the NDPluginDriverBlockingCallbacks flag.
  *            0=callbacks are queued and executed by the callback thread; 1 callbacks execute in the thread
  *            of the driver doing the callbacks.
  * \param[in] NDArrayPort Name of asyn port driver for initial source of NDArray callbacks.
  * \param[in] NDArrayAddr asyn port driver address for initial source of NDArray callbacks.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
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
                   2, 0, asynGenericPointerMask, asynGenericPointerMask, 
                   ASYN_CANBLOCK, 1, priority, stackSize, 1)
{
    //static const char *functionName = "NDFileRaw";

    /* Set the plugin type string */    
    setStringParam(NDPluginDriverPluginType, "NDFileRaw");
    this->supportsMultipleArrays = 0;
}

/* Configuration routine.  Called directly, or from the iocsh  */

extern "C" int NDFileRawConfigure(const char *portName, int queueSize, int blockingCallbacks,
                                   const char *NDArrayPort, int NDArrayAddr,
                                   int priority, int stackSize)
{
    NDFileRaw *pPlugin = new NDFileRaw(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
                                         priority, stackSize);
    return pPlugin->start();
}

/* EPICS iocsh shell commands */

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
    NDFileRawConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval, args[4].ival, args[5].ival, args[6].ival);
}

extern "C" void NDFileRawRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
epicsExportRegistrar(NDFileRawRegister);
}
