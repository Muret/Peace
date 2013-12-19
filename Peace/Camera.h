
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "includes.h"

void init();

void handle_user_input_down(char key);

void handle_user_input_up(char key);

void tick_user_inputs();

void startMovingCamera();

void stopMovingCamera();

void set_render_constants_buffer();


#endif