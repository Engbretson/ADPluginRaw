/*
 * NDFileRaw.h
 */

#ifndef DRV_NDFileNULL_H
#define DRV_NDFileNULL_H


#include <fstream>
#include <asynDriver.h>
#include <NDPluginFile.h>
#include <NDArray.h>

/** Writes NDArrays in the Raw file format. */

class epicsShareClass NDFileRaw : public NDPluginFile {
public:
    NDFileRaw(const char *portName, int queueSize, int blockingCallbacks,
                 const char *NDArrayPort, int NDArrayAddr,
                 int priority, int stackSize);

    /* The methods that this class implements */
    virtual asynStatus openFile(const char *fileName, NDFileOpenMode_t openMode, NDArray *pArray);
    virtual asynStatus readFile(NDArray **pArray);
    virtual asynStatus writeFile(NDArray *pArray);
    virtual asynStatus closeFile();
	
	  protected:
    /* plugin parameters */

  private:
	std::ofstream file;
};
#define NUM_NDFILE_RAW_PARAMS 0
#endif
