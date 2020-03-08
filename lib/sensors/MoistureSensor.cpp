#include "mbed.h"
#include "MoistureSensor.h"

constexpr int Sensor_I2C_Address = 0x36 << 1;                               //MBED I2C uses 8 bit addressing, so addresses are left shifted by 1 (may need to be shifted by 2) 

constexpr int Sensor_Status_Base = 0x00;                                    //Base address registers for different modules
constexpr int Sensor_Moisture_Base = 0x0F;

constexpr int Sensor_Status_HW_ID = 0x01;                                   //Function address register for the sensor's HW ID
constexpr int Sensor_HW_ID_Code = 0x68;                                     //Expected value for sensor HW ID; NOTE: This is not the expected value as specified by the datasheet,
                                                                            //it's just what I found through my own testing. It sometimes fluctuates on its own, so it's hard to trust :(

constexpr int Sensor_Moisture_Function = 0x10;                              //Function address registers for moisture and temperature modules        
constexpr int Sensor_Temp_Function = 0x04;  

MoistureSensor::MoistureSensor(PinName sda, PinName scl) : i2c_(sda, scl){}

bool MoistureSensor::Is_Initialized(){
    if(this->Read_HW_ID() == Sensor_HW_ID_Code){                            //compare received HW ID Code to correct one
        return true;
    }

    return false;
}

int MoistureSensor::Read_HW_ID(){
    char cmd[2];
    cmd[0] = Sensor_Status_Base;
    cmd[1] = Sensor_Status_HW_ID;

    char check[1];

    i2c_.write(Sensor_I2C_Address, cmd, 2);                                 //initialize registers for checking device ID

    i2c_.read(Sensor_I2C_Address, check, 1);                                //read device ID

    return check[0];
}

uint16_t MoistureSensor::Read_Moisture(){
    /*
    if(!(this->Is_Initialized())){                                          //checks if device is initialized, returns 65534 if there is an issue
        return 65534;                                                       //this is commented out because the HW_ID reading isn't consistent currently
    }
    */

    char cmd[2];
    cmd[0] = Sensor_Moisture_Base;
    cmd[1] = Sensor_Moisture_Function;

    char buf[2];
    uint16_t ret = 65535;

    do{
        wait_ms(1);

        i2c_.write(Sensor_I2C_Address, cmd, 2);                             //initialize registers for reading moisture

        wait_ms(1);

        i2c_.read(Sensor_I2C_Address, buf, 2);                              //read moisture

        ret = ((uint16_t)buf[0] << 8 | buf[1]);                             //concatenate bytes together

    } while(ret == 65535);                                                  //repeat until value has been measured

    return ret;
}

float MoistureSensor::Read_Temperature(){
    /*
    if(!(this->Is_Initialized())){                                          //checks if device is initialized, returns -273.0 if there is an issue
        return -273.0;                                                      //this is commented out because the HW_ID reading isn't consistent currently
    }
    */

    char cmd[2];
    cmd[0] = Sensor_Status_Base;
    cmd[1] = Sensor_Temp_Function;

    char buf[4];

    i2c_.write(Sensor_I2C_Address, cmd, 2);                                 //initialize registers for reading temperature

    wait_ms(1);

    i2c_.read(Sensor_I2C_Address, buf, 4);                                  //read temp

    int32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |     //concatenate bytes together
                  ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];

    return (1.0 / (1UL << 16)) * ret;
}