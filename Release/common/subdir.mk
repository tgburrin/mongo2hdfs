################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../common/ApplicationException.cpp \
../common/BookmarkManager.cpp \
../common/ProcessCfg.cpp 

OBJS += \
./common/ApplicationException.o \
./common/BookmarkManager.o \
./common/ProcessCfg.o 

CPP_DEPS += \
./common/ApplicationException.d \
./common/BookmarkManager.d \
./common/ProcessCfg.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I/Users/tburrington/utils/include -I/Users/tburrington/utils/include/libbson-1.0 -I/Users/tburrington/utils/include/libmongoc-1.0 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


