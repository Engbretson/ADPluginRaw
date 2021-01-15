/* NDFileHDF5.h
 * Writes NDArrays to HDF5 files.
 *
 * Ulrik Kofoed Pedersen
 * March 20. 2011
 */

#include <fstream>
#include <asynDriver.h>
#include <NDPluginFile.h>
#include <NDArray.h>

#define largestsize  251666336 // 4096*3072*2 bytes + 512 slop

class epicsShareClass NDFileRaw : public NDPluginFile
{
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
//	std::ofstream file;
//	FILE* pRawFile;
	int rfile;
	void *alignedbuffer;
	    int *pAttributeId;
    NDAttributeList *pFileAttributes;



};

