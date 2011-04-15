################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CinderOpenNI.cpp \
../src/QuadDistrust.cpp \
../src/ZoaDebugFunctions.cpp 

OBJS += \
./src/CinderOpenNI.o \
./src/QuadDistrust.o \
./src/ZoaDebugFunctions.o 

CPP_DEPS += \
./src/CinderOpenNI.d \
./src/QuadDistrust.d \
./src/ZoaDebugFunctions.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/mariogonzalez/GIT/library/Cinder/boost -I/Users/mariogonzalez/GIT/library/Cinder/include -I"/Users/mariogonzalez/GIT/QuadDistrust/includes" -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I"/Users/mariogonzalez/GIT/QuadDistrust/thirdparty/ruiNoise/includes" -I"/Users/mariogonzalez/GIT/QuadDistrust/includes/OpenNI" -O0 -g -Wall -c -fmessage-length=0 -arch i386 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


