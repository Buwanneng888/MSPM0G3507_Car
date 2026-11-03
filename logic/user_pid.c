/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include <math.h>
#include "user_pid.h"



#define V_L_MAX 120                         /* 左轮速度目标上限，单位：cm/s。 */
#define V_R_MAX 120                         /* 右轮速度目标上限，单位：cm/s。 */
#define PID_DEBUG_PRINT 0                   /* 速度/循迹 PID 调试打印开关。 */


#define HEADING_STEP_DEG 90.0f               /* 调试按键每次修改的航向角度，单位：度。 */
#define HEADING_KEY_DEBOUNCE_MS 30U          /* 航向调试按键消抖时间，单位：ms。 */
#define HEADING_STOP_TOLERANCE_DEG 1.0f      /* 航向控制死区，误差不超过该值时输出零。 */
#define HEADING_MIN_TURN_SPEED_CM_S 4.0f     /* 克服电机死区所需的最小原地转向轮速。 */

int basic_speed = 30; 												/* 当前基础目标速度，单位：cm/s。 */

/* 四个控制器的运行状态：参数、目标、误差、积分和输出均保存在 PID_T 中。 */
PID_T pid_speed_left; 											 						  /* 左轮速度闭环控制器，目标/反馈单位：cm/s。 */
PID_T pid_speed_right;  														  		/* 右轮速度闭环控制器，目标/反馈单位：cm/s。 */	
						                                            
PID_T pid_line;         															    /* 循迹位置外环控制器，输入为归一化横向偏差。 */	
PID_T pid_heading;    											 						  /* 航向外环控制器，目标/反馈单位：度。 */
static uint32_t heading_debug_last_print; 							  /* 上一次航向调试打印时刻，单位：ms。 */	


static float heading_normalize_angle(float angle)
{
  while (angle >= 180.0f)
  {
    angle -= 360.0f;
  }
  while (angle < -180.0f)
  {
    angle += 360.0f;
  }

  return angle;
}


/* 左轮速度环参数。 */
PidParams_t pid_params_left = {
		 .kp =3.5f,    				 
			.ki =1.0f,   		
     .kd =0.0f,   		  
		 .out_min = -V_L_MAX,
     .out_max = V_L_MAX,
	
};
/* 右轮速度环参数。 */
PidParams_t pid_params_right = {
    .kp = 1.5f,     
	 .ki = 0.6f,  
  
    .kd = 0.0f,     
	  .out_min = -V_R_MAX,
    .out_max = V_R_MAX,
};

/* 循迹位置外环参数，输出用于形成左右轮差速目标。 */
PidParams_t pid_params_line = {
    .kp = 3.2f,
    .ki = 0.0001f,
    .kd = 0.0f,
    .out_min = -30.0f,
    .out_max = 30.0f,
};

/* 航向外环参数，输出为左右轮相反方向的转向速度分量。 */
PidParams_t pid_params_heading = {
    .kp = 0.5f,		
    .ki = 0.0f,
    .kd = 0.0f,
    .out_min = -15.0f,
    .out_max = 15.0f,
};


void PID_Init(void)
{
  my_printf(UART_0_INST, "pid init ok\r\n");
  pid_init(&pid_speed_left,
           pid_params_left.kp, pid_params_left.ki, pid_params_left.kd,
           0.0f, pid_params_left.out_max);
  
  pid_init(&pid_speed_right,
           pid_params_right.kp, pid_params_right.ki, pid_params_right.kd,
           0.0f, pid_params_right.out_max);
  

  pid_init(&pid_line,
           pid_params_line.kp, pid_params_line.ki, pid_params_line.kd,
           0.0f, pid_params_line.out_max);

  pid_init(&pid_heading,
           pid_params_heading.kp, pid_params_heading.ki, pid_params_heading.kd,
           0.0f, pid_params_heading.out_max);
   
  pid_set_target(&pid_speed_left, basic_speed);
  pid_set_target(&pid_speed_right, basic_speed);
  pid_set_target(&pid_line, 0);
  pid_set_target(&pid_heading, 0);
  heading_debug_last_print = 0;

}

void PID_SetBasicSpeed(int speed)
{
  if (speed < 0)
  {
    speed = 0;
  }
  if (speed > V_L_MAX)
  {
    speed = V_L_MAX;
  }
  if (speed > V_R_MAX)
  {
    speed = V_R_MAX;
  }

  basic_speed = speed;
  pid_set_target(&pid_speed_left, (float)basic_speed);
  pid_set_target(&pid_speed_right, (float)basic_speed);
}

