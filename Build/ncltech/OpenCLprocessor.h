#pragma once
#include "OpenCLSetup.h"
#include <nclgl/Vector4.h>
class OpenCLprocessor
{
public:
	OpenCLprocessor();
	~OpenCLprocessor();

	void initBuffers(int nCons, Vector4* posA, Vector4* posB, Vector4* velA, Vector4* velB, float* massA, float* massB, float* targetLength);
	

	void updateBuffers(int nCons, Vector4 * posA, Vector4 * posB, Vector4 * velA, Vector4 * velB);


	void updateConstraints(int nCons, bool lastIt);

	void updateConstraints(int nCons, Vector4* velA, Vector4* velB);

	void readBack(int nCons, Vector4* velA, Vector4* velB);

	void cleanUp();

private:
	cl::Program constraintProg;
	cl::Context context;
	cl::CommandQueue queue;
	cl_int err;
	cl::Kernel kernel;

	cl::Buffer posABuf;
	cl::Buffer posBBuf;

	cl::Buffer velAReadBuf;
	cl::Buffer velBReadBuf;

	cl::Buffer velAWriteBuf;
	cl::Buffer velBWriteBuf;

	cl::Buffer massABuf;
	cl::Buffer massBBuf;

	cl::Buffer targLengthBuf;
};

