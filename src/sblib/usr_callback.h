/*
 * usr_callback.h
 *
 *  Created on: May 26, 2016
 *      Author: Florian Voelzke
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 */

#ifndef SBLIB_USR_CALLBACK_H_
#define SBLIB_USR_CALLBACK_H_

#define USR_CALLBACK_RESET         1
#define USR_CALLBACK_FLASH         2
#define USR_CALLBACK_BCU_END       3

class UsrCallback
{
public:
    virtual void Notify(int type)=0;
};

#endif
