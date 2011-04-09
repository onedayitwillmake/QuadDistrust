################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/QuadDistrust.cpp \
../src/Simplex.cpp \
../src/ZoaDebugFunctions.cpp 

OBJS += \
./src/QuadDistrust.o \
./src/Simplex.o \
./src/ZoaDebugFunctions.o 

CPP_DEPS += \
./src/QuadDistrust.d \
./src/Simplex.d \
./src/ZoaDebugFunctions.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/onedayitwillmake/GIT/LIBRARY/Cinder/boost -I/Users/onedayitwillmake/GIT/LIBRARY/Cinder/include -I"/Users/onedayitwillmake/GIT/QuadDistrust/includes" -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I"/Users/onedayitwillmake/GIT/QuadDistrust/thirdparty/ruiNoise/includes" -O0 -g -Wall -c -fmessage-length=0 -arch i386 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


