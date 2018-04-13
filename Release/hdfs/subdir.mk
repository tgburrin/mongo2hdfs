################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../hdfs/HdfsFile.cpp \
../hdfs/HdfsFileException.cpp \
../hdfs/HdfsFileFactory.cpp 

OBJS += \
./hdfs/HdfsFile.o \
./hdfs/HdfsFileException.o \
./hdfs/HdfsFileFactory.o 

CPP_DEPS += \
./hdfs/HdfsFile.d \
./hdfs/HdfsFileException.d \
./hdfs/HdfsFileFactory.d 


# Each subdirectory must supply rules for building sources it contributes
hdfs/%.o: ../hdfs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I/Users/tburrington/utils/include -I/Users/tburrington/utils/include/libbson-1.0 -I/Users/tburrington/utils/include/libmongoc-1.0 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


