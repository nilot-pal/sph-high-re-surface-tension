# Audit: what Reynolds and Weber numbers do the shipped SPHinXsys examples actually exercise?

Compiled October 2023, against the SPHinXsys example suite of that date. This is the
evidence that the high-Re multiphase surface-tension failure sat in untested territory.

```
Example                   		Re (high)           We
2d T shaped pipe          		100               Not used
2d flow around cylinder   		100               Not used
2d fsi                    		100               Not used
2d heat transfer          		100               Not used
2d hydrostatic fsi        		0.1               Not used
2d non linear wave fsi    		1.3e6             Not used
2d Oscillating Wave Surge Converter	6.6e8         	  Not used
2d poisseule flow                       100               Not used
2d tethered dead fish in flow           5000              Not used

Core points: 1) They have two papers that discuss how to handle high Re for single phase flow
using transport velocity correction. In these examples, surface tension and hence 
Weber no. is non existent.

2) There are a couple of examples on GitHub that take realistic values of density and viscosity 
of water, thus getting a pretty high Re (~ 1e6 - 1e8). This suggests that the code has the 
capability of handling high Re for single phase flow.

3) Only a few examples/papers that discuss a multiphase problem with high Re.
Example, two phase dambreak, but they ignore some important physics, say viscous forces 
and surface tension.

4) For the 2d drop impact code that I wrote, 
Re = 80, We = 109 (non realistic values of viscosity)
Transport velocity correction was used, contact angle of 90 degrees was also used.
Results: Some what physical (surface tension model not good)

5) For the 3d drop impact code that I wrote,
Re = 217, We = 596 (U_ref was doubled, non realistic values of viscosity)
The domain size was half the domain size in 2D.
Transport velocity correction was used, contact angle of 90 degrees was also used.
Results: unphysical (total splash)

Final remarks: Multiphase with high Re (>100) and high We (> 100) seems challenging with:
1) Current surface tension model
2) Transport velocity correction
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
1) After ramping up the viscosity and density values and calculating U_ref to be 0.16 m/s
Real rho0_f = 1.0e5;
Real rho0_a = 1.0e2;  
Real mu_f = 1.0e-1;     
Real mu_a = 1.0e-3;  
tension force = 7.3
U_ref = 0.16 m/s
Results: https://screenrec.com/share/alIWdJxCBi

2) Only viscosity is increased but density and surface tension take physical values.
Real rho0_f = 1000.0;
Real rho0_a = 1.0;  
Real mu_f = 1.0e-1;     
Real mu_a = 1.0e-3;  
tension force = 0.073
U_ref = 0.16 m/s
Results: https://screenrec.com/share/z98rlDWMQw

3) Density, viscosity and surface tension take physical values.
Real rho0_f = 1000.0;
Real rho0_a = 1.0;  
Real mu_f = 1.0e-3;     
Real mu_a = 1.789e-5;  
tension force = 0.073
U_ref = 0.16 m/s
Results: https://screenrec.com/share/t2X8GY1mwg

4) Density, viscosity and surface tension take physical values. U_ref is calculated as the minimum
of v1, v2, v3 where v1, v2, v3 are calculated by using the following equality, two at a time:
Inertial force = surface tension force = viscous force, i.e.,
rho*v^2*l^2 = sigma*l = mu*v*l
Real rho0_f = 1000.0;
Real rho0_a = 1.0;  
Real mu_f = 1.0e-3;     
Real mu_a = 1.789e-5;  
tension force = 0.073
U_ref = 3.7e-4 m/s
Results: https://screenrec.com/share/KcSXWykQP0








```
