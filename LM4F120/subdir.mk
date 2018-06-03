################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gcode.c \
../joystick.c \
../lcd.c \
../main.c \
../motion_control.c \
../newlib_stubs.c \
../planner.c \
../sense_control.c \
../serial.c \
../startup.c \
../stepper.c \
../tasks.c \
../temperature.c \
../usb_serial_structs.c 

OBJS += \
./gcode.o \
./joystick.o \
./lcd.o \
./main.o \
./motion_control.o \
./newlib_stubs.o \
./planner.o \
./sense_control.o \
./serial.o \
./startup.o \
./stepper.o \
./tasks.o \
./temperature.o \
./usb_serial_structs.o 

C_DEPS += \
./gcode.d \
./joystick.d \
./lcd.d \
./main.d \
./motion_control.d \
./newlib_stubs.d \
./planner.d \
./sense_control.d \
./serial.d \
./startup.d \
./stepper.d \
./tasks.d \
./temperature.d \
./usb_serial_structs.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -Dgcc=1 -DENABLE_LCD -DJOY_INVERT_Y -DTARGET_IS_BLIZZARD_RA1 -DARM_MATH_CM4 -DPART_TM4C1233H6PM -I"/home/uppsalamakerspace/laser/LasaurGrbl_UMS/LasaurGrbl" -Os -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -ffunction-sections -fdata-sections -g3 -pedantic -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