#define LINE_TURN_MS    (uint16_t)20/basic_speed 
void Line_PID_control(void)
{
    static float last_valid_error = 0.0f; /* 最近一次有效循迹误差，丢线时用于决定原地搜索方向。 */
    float line_pid_output = 0.0f;
    float left_target = 0.0f;
    float right_target = 0.0f;

    /*
     * 正常检测到黑线：根据灰度偏差计算左右轮目标速度。
     * 丢线：不再使用 while 阻塞，而是按最后一次看到黑线的方向搜索。
     */
    if (gray_noroad_enable == false)
    {
        last_valid_error = g_line_position_error;

        line_pid_output = pid_calculate_positional(&pid_line, g_line_position_error);
        line_pid_output = pid_constrain(line_pid_output,
                                        pid_params_line.out_min,
                                        pid_params_line.out_max);

        left_target = (float)basic_speed - line_pid_output;
        right_target = (float)basic_speed + line_pid_output;

        left_target = pid_constrain(left_target, 0.0f, (float)V_L_MAX);
        right_target = pid_constrain(right_target, 0.0f, (float)V_R_MAX);
    }
    else
    {
        const float search_speed = 20.0f;

        /* 丢线时清除巡线积分，避免重新找到线后瞬间冲击。 */
        pid_reset(&pid_line);

        if (last_valid_error < 0.0f)
        {
            /* 最后一次黑线在左侧，向左原地寻找。 */
            left_target = -search_speed;
            right_target = search_speed;
        }
        else if (last_valid_error > 0.0f)
        {
            /* 最后一次黑线在右侧，向右原地寻找。 */
            left_target = search_speed;
            right_target = -search_speed;
        }
        else
        {
            /* 上电后还没有检测到黑线，先停车。 */
            left_target = 0.0f;
            right_target = 0.0f;
        }
    }

    /* 将巡线计算结果写入原有左右速度环；速度环本身不做任何修改。 */
    pid_set_target(&pid_speed_left, left_target);
    pid_set_target(&pid_speed_right, right_target);
}

static float heading_target = 0.0f; /* 当前航向目标，范围 [-180°, 180°)。 */

void Heading_PID_control(void)
{
  float yaw = imu_imu_get_yaw();
  float heading_error = 0.0f;
  float yaw_for_pid = 0.0f;
  float heading_pid_output = 0.0f;
  float left_target = 0.0f;
  float right_target = 0.0f;
	

  heading_error = heading_normalize_angle(pid_heading.target - yaw);
  if (fabsf(heading_error) <= HEADING_STOP_TOLERANCE_DEG)
  {
    heading_error = 0.0f;
  }
  yaw_for_pid = pid_heading.target - heading_error;
  heading_pid_output = pid_calculate_positional(&pid_heading, yaw_for_pid);
  heading_pid_output = pid_constrain(heading_pid_output,
                                     pid_params_heading.out_min,
                                     pid_params_heading.out_max);

  /* 克服电机低速死区，避免在 1~8 度附近因目标速度过小而无法回正。 */
  if (heading_error > HEADING_STOP_TOLERANCE_DEG &&
      heading_pid_output < HEADING_MIN_TURN_SPEED_CM_S)
  {
    heading_pid_output = HEADING_MIN_TURN_SPEED_CM_S;
  }
  else if (heading_error < -HEADING_STOP_TOLERANCE_DEG &&
           heading_pid_output > -HEADING_MIN_TURN_SPEED_CM_S)
  {
    heading_pid_output = -HEADING_MIN_TURN_SPEED_CM_S;
  }

  left_target = (float)basic_speed + heading_pid_output;
  right_target = (float)basic_speed - heading_pid_output;
	
//	 my_printf(UART_0_INST,"H_targe_L,H_targe_R:%.2f,%.2f",left_target,right_target);
	
  pid_set_target(&pid_speed_left, left_target);
  pid_set_target(&pid_speed_right, right_target);


}

void heading_debug_task()
{
	float yaw = imu_imu_get_yaw();
	my_printf(UART_0_INST, "x:%.2f\r\n",yaw);
}

void heading_rest_task(void)
{
	static uint32_t last_key_tick = 0U;      /* 最近一次接受航向按键的时刻，单位：ms。 */
	static bool has_accepted_key = false;   /* 是否已经接受过至少一次航向按键。 */
	uint8_t heading_key = key_down & (KEY_MASK_MY_UP | KEY_MASK_MY_DOWN); /* 本次有效航向按键位图。 */

	if ((heading_key == 0U) ||
	    (heading_key == (KEY_MASK_MY_UP | KEY_MASK_MY_DOWN)))
	{
		return;
	}

	/* Suppress both mechanical bounce and a stale key_down being read twice. */
	if (has_accepted_key &&
	    ((uint32_t)(uwTick - last_key_tick) < HEADING_KEY_DEBOUNCE_MS))
	{
		return;
	}

	last_key_tick = uwTick;
	has_accepted_key = true;

	if (heading_key == KEY_MASK_MY_UP)
	{
		/* In this project negative yaw/output is a left turn. */
		heading_target = heading_normalize_angle(heading_target - HEADING_STEP_DEG);
	}
	else
	{
		/* DOWN key: right turn by 90 degrees. */
		heading_target = heading_normalize_angle(heading_target + HEADING_STEP_DEG);
	}

	pid_set_target(&pid_heading, heading_target);
}


