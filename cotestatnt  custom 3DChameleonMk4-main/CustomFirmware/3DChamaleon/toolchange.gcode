;===== machine: A1 =========================
;===== date: 20240830 =======================

;previous_extruder: [previous_extruder]
;toolchange_count: [toolchange_count]
;flush_length_1: [flush_length_1]
;flush_length_2: [flush_length_2]


M1007 S0 ; turn off mass estimation
G392 S0  ; turn off clog detect
;M620 S[next_extruder]A
M204 S9000 ; set starting acceleration

{if toolchange_count > 1}
G17
G2 Z{max_layer_z + 0.4} I0.86 J0.86 P1 F10000 ; spiral lift a little from second lift
{endif}
G1 Z{max_layer_z + 3.0} F1200

M400
M106 P1 S0
M106 P2 S0

; {if old_filament_temp > 142 && next_extruder < 255}
; M104 S[old_filament_temp]
; {endif}


{if toolchange_count>0}
; G1 X267 F18000
; M620.1 E F[old_filament_e_feedrate] T{nozzle_temperature_range_high[previous_extruder]}
; M620.10 A0 F[old_filament_e_feedrate]
; T[next_extruder]
; M620.1 E F[new_filament_e_feedrate] T{nozzle_temperature_range_high[next_extruder]}
; M620.10 A1 F[new_filament_e_feedrate] L[flush_length] H[nozzle_diameter] T[nozzle_temperature_range_high]

; M73 P99 R99

G0 E-5 F500 ; retract a bit, adjust this to tune waste

M17 X1.0 ; increase X motor current
G1 X279 F18000; cut filament
M400
M17 X0.65 Y1.2 Z0.6 ; reset motor current to default
{if next_extruder==0}
    G4 P600 ; dwell for .6 seconds - adjust this to match your machines single pulse time
{endif}
{if next_extruder==1}
    G4 P1100 ; dwell for 1.1 seconds - adjust this to match your machines two pulse time
{endif}
{if next_extruder==2}
    G4 P1600 ; dwell for 1.6 seconds - adjust this to match your machines three pulse time
{endif}
{if next_extruder==3}
    G4 P2100 ; dwell for 2.1 seconds - adjust this to match your machines four pulse time
{endif}

; move to poop chute
G1 X-38.2 F9000
G1 X-48.2 F1500
G1 Y250 F9000

;<<< Start Of Tip Shaping >>>
M109 R180; cool down to prevent swelling
M302 S0 ; enable cold extrusion
M106 S255
G0 E5 F1500 ;
G0 E-5 F1000 ;
M109 R165; cool down to prevent swelling
G0 E1 F1500 ;
G0 E-1 F1000 ;
; M109 R155; cool down to prevent swelling
; G0 E1 F1500 ;
; G0 E-25 F500 ;
; M109 R150; cool down to prevent swelling
; G0 E24 F1500 ; last tip dip with cold tip
; G0 E-24 ; last tip dip with cold tip
M109 R180; ok... go back up in temp so we can move the extruder
G0 E-80 F500 ; back out of the extruder
G92 E0
M106 P1 S0
M104 S[temperature];
;<<< End Of Tip Shaping >>>

;<<< Start 3D Chamaleon change movement>>>
G1 X265 F18000
G1 X275 F9000
G1 X-38.2 F18000
G1 X-48.2 F3000
M400

G4 P5000 ;wait for stepper movement complete
; M400 U1	; pause for user to load and press resume

G0 E100 F500 ; go in the extruder
;<<< Stop 3D Chamaleon change movement>>>


{endif}

{if next_extruder < 255}
M400

G92 E0
M628 S0

{if flush_length_1 > 1}
; FLUSH_START
; always use highest temperature to flush
M400
M1002 set_filament_type:UNKNOWN
M109 S[nozzle_temperature_range_high]
M106 P1 S60
{if flush_length_1 > 23.7}
G1 E23.7 F{old_filament_e_feedrate} ; do not need pulsatile flushing for start part
G1 E{(flush_length_1 - 23.7) * 0.02} F50
G1 E{(flush_length_1 - 23.7) * 0.23} F{old_filament_e_feedrate}
G1 E{(flush_length_1 - 23.7) * 0.02} F50
G1 E{(flush_length_1 - 23.7) * 0.23} F{new_filament_e_feedrate}
G1 E{(flush_length_1 - 23.7) * 0.02} F50
G1 E{(flush_length_1 - 23.7) * 0.23} F{new_filament_e_feedrate}
G1 E{(flush_length_1 - 23.7) * 0.02} F50
G1 E{(flush_length_1 - 23.7) * 0.23} F{new_filament_e_feedrate}
{else}
G1 E{flush_length_1} F{old_filament_e_feedrate}
{endif}
; FLUSH_END
G1 E-[old_retract_length_toolchange] F1800
G1 E[old_retract_length_toolchange] F300
M400
M1002 set_filament_type:{filament_type[next_extruder]}
{endif}

