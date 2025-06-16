# RealTime_SpeedControl
C++ code to collect and manipulate the Bertec split belt treadmill belt speeds in real time using data from imbedded force plates and participants' stride characteristics.

Below is a graphic from my dissertation work outlining the speed controller set up and the operation on the treadmill with a participant.

### Real Time Controller Set Up
![Real Time Speed and Foot Placement Feedback System](G:\My Drive\Python\MyGitRepo\RealTime_SpeedControl\Foot placement and speed change example 2.jpg)

(A) Overview of experimental set up and margin of stability calculation. STa group
example given. Visual feedback was provided in front of the treadmill with stride-bystride cadence displayed, highlighted in green when matched to ±1 stride per minute. Foot
placement feedback was given only on the visual display in real-time, in global lab-space.
Shaded gradient on each treadmill belt indicates changing belt speed at 5 m/s2, e.g., from
0.6 m/s (black) to 1.8 m/s (white) from front to back on the left limb. Measure of the
margin of stability shown with black and gray vectors indicating contributions of the
center-of-mass oscillations and treadmill belt speed to relative center-of-mass velocity for
calculation of 𝜉𝜉. Blue bi-directional arrow indicates 𝑢𝑢 in center-of-mass space, with 𝑏𝑏 as
the difference between 𝑢𝑢 and the downward projection of the anterior-posterior position
of 𝜉𝜉. 

(B) Single subject examples of real-time belt-speed changes from 0-100% of the gait
cycle for the S2a group (left panel) and the FPa and STa groups (right panel). Yellow
outlined boxes indicate either loose (30 cm, S2a, FPa) or tight (3 cm, STa) constraints on
foot placement. Horizontal colored bars indicate left and right stance. Gray shading
indicates double support. (C) Experimental paradigm. White panels indicated tied walking
conditions. Gray shading is tied-belt feedback training. Yellow shading indicates splitbelt walking trials. Training adaptation (block 3, trials 1-2) included the three
experimental groups (S2a, cyan; FPa, yellow; STa, magenta). The transfer adaptation
block on day 2 involved all three experimental groups plus the control group (CON,
black). Pink and blue horizontal lines indicated belt speeds for each limb. Y-axis indicates
belt speed, x-axis by strides. Metronomes in block 3 indicate the percentage of preferred
stride rate by indicated belt speed. (D) Spatiotemporal measures, over time. Black and
gray lines are ankle marker positions for the fast and slow limbs, in center-of-mass space.
Up indicates forward. Pink fill and lines indicate slow limb parameters, while blue
indicates fast. Dash-dot lines indicated heel-strike and toe-off events. Foot placement
indicated by green outlined circles, step lengths by vertical arrows, horizontal dot-arrows
indicate step times (bottom) and double support times (top).
