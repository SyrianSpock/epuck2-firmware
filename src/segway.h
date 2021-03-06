#ifndef SEGWAY_H
#define SEGWAY_H

#include "attitude_estimation.h"
#include "pose_estimation.h"

#ifdef __cplusplus
extern "C" {
#endif

extern att_estim_t segway_att_estim;
extern pose_estim_base_t segway_pose_estim;

void segway_control_init(void);

void segway_control_start(void);

#ifdef __cplusplus
}
#endif

#endif /* SEGWAY_H */
