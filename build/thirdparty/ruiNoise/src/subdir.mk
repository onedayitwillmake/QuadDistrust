################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../thirdparty/ruiNoise/src/ofxPerlin.cpp \
../thirdparty/ruiNoise/src/ofxSimplex.cpp 

OBJS += \
./thirdparty/ruiNoise/src/ofxPerlin.o \
./thirdparty/ruiNoise/src/ofxSimplex.o 

CPP_DEPS += \
./thirdparty/ruiNoise/src/ofxPerlin.d \
./thirdparty/ruiNoise/src/ofxSimplex.d 


# Each subdirectory must supply rules for building sources it contributes
thirdparty/ruiNoise/src/%.o: ../thirdparty/ruiNoise/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/mariogonzalez/GIT/LIBRARY/Cinder/boost -I/Users/mariogonzalez/GIT/LIBRARY/Cinder/include -I"/Users/mariogonzalez/GIT/QuadDistrust/includes" -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I"/Users/mariogonzalez/GIT/QuadDistrust/thirdparty/ruiNoise/includes" -I"/Users/mariogonzalez/GIT/QuadDistrust/includes/OpenNI" -O1 -g -Wall -c -fmessage-length=0 -arch i386 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


