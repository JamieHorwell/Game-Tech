#include "OpenCLprocessor.h"
#include <iostream>



OpenCLprocessor::OpenCLprocessor()
{
	constraintProg = CreateProgramGPU("../solveConstraints.cl");
	context = constraintProg.getInfo<CL_PROGRAM_CONTEXT>();
	auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
	auto& device = devices.front();
	std::cout << "-----OPENCL DEVICE------" << device.getInfo<CL_DEVICE_NAME>() << "\n";
	queue = cl::CommandQueue(context, device);

}


OpenCLprocessor::~OpenCLprocessor()
{
}

void OpenCLprocessor::initBuffers(int nCons, Vector4* posA, Vector4* posB, Vector4* velA, Vector4* velB, float* massA, float* massB, float* targetLength)
{
	
	//Dont worry about rotational, as cloth nodes should not rotate
	std::cout << "CONSTRAINTS" << nCons;
	//send PnodeA position every frame
	posABuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Vector4) * nCons, &posA[0], &err);
	std::cout << "ERROR BUFFER:" << err << "\n";

	//send PnodeB position every frame
	posBBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY| CL_MEM_COPY_HOST_PTR, sizeof(Vector4) * nCons, &posB[0], &err);
	std::cout << "ERROR BUFFER:" << err << "\n";

	////send PnodeA linVel every frame
	velAReadBuf = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Vector4) * nCons, &velA[0], &err);
	std::cout << "ERROR BUFFER:" << err << "\n";
	////send PnodeB linVel every frame
	velBReadBuf = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Vector4) * nCons, &velB[0], &err);
	std::cout << "ERROR BUFFER:" << err << "\n";

	velAWriteBuf = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY , sizeof(Vector4) * nCons, NULL, &err);
	velBWriteBuf = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, sizeof(Vector4) * nCons, NULL, &err);


	////send pnodeA invMass ONCE
	massABuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(float) * nCons, massA, &err);
	std::cout << "ERROR BUFFER:" << err << "\n";
	////send pnodeB invMass ONCE
	massBBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(float) * nCons, massB, &err);
	std::cout << "ERROR BUFFER:" << err << "\n";


	////send targetlength ONCE
	targLengthBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(float) * nCons, targetLength, &err);
	std::cout << "ERROR BUFFER:" << err << "\n";
	//send delta t ONCE?



	kernel = cl::Kernel(constraintProg, "solveConstraints");
	float time = 0.0013;
	int last = 0;
	std::cout << "ERROR BEFORE:" << err << "\n";
	err = kernel.setArg(0, posABuf);
	std::cout << "ERROR ARG 1:" << err << "\n";;
	err = kernel.setArg(1, posBBuf);
	std::cout << "ERROR ARG 2:" << err << "\n";;
	err = kernel.setArg(2, velAReadBuf);
	std::cout << "ERROR ARG 3:" << err << "\n";;
	err = kernel.setArg(3, velBReadBuf);
	std::cout << "ERROR ARG 4:" << err << "\n";;
	err = kernel.setArg(4, massABuf);
	std::cout << "ERROR ARG 5:" << err << "\n";;
	err = kernel.setArg(5, massBBuf);
	std::cout << "ERROR ARG 6:" << err << "\n";;
	err = kernel.setArg(6, targLengthBuf);
	std::cout << "ERROR ARG 7:" << err << "\n";;
	err = kernel.setArg(7, sizeof(float), &time);
	std::cout << "ERROR ARG 8:" << err << "\n";;
	err = kernel.setArg(8, velAWriteBuf);
	err = kernel.setArg(9, velBWriteBuf);
	err = kernel.setArg(10, sizeof(int), &last);
	
}

void OpenCLprocessor::updateBuffers(int nCons, Vector4 * posA, Vector4 * posB, Vector4 * velA, Vector4 * velB)
{
	//write new values to buffers
	int last = 0;
	kernel.setArg(10, sizeof(int), &last);
	err = queue.enqueueWriteBuffer(posABuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &posA[0], NULL, NULL);
	err = queue.enqueueWriteBuffer(posBBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &posB[0], NULL, NULL);
	err = queue.enqueueWriteBuffer(velAWriteBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velA[0], NULL, NULL);
	err = queue.enqueueWriteBuffer(velBWriteBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velB[0], NULL, NULL);
	err = queue.finish();
}

void OpenCLprocessor::updateConstraints(int nCons, bool lastIt)
{
	if (lastIt == true) {
		int last = 1;
		kernel.setArg(10, sizeof(int), &last);
	}
	else {
		int last = 0;
		kernel.setArg(10, sizeof(int), &last);
	}
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nCons));
	err = queue.finish();
}

void OpenCLprocessor::updateConstraints(int nCons, Vector4 * velA, Vector4 * velB)
{
	int last = 0;
	kernel.setArg(10, sizeof(int), &last);
	err = queue.enqueueWriteBuffer(velAWriteBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velA[0], NULL, NULL);
	err = queue.enqueueWriteBuffer(velBWriteBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velB[0], NULL, NULL);
	err = queue.finish();

	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nCons));
	err = queue.finish();

	err = queue.enqueueReadBuffer(velAReadBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velA[0]);
	err = queue.enqueueReadBuffer(velBReadBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velB[0]);
	err = queue.finish();
}

void OpenCLprocessor::readBack(int nCons, Vector4 * velA, Vector4 * velB)
{
	err = queue.enqueueReadBuffer(velAReadBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velA[0]);
	err = queue.enqueueReadBuffer(velBReadBuf, CL_TRUE, 0, sizeof(Vector4) * nCons, &velB[0]);
	err = queue.finish();
}

void OpenCLprocessor::cleanUp()
{
	
}
