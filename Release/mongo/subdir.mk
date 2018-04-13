################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mongo/MongoException.cpp \
../mongo/MongoMessage.cpp \
../mongo/MongoOplogClient.cpp \
../mongo/MongoShardInfo.cpp \
../mongo/MongoUtilities.cpp 

OBJS += \
./mongo/MongoException.o \
./mongo/MongoMessage.o \
./mongo/MongoOplogClient.o \
./mongo/MongoShardInfo.o \
./mongo/MongoUtilities.o 

CPP_DEPS += \
./mongo/MongoException.d \
./mongo/MongoMessage.d \
./mongo/MongoOplogClient.d \
./mongo/MongoShardInfo.d \
./mongo/MongoUtilities.d 


# Each subdirectory must supply rules for building sources it contributes
mongo/%.o: ../mongo/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I/Users/tburrington/utils/include -I/Users/tburrington/utils/include/libbson-1.0 -I/Users/tburrington/utils/include/libmongoc-1.0 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


