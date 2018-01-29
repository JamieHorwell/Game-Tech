#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.hpp>
//Boiler-Plate code for setup of OpenCL progra
cl::Program CreateProgramGPU(const std::string& file) {

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	auto platform = platforms.front();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

	auto device = devices.front();

	std::ifstream KernerlFile(file);
	std::string src(std::istreambuf_iterator<char>(KernerlFile), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	cl::Context context(device);
	cl::Program program(context, sources);
	
	cl_int err;
	err = program.build("-cl-std=CL1.2");
	std::cout << "PROGRAM ERROR" << err;
	return program; 
}