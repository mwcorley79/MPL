/* 
 * File:   Commands.h
 * Author: tsr
 *
 * Created on May 20, 2015, 7:31 PM
 */

#ifndef COMMANDS_H
#define	COMMANDS_H

enum class Commands { 
                      REQUEST_FILE_LIST=100, 
                      ACK_FILE_LIST=110, 
                      FILE_SEND=120,
                      FILE_RECV=130,
                      FILE_BLOCK=140, 
                      FILE_NOT_FOUND=150, 
                      FILE_CLOSE=170   
                    }; 

#endif	/* COMMANDS_H */

