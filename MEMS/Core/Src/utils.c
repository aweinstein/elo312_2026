#include "utils.h"

static char dataOut[MAX_BUF_SIZE];

/**
 * @brief This function split a float into integer and decimal components
 * @param The float in number
 * @param out_value A pointer to integer and decimal components of float in
 * @param dec_prec The number of decimals
 * @retval None
 */
void floatToInt(float in, displayFloatToInt_t *out_value, int32_t dec_prec)
{
	if(in >= 0.0f)
	{
		out_value->sign = 0;
	}else
	{
		out_value->sign = 1;
		in = -in;
	}

	in = in + (0.5f / pow(10, dec_prec));
	out_value->out_int = (int32_t)in;
	in = in - (float)(out_value->out_int);
	out_value->out_dec = (int32_t)trunc(in * pow(10, dec_prec));
}

/**
 * @brief This function initialize all sensor chips of the MEMS board
 * @retval none
 */
void IKS01A2_init(void)
{
	IKS01A2_MOTION_SENSOR_Init(IKS01A2_LSM6DSL_0, MOTION_ACCELERO | MOTION_GYRO);

	IKS01A2_MOTION_SENSOR_Init(IKS01A2_LSM303AGR_ACC_0, MOTION_ACCELERO);

	IKS01A2_MOTION_SENSOR_Init(IKS01A2_LSM303AGR_MAG_0, MOTION_MAGNETO);

	IKS01A2_ENV_SENSOR_Init(IKS01A2_HTS221_0, ENV_TEMPERATURE | ENV_HUMIDITY);

	IKS01A2_ENV_SENSOR_Init(IKS01A2_LPS22HB_0, ENV_TEMPERATURE | ENV_PRESSURE);
}

/**
 * @brief This function read and return the acceleration from the instanced sensor
 * @retval The structure from acceleration components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Accelero_Sensor_Handler(uint32_t Instance)
{
	IKS01A2_MOTION_SENSOR_Axes_t acceleration;

	if (IKS01A2_MOTION_SENSOR_GetAxes(Instance, MOTION_ACCELERO, &acceleration))
		printf("\r\nACC[%d]: Error\r\n", (int)Instance);

	return acceleration;
}

/**
 * @brief This function read and return the angular velocity from the instanced sensor
 * @retval The structure from angular velocity components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Gyro_Sensor_Handler(uint32_t Instance)
{

	IKS01A2_MOTION_SENSOR_Axes_t angular_velocity;
	if (IKS01A2_MOTION_SENSOR_GetAxes(Instance, MOTION_GYRO, &angular_velocity))
		printf(dataOut, MAX_BUF_SIZE, "\r\nGYR[%d]: Error\r\n", (int)Instance);

	return angular_velocity;
}

/**
 * @brief This function read and return the magnetic field from the instanced sensor
 * @retval The structure from magnetic field components x, y and z
 */
IKS01A2_MOTION_SENSOR_Axes_t Magneto_Sensor_Handler(uint32_t Instance)
{
	IKS01A2_MOTION_SENSOR_Axes_t magnetic_field;


	if (IKS01A2_MOTION_SENSOR_GetAxes(Instance, MOTION_MAGNETO, &magnetic_field))
		printf(dataOut, MAX_BUF_SIZE, "\r\nMAG[%d]: Error\r\n", (int)Instance);

	return magnetic_field;
}

/**
 * @brief This function read and return the temperature from the instanced sensor
 * @retval The structure from temperature sign, integer and decimal component
 */
displayFloatToInt_t Temp_Sensor_Handler(uint32_t Instance)
{
	float temperature;
	displayFloatToInt_t out_value;

	if (IKS01A2_ENV_SENSOR_GetValue(Instance, ENV_TEMPERATURE, &temperature))
		printf(dataOut, MAX_BUF_SIZE, "\r\nTemp[%d]: Error\r\n", (int)Instance);
	else
		floatToInt(temperature, &out_value, 2);
	return out_value;
}

/**
 * @brief This function read and return the pressure from the instanced sensor
 * @retval The structure from pressure  integer and decimal component
 */
displayFloatToInt_t Press_Sensor_Handler(uint32_t Instance)
{
	float pressure;
	displayFloatToInt_t out_value;

	if (IKS01A2_ENV_SENSOR_GetValue(Instance, ENV_PRESSURE, &pressure))
		printf(dataOut, MAX_BUF_SIZE, "\r\nPress[%d]: Error\r\n", (int)Instance);
	else
		floatToInt(pressure, &out_value, 2);

	return out_value;
}

/**
 * @brief This function read and return the humidity from the instanced sensor
 * @retval The structure from humidity integer and decimal component
 */
displayFloatToInt_t Hum_Sensor_Handler(uint32_t Instance)
{

	float humidity;
	displayFloatToInt_t out_value;


	if (IKS01A2_ENV_SENSOR_GetValue(Instance, ENV_HUMIDITY, &humidity))
		printf(dataOut, MAX_BUF_SIZE, "\r\nHum[%d]: Error\r\n", (int)Instance);
	else
		floatToInt(humidity, &out_value, 2);

	return out_value;
}
