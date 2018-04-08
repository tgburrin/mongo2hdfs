################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../hdfs/HdfsFile.cpp \
../hdfs/HdfsFileException.cpp 

OBJS += \
./hdfs/HdfsFile.o \
./hdfs/HdfsFileException.o 

CPP_DEPS += \
./hdfs/HdfsFile.d \
./hdfs/HdfsFileException.d 


# Each subdirectory must supply rules for building sources it contributes
hdfs/%.o: ../hdfs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/tgburrin/utils/include -I/home/tgburrin/utils/include/libmongoc-1.0 -I/home/tgburrin/utils/include/libbson-1.0 -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