void PID_SetHeadingTarget(float target_deg)
{
	heading_target = heading_normalize_angle(target_deg);
	pid_set_target(&pid_heading, heading_target);
	pid_reset(&pid_heading);
}

float PID_GetHeadingError(void)
{
	return heading_normalize_angle(heading_target - imu_imu_get_yaw());
}

// volatile bool heading_contrl_enable = true; /* 原航向测试模式，保留不删除。 */
void PID_SetHeadingControl(bool enable)
{
	heading_contrl_enable = enable;

	/* 切换循迹/航向模式时清除旧控制器残留，防止目标突变造成冲击。 */
	pid_reset(&pid_speed_left);
	pid_reset(&pid_speed_right);
	pid_reset(&pid_line);
	pid_reset(&pid_heading);

	if (enable)
	{
		pid_set_target(&pid_speed_left, 0.0f);
		pid_set_target(&pid_speed_right, 0.0f);
	}
	else
	{
		pid_set_target(&pid_speed_left, (float)basic_speed);
		pid_set_target(&pid_speed_right, (float)basic_speed);
	}
}
/* PID 总开关。演示版默认 false：上电不跑闭环，只看各模块初始化与状态显示。
 * 要跑闭环：改为 true，或运行时调用 PID_SetRunning(true)。 */
volatile bool pid_running = true;

volatile bool heading_contrl_enable = false; /* false=循迹控制，true=航向闭环控制。 */
static bool pid_running_last = false;         /* 上一个控制周期的 PID 运行状态，用于识别启停边沿。 */


void PID_SetRunning(bool enable)
{
	pid_running = enable;

	if (!enable)
	{
		/* PID_SetRunning(false) 时立即停车，不等待下一次 10ms PID 调度。 */
		motor_set_l(0.0f);
		motor_set_r(0.0f);
	}
}

void PID_Task(void)
{

    float output_left = 0, output_right = 0;
    float duty_l = 0.0f;
    float duty_r = 0.0f;

	if(pid_running == false)
	{
		if(pid_running_last)
		{
			pid_reset(&pid_speed_left);
			pid_reset(&pid_speed_right);
			pid_reset(&pid_line);
			pid_reset(&pid_heading);
			motor_set_l(0.0f);
			motor_set_r(0.0f);
		}
		pid_running_last = false;
		return;
	}

	if(pid_running_last == false)
	{
		pid_reset(&pid_speed_left);
		pid_reset(&pid_speed_right);
		pid_reset(&pid_line);
		pid_reset(&pid_heading);
	}
	pid_running_last = true;


	if(heading_contrl_enable) 
	{
			if (!imu_imu_app_is_ready())
			{
				pid_reset(&pid_speed_left);
				pid_reset(&pid_speed_right);
				pid_reset(&pid_heading);
				motor_set_l(0.0f);
				motor_set_r(0.0f);
				return;
			}
			
			
	//	Line_PID_control(); /* 原无条件调用方式保留；当前按控制模式二选一。 */
			

//			Heading_PID_control();
			
	}

    output_left = pid_calculate_positional(&pid_speed_left, encoder_left.speed_cm_s);
    output_right = pid_calculate_positional(&pid_speed_right, encoder_right.speed_cm_s);
  
    output_left = pid_constrain(output_left, pid_params_left.out_min, pid_params_left.out_max);
    output_right = pid_constrain(output_right, pid_params_right.out_min, pid_params_right.out_max);

		duty_l = output_left  / V_L_MAX * 100.0f;
		duty_r = output_right / V_R_MAX * 100.0f;

		motor_set_l(duty_l);
    motor_set_r(duty_r);
		
	
//*****************************************************Debug*************************************************************************//

	
//		my_printf(UART_0_INST,"{leftpid}:%.2f,%.2f,%.2f\n",pid_speed_left.target,encoder_left.speed_cm_s);
//	  my_printf(UART_0_INST,"{rightpid}%.2f,%.2f\n",pid_speed_right.target,encoder_right.speed_cm_s);
//					

//		my_printf(UART_0_INST,"{leftpid}%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
//										pid_speed_left.target,
//										encoder_left.speed_cm_s,output_left
//										,pid_speed_right.target,
////										encoder_right.speed_cm_s,output_right);

//					my_printf(UART_0_INST,"{line}target,error%.2f,%.2f\r\n", pid_line.target, g_line_position_error);		

//*****************************************************Debug*************************************************************************//			
}


