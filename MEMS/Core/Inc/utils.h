#ifndef INC_UTILS_H_
#define INC_UTILS_H_

/*Includes*/
#include "stdio.h"
#include "iks01a2_motion_sensors.h"
#include "iks01a2_env_sensors.h"

/*Defines*/
#define MAX_BUF_SIZE 256

/*Typedefs*/
typedef struct displayFloatToInt_s {
	int8_t sign; /* 0 means positive, 1 means negative*/
	uint32_t  out_int;
	uint32_t  out_dec;
} displayFloatToInt_t;

/*Variables*/
extern IKS01A2_MOTION_SENSOR_Capabilities_t MotionCapabilities[IKS01A2_MOTION_INSTANCES_NBR];
extern IKS01A2_ENV_SENSOR_Capabilities_t EnvCapabilities[IKS01A2_ENV_INSTANCES_NBR];

/*Function Prototipes*/

/**
 * @brief This function split a float into integer and decimal components
 * @param The float in number
 * @param out_value A pointer to integer and decimal components of float in
 * @param dec_prec The number of decimals
 * @retval None
 */
void floatToInt(float in, displayFloatToInt_t *out_value, int32_t dec_prec);

/**
 * @brief This function initialize all sensor chips of the MEMS board
 * @retval none
 */
void IKS01A2_init(void);

/**
 * @brief This function read and return the acceleration from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from acceleration components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Accelero_Sensor_Handler(uint32_t Instance);

/**
 * @brief This function read and return the angular velocity from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from angular velocity components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Gyro_Sensor_Handler(uint32_t Instance);

/**
 * @brief This function read and return the magnetic field from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from magnetic field components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Magneto_Sensor_Handler(uint32_t Instance);

/**
 * @brief This function read and return the temperature from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from temperature sign, integer and decimal component
 */
displayFloatToInt_t Temp_Sensor_Handler(uint32_t Instance);

/**
 * @brief This function read and return the pressure from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from pressure  integer and decimal component
 */
displayFloatToInt_t Press_Sensor_Handler(uint32_t Instance);

/**
 * @brief This function read and return the humidity from the instanced sensor
 * @param Instance The sensor instanced
 * @retval The structure from humidity integer and decimal component
 */
displayFloatToInt_t Hum_Sensor_Handler(uint32_t Instance);

#endif /* INC_UTILS_H_ */
