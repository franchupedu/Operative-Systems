################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EjecuciónDeRequests.c \
../Hermafrodita.c \
../ParseoDeRequests.c \
../Planificador.c \
../Prueba.c 

OBJS += \
./EjecuciónDeRequests.o \
./Hermafrodita.o \
./ParseoDeRequests.o \
./Planificador.o \
./Prueba.o 

C_DEPS += \
./EjecuciónDeRequests.d \
./Hermafrodita.d \
./ParseoDeRequests.d \
./Planificador.d \
./Prueba.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


