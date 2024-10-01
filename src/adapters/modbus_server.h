/*
 * modbus_server.h
 *
 *  Created on: 27 Jun 2024
 *      Author: Maldus
 */

#ifndef ADAPTERS_MODBUS_SERVER_H_
#define ADAPTERS_MODBUS_SERVER_H_


#include "model/model.h"


void modbus_server_init(void);
void modbus_server_manage(mut_model_t *p_model);


#endif /* ADAPTERS_MODBUS_SERVER_H_ */