{if flush_length_1 > 45 && flush_length_2 > 1}
; WIPE
M400
M106 P1 S178
M400 S3
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
M400
M106 P1 S0
{endif}

{if flush_length_2 > 1}
M106 P1 S60
; FLUSH_START
G1 E{flush_length_2 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_2 * 0.02} F50
G1 E{flush_length_2 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_2 * 0.02} F50
G1 E{flush_length_2 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_2 * 0.02} F50
G1 E{flush_length_2 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_2 * 0.02} F50
G1 E{flush_length_2 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_2 * 0.02} F50
; FLUSH_END
G1 E-[new_retract_length_toolchange] F1800
G1 E[new_retract_length_toolchange] F300
{endif}

{if flush_length_2 > 45 && flush_length_3 > 1}
; WIPE
M400
M106 P1 S178
M400 S3
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
M400
M106 P1 S0
{endif}

{if flush_length_3 > 1}
M106 P1 S60
; FLUSH_START
G1 E{flush_length_3 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_3 * 0.02} F50
G1 E{flush_length_3 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_3 * 0.02} F50
G1 E{flush_length_3 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_3 * 0.02} F50
G1 E{flush_length_3 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_3 * 0.02} F50
G1 E{flush_length_3 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_3 * 0.02} F50
; FLUSH_END
G1 E-[new_retract_length_toolchange] F1800
G1 E[new_retract_length_toolchange] F300
{endif}

{if flush_length_3 > 45 && flush_length_4 > 1}
; WIPE
M400
M106 P1 S178
M400 S3
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
M400
M106 P1 S0
{endif}

{if flush_length_4 > 1}
M106 P1 S60
; FLUSH_START
G1 E{flush_length_4 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_4 * 0.02} F50
G1 E{flush_length_4 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_4 * 0.02} F50
G1 E{flush_length_4 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_4 * 0.02} F50
G1 E{flush_length_4 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_4 * 0.02} F50
G1 E{flush_length_4 * 0.18} F{new_filament_e_feedrate}
G1 E{flush_length_4 * 0.02} F50
; FLUSH_END
{endif}

M629

M400
M106 P1 S60
M109 S[new_filament_temp]
G1 E6 F{new_filament_e_feedrate} ;Compensate for filament spillage during waiting temperature
M400
G92 E0
G1 E-[new_retract_length_toolchange] F1800
M400
M106 P1 S178
M400 S3
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
G1 X-38.2 F18000
G1 X-48.2 F3000
M400
G1 Z{max_layer_z + 3.0} F3000
M106 P1 S0
{if layer_z <= (initial_layer_print_height + 0.001)}
M204 S[initial_layer_acceleration]
{else}
M204 S[default_acceleration]
{endif}
; {else}
; G1 X[x_after_toolchange] Y[y_after_toolchange] Z[z_after_toolchange] F12000
{endif}

M622.1 S0
M9833 F{outer_wall_volumetric_speed/2.4} A0.3 ; cali dynamic extrusion compensation
M1002 judge_flag filament_need_cali_flag
M622 J1
  G92 E0
  G1 E-[new_retract_length_toolchange] F1800
  M400
  
  M106 P1 S178
  M400 S4
  G1 X-38.2 F18000
  G1 X-48.2 F3000
  G1 X-38.2 F18000 ;wipe and shake
  G1 X-48.2 F3000
  G1 X-38.2 F12000 ;wipe and shake
  G1 X-48.2 F3000
  M400
  M106 P1 S0 
  

M623

; M621 S[next_extruder]A

; as there is no AMS, these next three lines only serve to hide T[next_extruder]
; if this was not included, the T[next_extruder] command is input after this
; code and will cause the system to hang as the toolchange command searches
; for the AMS
M620 S[next_extruder]A
T[next_extruder]
M621 S[next_extruder]A

G392 S0

M1007 S1