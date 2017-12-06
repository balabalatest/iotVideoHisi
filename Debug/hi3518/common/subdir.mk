################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hi3518/common/loadbmp.c \
../hi3518/common/sample_comm_audio.c \
../hi3518/common/sample_comm_isp.c \
../hi3518/common/sample_comm_ive.c \
../hi3518/common/sample_comm_sys.c \
../hi3518/common/sample_comm_venc.c \
../hi3518/common/sample_comm_vi.c \
../hi3518/common/sample_comm_vo.c \
../hi3518/common/sample_comm_vpss.c 

OBJS += \
./hi3518/common/loadbmp.o \
./hi3518/common/sample_comm_audio.o \
./hi3518/common/sample_comm_isp.o \
./hi3518/common/sample_comm_ive.o \
./hi3518/common/sample_comm_sys.o \
./hi3518/common/sample_comm_venc.o \
./hi3518/common/sample_comm_vi.o \
./hi3518/common/sample_comm_vo.o \
./hi3518/common/sample_comm_vpss.o 

C_DEPS += \
./hi3518/common/loadbmp.d \
./hi3518/common/sample_comm_audio.d \
./hi3518/common/sample_comm_isp.d \
./hi3518/common/sample_comm_ive.d \
./hi3518/common/sample_comm_sys.d \
./hi3518/common/sample_comm_venc.d \
./hi3518/common/sample_comm_vi.d \
./hi3518/common/sample_comm_vo.d \
./hi3518/common/sample_comm_vpss.d 


# Each subdirectory must supply rules for building sources it contributes
hi3518/common/%.o: ../hi3518/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv300-linux-uclibcgnueabi-gcc -Dhi3518e -Dhi3518ev200 -DISP_V2 -DHICHIP=0x3518E200 -DHI_XXXX -DHI_ACODEC_TYPE_INNER -DHI_RELEASE -DCHIP_ID=CHIP_HI3518E_V200 -DSENSOR_TYPE=APTINA_AR0130_DC_720P_30FPS -I"/home/lichen/eclipse-workspace/iotVideoHisi/iotRtmp" -I"/home/lichen/eclipse-workspace/iotVideoHisi/iotMqtt" -I"/home/lichen/eclipse-workspace/iotVideoHisi/iotMqtt/libmqtt" -I"/home/lichen/eclipse-workspace/iotVideoHisi/iotMqtt/client" -I"/home/lichen/eclipse-workspace/iotVideoHisi/hi3518/include" -I"/home/lichen/eclipse-workspace/iotVideoHisi/hi3518/common" -I"/home/lichen/eclipse-workspace/iotVideoHisi/src" -I"/home/lichen/eclipse-workspace/iotVideoHisi/misc" -I"/home/lichen/eclipse-workspace/iotVideoHisi/cJSON" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


