/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 *        Copyright 2014, 2015 Big Switch Networks, Inc.       
 * 
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * 
 *        http://www.eclipse.org/legal/epl-v10.html
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 * 
 * </bsn.cl>
 ************************************************************
 *
 * x86_64_ufispace_s9180_32x_log.c
 *
 ***********************************************************/

#include <x86_64_ufispace_s9180_32x/x86_64_ufispace_s9180_32x_config.h>

#include "x86_64_ufispace_s9180_32x_log.h"
/*
 * x86_64_ufispace_s9180_32x log struct.
 */
AIM_LOG_STRUCT_DEFINE(
                      X86_64_UFISPACE_S9180_32X_CONFIG_LOG_OPTIONS_DEFAULT,
                      X86_64_UFISPACE_S9180_32X_CONFIG_LOG_BITS_DEFAULT,
                      NULL, /* Custom log map */
                      X86_64_UFISPACE_S9180_32X_CONFIG_LOG_CUSTOM_BITS_DEFAULT
                     );

