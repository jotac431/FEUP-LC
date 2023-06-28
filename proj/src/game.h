#pragma once


#include "game.h"
#include "devices/video.h"
#include "xpms/pongBackground.xpm"
#include "xpms/ball.xpm"
#include "devices/keyboard.h"
#include "devices/i8042.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @struct ball
 * @brief this struct represent the ball in the pong game
 * @var ball::x
 * Member 'x' contains the x coordinate of the ball's current position
 * @var ball::y
 * Member 'y' contains the y coordinate of the ball's current position
 * @var ball::dx
 * Member 'dx' contains the change that will be applied to the ball's x coordinate
 * @var ball::dy
 * Member 'dy' contains the change that will be applied to the ball's y coordinate 
 * @var ball::sprite
 * Member 'sprite' contains the ball's image
 *  */
typedef struct ball_s {
  
  int x,y;            
  int dx,dy;          
  xpm_image_t sprite; 
} ball_t;

/**
 * @struct paddle
 * @brief this struct represent the paddle in the pong game
 * @var paddle::x
 * Member 'x' contains the x coordinate of the paddle's current position
 * @var paddle::y
 * Member 'y' contains the y coordinate of the paddle's current position
 * @var paddle::dx
 * Member 'dx' contains the change that will be applied to the paddle's x coordinate
 * @var paddle::dy
 * Member 'dy' contains the change that will be applied to the paddle's y coordinate 
 * @var paddle::sprite
 * Member 'sprite' contains the paddle's image
 *  */
typedef struct paddle_s {

  int x,y;            //Coordenada da posição atual
  int dx,dy;          //Vetor de movimento
  xpm_image_t sprite; //Imagem da barra
} paddle_t;

/**
 * @brief function that starts the game
 * 
 * @return 0
 */
int play();

/**
 * @brief function that checks the collision between the ball and one of the paddle's
 * 
 * @param b Ball
 * @param p Paddle
 * @return 1
 */
int check_collision(ball_t b, paddle_t p);

/**
 * @brief function that controls the ball's movement
 * 
 */
void move_ball();

/**
 * @brief function that controls the left's paddle movement with the arrow keys
 * 
 * @return 0
 */
int move_paddle_arrows();

/**
 * @brief function that controls the right's paddle movement with the W and D keys
 * 
 * @return 0
 */
int move_paddle_ws();

/**
 * @brief function that draws the two paddles
 * 
 */
void draw_paddles();
